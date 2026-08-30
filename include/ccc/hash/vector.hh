/**
 * @file vector.hh
 * @brief `ccc::hash` specializations for std::vector and std::vector<bool>
 *        (Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_VECTOR_HH
#define CCC_HASH_VECTOR_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <type_traits>
#include <vector>

#include "ccc/hash/detail/hash_bits.hh"
#include "ccc/hash/hash.hh"
#include "ccc/hash/utils.hh"

namespace ccc {

template<typename T, typename Allocator>
struct hash<std::vector<T, Allocator>,
            typename std::enable_if<detail::is_hashable<T>::value && !std::is_same<T, bool>::value, void>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::vector<T, Allocator>& value) noexcept(
        detail::is_nothrow_hashable<T>::value)
    {
        std::size_t seed = 0U;
        for (const T& element : value) {
            seed = hash_combine(seed, element);
        }
        return hash_combine(seed, value.size());
    }
};

template<typename Allocator>
struct hash<std::vector<bool, Allocator>, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::vector<bool, Allocator>& value) const noexcept
    {
        return detail::hash_bit_sequence(value, value.size());
    }
};

}  // namespace ccc

#endif  // !CCC_HASH_VECTOR_HH
