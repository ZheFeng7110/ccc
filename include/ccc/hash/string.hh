/**
 * @file string.hh
 * @brief `ccc::hash` specializations for std::basic_string and std::basic_string_view
 *        (Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_STRING_HH
#define CCC_HASH_STRING_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <string>

#if (__cplusplus >= 201703L)
#include <string_view>
#endif

#include "ccc/detail/murmur_hash3.hh"
#include "ccc/hash/hash.hh"

namespace ccc {

template<typename CharT, typename Traits, typename Allocator>
struct hash<std::basic_string<CharT, Traits, Allocator>, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(
        const std::basic_string<CharT, Traits, Allocator>& value) const noexcept
    {
        return detail::murmur_hash3(value.data(), value.size());
    }
};

#if (__cplusplus >= 201703L)
template<typename CharT, typename Traits>
struct hash<std::basic_string_view<CharT, Traits>, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::basic_string_view<CharT, Traits>& value) const noexcept
    {
        return detail::murmur_hash3(value.data(), value.size());
    }
};
#endif

}  // namespace ccc

#endif  // !CCC_HASH_STRING_HH
