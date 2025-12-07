#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace kernel::mm {

struct PhysicalAddr {
    explicit constexpr PhysicalAddr() : _value{0} {}
    explicit constexpr PhysicalAddr(std::uintptr_t val) : _value{val} {}

    [[nodiscard]] constexpr std::uintptr_t get() const {
        return _value;
    }

    [[nodiscard]] constexpr bool is_null() const {
        return _value == 0;
    }

    [[nodiscard]] constexpr operator std::uintptr_t() const {
        return _value;
    }

    [[nodiscard]] constexpr std::strong_ordering operator<=>(PhysicalAddr other) const {
        return _value <=> other._value;
    }

    constexpr PhysicalAddr operator+(std::size_t offset) const {
        return PhysicalAddr{_value + offset};
    }

    constexpr PhysicalAddr operator+(PhysicalAddr other) const {
        return PhysicalAddr{_value + other._value};
    }

    constexpr PhysicalAddr &operator+=(std::size_t offset) {
        _value += offset;
        return *this;
    }

    constexpr PhysicalAddr &operator+=(PhysicalAddr other) {
        _value += other._value;
        return *this;
    }

    constexpr PhysicalAddr operator-(std::size_t offset) const {
        return PhysicalAddr{_value - offset};
    }

    constexpr PhysicalAddr operator-(PhysicalAddr other) const {
        return PhysicalAddr{_value - other._value};
    }

    constexpr PhysicalAddr &operator-=(std::size_t offset) {
        _value -= offset;
        return *this;
    }

    constexpr PhysicalAddr &operator-=(PhysicalAddr other) {
        _value -= other._value;
        return *this;
    }

private:
    std::uintptr_t _value;
};

namespace pmm {

void init();

std::optional<PhysicalAddr> allocate_pages(std::size_t num_pages);

void free_pages(PhysicalAddr addr, std::size_t num_pages);

} // namespace pmm

struct AllocatedPages {
    static std::optional<AllocatedPages> allocate(std::size_t num_pages);

    AllocatedPages(const AllocatedPages &) = delete;
    AllocatedPages &operator=(const AllocatedPages &) = delete;

    AllocatedPages(AllocatedPages &&other) {
        _addr = other._addr;
        _num_pages = other._num_pages;

        other._addr = PhysicalAddr{};
        other._num_pages = 0;
    }

    AllocatedPages &operator=(AllocatedPages &&other) {
        if (this != &other) {
            _addr = other._addr;
            _num_pages = other._num_pages;

            other._addr = PhysicalAddr{};
            other._num_pages = 0;
        }

        return *this;
    }

    ~AllocatedPages();

    [[nodiscard]] PhysicalAddr address() const {
        return _addr;
    }

    [[nodiscard]] std::size_t num_pages() const {
        return _num_pages;
    }

    [[nodiscard]] bool is_empty() const {
        return _num_pages == 0;
    }

    [[nodiscard]] std::optional<PhysicalAddr> take(std::size_t count = 1);
    [[nodiscard]] std::optional<PhysicalAddr> take_all();

private:
    AllocatedPages(PhysicalAddr addr, std::size_t num_pages)
        : _addr{addr}, _num_pages{num_pages} {}

    PhysicalAddr _addr;
    std::size_t _num_pages;
};

} // namespace kernel::mm
