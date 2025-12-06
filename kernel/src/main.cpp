#include <cstdint>
#include <cstddef>
#include <limine.h>

#include <boot/limine.hpp>
#include <lib/debug.hpp>
#include <mm/pmm.hpp>

namespace kernel {

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .cpp file.

extern "C" {

void *memcpy(void *__restrict dest, const void *__restrict src, std::size_t n) {
    std::uint8_t *__restrict pdest = static_cast<std::uint8_t *__restrict>(dest);
    const std::uint8_t *__restrict psrc = static_cast<const std::uint8_t *__restrict>(src);

    for (std::size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, std::size_t n) {
    std::uint8_t *p = static_cast<std::uint8_t *>(s);

    for (std::size_t i = 0; i < n; i++) {
        p[i] = static_cast<uint8_t>(c);
    }

    return s;
}

void *memmove(void *dest, const void *src, std::size_t n) {
    std::uint8_t *pdest = static_cast<std::uint8_t *>(dest);
    const std::uint8_t *psrc = static_cast<const std::uint8_t *>(src);

    if (src > dest) {
        for (std::size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (std::size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, std::size_t n) {
    const std::uint8_t *p1 = static_cast<const std::uint8_t *>(s1);
    const std::uint8_t *p2 = static_cast<const std::uint8_t *>(s2);

    for (std::size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

}

// The following stubs are required by the Itanium C++ ABI (the one we use,
// regardless of the "Itanium" nomenclature).
// Like the memory functions above, these stubs can be moved to a different .cpp file,
// but should not be removed, unless you know what you are doing.
extern "C" {

int __cxa_atexit(void (*)(void *), void *, void *) {
    return 0;
}

void __cxa_pure_virtual() {
    lib::debug::panic("__cxa_pure_virtual() called");
}

void *__dso_handle;

// Extern declarations for global constructors array.
extern void (*__init_array[])();
extern void (*__init_array_end[])();

}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
extern "C" void kmain() {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(boot::limine::base_revision) == false) {
        for (;;) {}
    }

    // Call global constructors.
    for (std::size_t i = 0; &__init_array[i] != __init_array_end; i++) {
        __init_array[i]();
    }

    lib::debug::init();
    lib::debug::print("Hello, world!\n");

    mm::pmm::init();

    {
        auto pages1 = mm::pmm::allocate_pages(69);
        auto pages2 = mm::pmm::allocate_pages(420);
        auto pages3 = mm::pmm::allocate_pages(1337);

        lib::debug::print("Allocated pages at physical addresses: 0x{}, 0x{}, 0x{}\n",
            frg::hex_fmt{pages1 ? pages1->get() : 0},
            frg::hex_fmt{pages2 ? pages2->get() : 0},
            frg::hex_fmt{pages3 ? pages3->get() : 0});

        if (pages3) {
            mm::pmm::free_pages(*pages3, 1337);
        }

        if (pages2) {
            mm::pmm::free_pages(*pages2, 420);
        }

        if (pages1) {
            mm::pmm::free_pages(*pages1, 69);
        }
    }

    // We're done, just hang...
    lib::debug::panic("Nothing to do");
}

} // namespace kernel
