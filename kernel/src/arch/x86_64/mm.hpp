#pragma once

#include <cstddef>

namespace kernel::arch::mm {

constexpr static std::size_t page_size = 0x1000;
constexpr static std::size_t page_shift = 12;

} // namespace kernel::arch::mm
