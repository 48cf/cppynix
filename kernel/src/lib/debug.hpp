#pragma once

#include <frg/formatting.hpp>

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            lib::debug::panic("Assertion failed: " #cond); \
        } \
    } while (false)

namespace kernel::lib::debug {

struct Sink {
    void append(char ch) const;
    void append(const char *str) const;

    static Sink &get_sink() {
        static Sink sink;
        return sink;
    }

private:
    Sink() = default;
};

void init();

[[noreturn]] void panic(frg::string_view msg);

template <typename... Args>
void print(frg::string_view fmt, Args&&... args) {
    frg::format(frg::fmt(fmt, std::forward<Args>(args)...), Sink::get_sink());
}

} // namespace kernel::lib::debug
