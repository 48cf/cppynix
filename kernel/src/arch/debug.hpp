#pragma once

#include <frg/string.hpp>

namespace kernel::arch::debug {

void print(char ch);
void print(frg::string_view str);

} // namespace kernel::arch::debug
