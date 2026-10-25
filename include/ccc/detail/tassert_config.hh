#pragma once
#ifndef CCC_DETAIL_TASSERT_CONFIG_HH
#define CCC_DETAIL_TASSERT_CONFIG_HH

#if __cplusplus < 202002L
#error "ccc.tassert requires C++20 or higher"
#endif

#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>

#ifdef CCC_TASSERT_CONFIG_HEADER
#include CCC_TASSERT_CONFIG_HEADER
#else
namespace ccc::tassert_config {
inline constexpr std::array<std::string_view, 0> ignore_tags{};
}
#endif

namespace ccc::detail {
template<class T>
inline constexpr bool tassert_config_sequence = false;
template<std::size_t N>
inline constexpr bool tassert_config_sequence<std::string_view[N]> = true;
template<std::size_t N>
inline constexpr bool tassert_config_sequence<std::array<std::string_view, N>> = true;

static_assert(tassert_config_sequence<std::remove_cv_t<decltype(tassert_config::ignore_tags)>>,
              "ccc.tassert ignore_tags must be an inline constexpr string_view array or std::array<string_view, N>");

// Validate constant evaluation even in a translation unit that constructs no tags.
inline constexpr auto tassert_config_size = []() consteval {
    std::size_t size = 0;
    for (std::string_view ignored : tassert_config::ignore_tags) {
        size += ignored.size();
    }
    return size;
}();

constexpr bool tassert_tag_enabled(std::string_view name) noexcept
{
    for (std::string_view ignored : tassert_config::ignore_tags) {
        if (name == ignored) {
            return false;
        }
    }
    return true;
}
}  // namespace ccc::detail

// fmt is always opt-in. C++20 deliberately keeps the standard backend disabled.
#if __cplusplus > 202002L && __has_include(<format>)
#include <format>
#if defined(__cpp_lib_format) && __cpp_lib_format >= 202110L
#define CCC_DETAIL_TASSERT_HAS_STD_FORMAT 1
#endif
#endif
#ifndef CCC_DETAIL_TASSERT_HAS_STD_FORMAT
#define CCC_DETAIL_TASSERT_HAS_STD_FORMAT 0
#endif

#ifndef CCC_TASSERT_FORMAT_BACKEND
#if CCC_DETAIL_TASSERT_HAS_STD_FORMAT
#define CCC_TASSERT_FORMAT_BACKEND 1
#else
#define CCC_TASSERT_FORMAT_BACKEND 0
#endif
#endif

#if CCC_TASSERT_FORMAT_BACKEND == 1
#if !CCC_DETAIL_TASSERT_HAS_STD_FORMAT
#error "ccc.tassert standard formatting backend requires C++23 and usable std::format"
#endif
#elif CCC_TASSERT_FORMAT_BACKEND == 2
#if __has_include(<fmt/format.h>)
#include <fmt/format.h>
#else
#error "ccc.tassert fmt backend requires fmt/format.h; provide the fmt dependency"
#endif
#elif CCC_TASSERT_FORMAT_BACKEND != 0
#error "ccc.tassert invalid CCC_TASSERT_FORMAT_BACKEND (expected 0, 1 or 2)"
#endif

#endif  // !CCC_DETAIL_TASSERT_CONFIG_HH
