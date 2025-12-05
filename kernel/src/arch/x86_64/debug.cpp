#include <arch/debug.hpp>
#include <arch/x86_64/port.hpp>

namespace kernel::arch::debug {

void print(char ch) {
    outb(0xe9, ch);
}

void print(frg::string_view str) {
    for (size_t i = 0; i < str.size(); i++) {
        outb(0xe9, str[i]);
    }
}

} // namespace kernel::arch::debug
