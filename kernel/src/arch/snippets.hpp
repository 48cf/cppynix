#pragma once

namespace kernel::arch::snippets {

void wait_for_interrupt();

bool interrupts_enabled();
void enable_interrupts();
void disable_interrupts();

} // namespace kernel::arch::snippets

#if defined(__x86_64__)
#include <arch/x86_64/snippets.hpp>
#endif
