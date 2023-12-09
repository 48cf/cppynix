#pragma once

#include <frg/formatting.hpp>

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            panic("Assertion failed: " #cond); \
        } \
    } while (false)

namespace kernel {

struct DebugSink {
    void append(char ch) const;
    void append(const char *str) const;
};

DebugSink &debug_sink();

void initialize_debug();

[[noreturn]] void panic(frg::string_view msg);

template <typename... Args>
void print(frg::string_view fmt, Args&&... args) {
    frg::format(frg::fmt(fmt, std::forward<Args>(args)...), debug_sink());
}

} // namespace kernel
