/**
 * @file utils.hh
 * @brief Hash combination utilities(Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_UTILS_HH
#define CCC_HASH_UTILS_HH

#include "ccc/detail/config.hh"

#include <cstddef>

#include "ccc/hash/hash.hh"

namespace ccc {

/**
 * @brief Boost-style hash combination. Mixes the hash of `value` into `seed`
 *        and returns the new seed, following the classic scheme:
 *        `seed ^= hash(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2)`.
 * @param seed The seed to mix into.
 * @param value The value to hash and combine.
 * @return The new seed. Callers may chain: `seed = hash_combine(seed, v1, v2, ...)`.
 */
template<typename T>
inline CCC_CPP20_CONSTEXPR std::size_t hash_combine(std::size_t seed,
                                                    const T& value) noexcept(detail::is_nothrow_hashable<T>::value)
{
    const std::size_t value_hash = hash<T>()(value);
    return seed ^ (value_hash + 0x9e3779b9U + (seed << 6U) + (seed >> 2U));
}

/**
 * @brief Variadic overload that combines every value into the seed in order.
 */
template<typename T, typename... Rest>
inline CCC_CPP20_CONSTEXPR std::size_t hash_combine(std::size_t seed, const T& value, const Rest&... rest) noexcept(
    detail::all_nothrow_hashable<T, Rest...>::value)
{
    return hash_combine(hash_combine(seed, value), rest...);
}

}  // namespace ccc

#endif  // !CCC_HASH_UTILS_HH
