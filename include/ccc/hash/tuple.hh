/**
 * @file tuple.hh
 * @brief `ccc::hash` specializations for std::pair and std::tuple
 *        (Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_TUPLE_HH
#define CCC_HASH_TUPLE_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ccc/hash/hash.hh"
#include "ccc/hash/utils.hh"

namespace ccc {
namespace detail {

template<std::size_t Index, typename... Types>
inline CCC_CPP20_CONSTEXPR typename std::enable_if<Index == sizeof...(Types), std::size_t>::type hash_tuple_combine(
    std::size_t seed,
    const std::tuple<Types...>&) noexcept(all_nothrow_hashable<Types...>::value)
{
    return seed;
}

template<std::size_t Index, typename... Types>
inline CCC_CPP20_CONSTEXPR typename std::enable_if<(Index < sizeof...(Types)), std::size_t>::type hash_tuple_combine(
    std::size_t seed,
    const std::tuple<Types...>& value) noexcept(all_nothrow_hashable<Types...>::value)
{
    return hash_tuple_combine<Index + 1>(hash_combine(seed, std::get<Index>(value)), value);
}

}  // namespace detail

template<typename First, typename Second>
struct hash<std::pair<First, Second>, typename std::enable_if<detail::all_hashable<First, Second>::value, void>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::pair<First, Second>& value) noexcept(
        detail::is_nothrow_hashable<First>::value && detail::is_nothrow_hashable<Second>::value)
    {
        return hash_combine(hash_combine(0U, value.first), value.second);
    }
};

template<typename... Types>
struct hash<std::tuple<Types...>, typename std::enable_if<detail::all_hashable<Types...>::value, void>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::tuple<Types...>& value) noexcept(
        detail::all_nothrow_hashable<Types...>::value)
    {
        return detail::hash_tuple_combine<0>(0U, value);
    }
};

}  // namespace ccc

#endif  // !CCC_HASH_TUPLE_HH
