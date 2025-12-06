#pragma once

#include <cstdint>

namespace kernel::arch::snippets {

inline void wait_for_interrupt() {
    asm volatile("hlt");
}

inline bool interrupts_enabled() {
    std::uint64_t rflags;
    asm volatile("pushfq; popq %0" : "=r"(rflags));
    return rflags & (1 << 9);
}

inline void enable_interrupts() {
    asm volatile("sti");
}

inline void disable_interrupts() {
    asm volatile("cli");
}

} // namespace kernel::arch::snippets
