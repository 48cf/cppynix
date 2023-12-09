#pragma once

namespace kernel::arch {

void wait_for_interrupt();

bool interrupts_enabled();
void enable_interrupts();
void disable_interrupts();

} // namespace kernel::arch

#if defined(__x86_64__)
#include <arch/x86_64/asm.hpp>
#endif
