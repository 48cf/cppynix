#include <flanterm_backends/fb.h>

#include <arch/snippets.hpp>
#include <arch/debug.hpp>
#include <boot/limine.hpp>
#include <lib/debug.hpp>

namespace kernel::lib::debug {

namespace {

flanterm_context *flanterm_ctx = nullptr;

} // namespace

void Sink::append(char ch) const {
    arch::debug::print(ch);

    if (flanterm_ctx) {
        flanterm_write(flanterm_ctx, &ch, 1);
    }
}

void Sink::append(const char *str) const {
    frg::string_view str_view{str};

    arch::debug::print(str_view);

    if (flanterm_ctx) {
        flanterm_write(flanterm_ctx, str, str_view.size());
    }
}

void init() {
    auto response = boot::limine::framebuffer_request.response;

    // Ensure we got a framebuffer.
    if (response == nullptr || response->framebuffer_count < 1) {
        return;
    }

    // Fetch the first framebuffer.
    auto framebuffer = response->framebuffers[0];

    flanterm_ctx = flanterm_fb_init(
        nullptr, nullptr, static_cast<uint32_t *>(framebuffer->address),
        framebuffer->width, framebuffer->height, framebuffer->pitch,
        framebuffer->red_mask_size, framebuffer->red_mask_shift,
        framebuffer->green_mask_size, framebuffer->green_mask_shift,
        framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, 0, 0, 0, 0, 0, 0);
}

[[noreturn]] void panic(frg::string_view msg) {
    print("KERNEL PANIC: {}\n", msg);

    for (;;) {
        arch::snippets::disable_interrupts();
        arch::snippets::wait_for_interrupt();
    }
}

} // namespace kernel::lib::debug
