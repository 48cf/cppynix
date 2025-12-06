#include <arch/mm.hpp>
#include <boot/limine.hpp>
#include <lib/debug.hpp>
#include <mm/pmm.hpp>

#include <frg/list.hpp>
#include <frg/mutex.hpp>
#include <frg/spinlock.hpp>

namespace kernel::mm::pmm {

namespace {

frg::string_view memory_entry_type_to_string(std::uint32_t type) {
    switch (type) {
        case LIMINE_MEMMAP_USABLE:
            return "usable";
        case LIMINE_MEMMAP_RESERVED:
            return "reserved";
        case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
            return "ACPI reclaimable";
        case LIMINE_MEMMAP_ACPI_NVS:
            return "ACPI NVS";
        case LIMINE_MEMMAP_BAD_MEMORY:
            return "bad memory";
        case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
            return "bootloader reclaimable";
        case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
            return "executable and modules";
        case LIMINE_MEMMAP_FRAMEBUFFER:
            return "framebuffer";
        case LIMINE_MEMMAP_ACPI_TABLES:
            return "ACPI tables";
    }

    return "unknown";
}

struct FreePages {
    explicit FreePages(std::size_t pages) : num_pages{pages} {}

    frg::default_list_hook<FreePages> hook;
    std::size_t num_pages;

    [[nodiscard]] PhysicalAddr address() const {
        std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(this);
        return PhysicalAddr{addr - boot::limine::hhdm_request.response->offset};
    }
};

using FreeList = frg::intrusive_list<
    FreePages,
    frg::locate_member<FreePages, frg::default_list_hook<FreePages>, &FreePages::hook>
>;

FreeList free_pages_list;
frg::ticket_spinlock free_pages_lock;

} // namespace

void init() {
    limine_memmap_response *memmap_response = boot::limine::memmap_request.response;
    limine_hhdm_response *hhdm_response = boot::limine::hhdm_request.response;

    ASSERT(memmap_response != nullptr);
    ASSERT(hhdm_response != nullptr);

    std::size_t total_pages = 0;

    for (std::size_t i = 0; i < memmap_response->entry_count; i++) {
        limine_memmap_entry *entry = memmap_response->entries[i];

        lib::debug::print("Memory map entry: base=0x{}, length=0x{}, type={}\n",
            frg::hex_fmt{entry->base}, frg::hex_fmt{entry->length}, memory_entry_type_to_string(entry->type));

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        void *free_pages = reinterpret_cast<void *>(entry->base + hhdm_response->offset);

        FreePages *pages = new(free_pages) FreePages(entry->length / arch::mm::page_size);

        free_pages_list.push_back(pages);
        total_pages += pages->num_pages;
    }

    lib::debug::print("Total physical memory available: {} KiB\n",
        total_pages * arch::mm::page_size / 1024);
}

std::optional<PhysicalAddr> allocate_pages(std::size_t num_pages) {
    frg::unique_lock lock{free_pages_lock};

    FreePages *free_pages = nullptr;

    // Fast path for single-page allocations which are the most common.
    // Any block on the free list will be able to satisfy those.
    if (num_pages == 1) [[likely]] {
        free_pages = free_pages_list.front();
    } else {
        for (auto it = free_pages_list.begin(); it != free_pages_list.end(); ++it) {
            FreePages *pages = *it;

            if (pages->num_pages >= num_pages) {
                free_pages = pages;
                break;
            }
        }
    }

    if (free_pages == nullptr) {
        return std::nullopt;
    }

    PhysicalAddr addr = free_pages->address();

    // Adjust the free list entry - if we used all the pages, remove it.
    if (free_pages->num_pages == num_pages) {
        free_pages_list.erase(free_pages_list.iterator_to(free_pages));
    } else {
        free_pages->num_pages -= num_pages;
        addr += free_pages->num_pages * arch::mm::page_size;
    }

    return addr;
}

void free_pages(PhysicalAddr addr, std::size_t num_pages) {
    frg::unique_lock lock{free_pages_lock};

    void *free_pages = reinterpret_cast<void *>(addr.get() + boot::limine::hhdm_request.response->offset);

    free_pages_list.push_back(new(free_pages) FreePages(num_pages));
}

} // namespace kernel::mm::pmm

namespace kernel::mm {

std::optional<AllocatedPages> AllocatedPages::allocate(std::size_t num_pages) {
    auto pages_opt = pmm::allocate_pages(num_pages);

    if (!pages_opt) {
        return std::nullopt;
    }

    return AllocatedPages{*pages_opt, num_pages};
}

AllocatedPages::~AllocatedPages() {
    if (!_addr.is_null() && !is_empty()) {
        pmm::free_pages(_addr, _num_pages);
    }
}

std::optional<PhysicalAddr> AllocatedPages::take(std::size_t count) {
    if (count > _num_pages || count == 0) {
        return std::nullopt;
    }

    PhysicalAddr addr = _addr;

    _addr = PhysicalAddr{_addr.get() + count * arch::mm::page_size};
    _num_pages -= count;

    return addr;
}

std::optional<PhysicalAddr> AllocatedPages::take_all() {
    return take(_num_pages);
}

} // namespace kernel::mm
