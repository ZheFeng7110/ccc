#pragma once
#ifndef CCC_TEST_TASSERT_FORMAT_SUPPORT_HH
#define CCC_TEST_TASSERT_FORMAT_SUPPORT_HH

#include <stdexcept>
#include <string>
#if TEST_FORMAT_BACKEND == 2
#include <fmt/format.h>
#else
#include <format>
#endif

inline int formatter_calls = 0;
struct Item {
    bool throws;
};

#if TEST_FORMAT_BACKEND == 2
template<>
struct fmt::formatter<Item> : fmt::formatter<std::string_view> {
#else
template<>
struct std::formatter<Item> : std::formatter<std::string_view> {
#endif
    template<class Context>
    auto format(const Item& item, Context& context) const
    {
        ++formatter_calls;
        if (item.throws) {
            throw std::runtime_error("formatter");
        }
#if TEST_FORMAT_BACKEND == 2
        return fmt::format_to(context.out(), "custom");
#else
        return std::format_to(context.out(), "custom");
#endif
    }
};
#endif  // !CCC_TEST_TASSERT_FORMAT_SUPPORT_HH
