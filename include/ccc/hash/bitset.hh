/**
 * @file bitset.hh
 * @brief `ccc::hash` specialization for std::bitset (Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_BITSET_HH
#define CCC_HASH_BITSET_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <bitset>

#include "ccc/hash/detail/hash_bits.hh"
#include "ccc/hash/hash.hh"

namespace ccc {

template<std::size_t Bits>
struct hash<std::bitset<Bits>, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::bitset<Bits>& value) const noexcept
    {
        return detail::hash_bit_sequence(value, Bits);
    }
};

}  // namespace ccc

#endif  // !CCC_HASH_BITSET_HH
