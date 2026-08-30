/**
 * @file optional_variant.hh
 * @brief `ccc::hash` specializations for std::optional, std::variant and
 *        std::monostate (Compile-time-capable after C++20). Requires C++17.
 */

#pragma once
#ifndef CCC_HASH_OPTIONAL_VARIANT_HH
#define CCC_HASH_OPTIONAL_VARIANT_HH

#include "ccc/detail/config.hh"

#if (__cplusplus >= 201703L)

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#include "ccc/hash/hash.hh"
#include "ccc/hash/utils.hh"

namespace ccc {
namespace detail {

// Fixed seeds so that empty optionals / valueless variants hash distinctly
// from any engaged state
inline constexpr std::size_t hash_seed_nullopt = 0x6e756c6cU;    // "null"
inline constexpr std::size_t hash_seed_monostate = 0x6d6f6e6fU;  // "mono"
inline constexpr std::size_t hash_seed_valueless = 0x766c6573U;  // "vles"

template<std::size_t Index, typename... Types>
inline CCC_CPP20_CONSTEXPR typename std::enable_if<Index == sizeof...(Types), std::size_t>::type
hash_variant_alternative(std::size_t seed,
                         const std::variant<Types...>&) noexcept(all_nothrow_hashable<Types...>::value)
{
    return seed;
}

template<std::size_t Index, typename... Types>
inline CCC_CPP20_CONSTEXPR typename std::enable_if<(Index < sizeof...(Types)), std::size_t>::type
hash_variant_alternative(std::size_t seed,
                         const std::variant<Types...>& value) noexcept(all_nothrow_hashable<Types...>::value)
{
    if (value.index() == Index) {
        return hash_combine(seed, std::get<Index>(value));
    }
    return hash_variant_alternative<Index + 1>(seed, value);
}

}  // namespace detail

template<>
struct hash<std::monostate, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(std::monostate) const noexcept
    {
        return detail::hash_seed_monostate;
    }
};

template<typename T>
struct hash<std::optional<T>, typename std::enable_if<detail::is_hashable<T>::value, void>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::optional<T>& value) noexcept(
        detail::is_nothrow_hashable<T>::value)
    {
        if (!value.has_value()) {
            return detail::hash_seed_nullopt;
        }
        return hash_combine(detail::hash_seed_nullopt, *value);
    }
};

template<typename... Types>
struct hash<std::variant<Types...>, typename std::enable_if<detail::all_hashable<Types...>::value, void>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::variant<Types...>& value) noexcept(
        detail::all_nothrow_hashable<Types...>::value)
    {
        if (value.valueless_by_exception()) {
            return detail::hash_seed_valueless;
        }
        return detail::hash_variant_alternative<0>(hash_combine(0U, value.index()), value);
    }
};

}  // namespace ccc

#endif  // C++17

#endif  // !CCC_HASH_OPTIONAL_VARIANT_HH
