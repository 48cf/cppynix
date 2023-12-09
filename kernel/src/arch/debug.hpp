#pragma once

#include <frg/string.hpp>

namespace kernel::arch {

void debug_print(char ch);
void debug_print(frg::string_view str);

} // namespace kernel::arch
