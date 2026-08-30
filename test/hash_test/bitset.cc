/**
 * @file bitset.cc
 * @brief Tests for ccc::hash specialization of std::bitset
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <bitset>
#include <cstddef>
#include <type_traits>

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

#if (__cplusplus >= 202002L)
static_assert(ccc::hash<std::bitset<8>>()(std::bitset<8>(0xFF)) == ccc::hash<std::bitset<8>>()(std::bitset<8>(0xFF)),
              "ccc::hash for bitset must be constexpr in C++20");
#endif

TEST_CASE("Hash - Bitset")
{
    using byte_bitset = std::bitset<8>;
    using big_bitset = std::bitset<300>;

    CHECK(ccc::hash<byte_bitset>()(byte_bitset{0xFF}) == ccc::hash<byte_bitset>()(byte_bitset{0xFF}));
    CHECK(ccc::hash<byte_bitset>()(byte_bitset{0xFF}) != ccc::hash<byte_bitset>()(byte_bitset{0x00}));
    CHECK(ccc::hash<byte_bitset>()(byte_bitset{0x01}) != ccc::hash<byte_bitset>()(byte_bitset{0x80}));

    // Bits beyond one word (64 bits) must participate in the hash
    const big_bitset high = big_bitset{1} << 299;
    const big_bitset low = big_bitset{1};
    CHECK(ccc::hash<big_bitset>()(high) == ccc::hash<big_bitset>()(high));
    CHECK(ccc::hash<big_bitset>()(high) != ccc::hash<big_bitset>()(low));
    CHECK(ccc::hash<big_bitset>()(big_bitset{}) == ccc::hash<big_bitset>()(big_bitset{}));
}
