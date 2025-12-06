#include <boot/limine.hpp>

#define LIMINE_REQUEST(NAME, ID, REVISION, ...) \
    [[gnu::used, gnu::section(".limine_requests")]] \
    volatile limine_##NAME NAME = { \
        .id = ID, \
        .revision = (REVISION), \
        .response = nullptr, \
        __VA_ARGS__ \
    }

namespace kernel::boot::limine {

LIMINE_REQUEST(framebuffer_request, LIMINE_FRAMEBUFFER_REQUEST_ID, 0);
LIMINE_REQUEST(memmap_request, LIMINE_MEMMAP_REQUEST_ID, 0);
LIMINE_REQUEST(hhdm_request, LIMINE_HHDM_REQUEST_ID, 0);

// Set the base revision to 4, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

[[gnu::used, gnu::section(".limine_requests")]]
volatile std::uint64_t base_revision[] = LIMINE_BASE_REVISION(4);

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .cpp file, as seen fit.

namespace {

[[gnu::used, gnu::section(".limine_requests_start")]]
volatile std::uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

[[gnu::used, gnu::section(".limine_requests_end")]]
volatile std::uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

}

} // namespace kernel::boot::limine
