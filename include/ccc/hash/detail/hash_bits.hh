/**
 * @file hash_bits.hh
 * @brief Internal helper for hashing bit sequences (vector<bool>, bitset)
 */

#pragma once
#ifndef CCC_HASH_DETAIL_HASH_BITS_HH
#define CCC_HASH_DETAIL_HASH_BITS_HH

#include "ccc/detail/config.hh"

#include <cstddef>

#include "ccc/hash/hash.hh"
#include "ccc/hash/utils.hh"

namespace ccc {
namespace detail {

/**
 * @brief Hashes a bit sequence by packing bits into words (LSB-first) and
 *        combining them with hash_combine.
 * @param bits A container addressable by `operator[]`, whose elements are
 *             convertible to bool (e.g. std::vector<bool>, std::bitset).
 * @param bit_count Number of significant bits in the sequence.
 * @return The combined hash value, length-sensitive via the bit count.
 */
template<typename BitContainer>
inline CCC_CPP20_CONSTEXPR std::size_t hash_bit_sequence(const BitContainer& bits, std::size_t bit_count) noexcept
{
    const std::size_t bits_per_word = sizeof(std::size_t) * 8U;

    std::size_t seed = 0U;
    std::size_t word = 0U;
    std::size_t word_bits = 0U;

    for (std::size_t bit_index = 0; bit_index < bit_count; ++bit_index) {
        word |= static_cast<std::size_t>(bits[bit_index] ? 1U : 0U) << word_bits;
        ++word_bits;
        if (word_bits == bits_per_word) {
            seed = hash_combine(seed, word);
            word = 0U;
            word_bits = 0U;
        }
    }

    if (word_bits != 0U) {
        seed = hash_combine(seed, word);
    }

    return hash_combine(seed, bit_count);
}

}  // namespace detail
}  // namespace ccc

#endif  // !CCC_HASH_DETAIL_HASH_BITS_HH
