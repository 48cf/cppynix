#pragma once

#include <cstdint>

#include <limine.h>

namespace kernel::boot::limine {

extern volatile std::uint64_t base_revision[];

extern volatile limine_framebuffer_request framebuffer_request;
extern volatile limine_memmap_request memmap_request;
extern volatile limine_hhdm_request hhdm_request;

} // namespace kernel::boot::limine
