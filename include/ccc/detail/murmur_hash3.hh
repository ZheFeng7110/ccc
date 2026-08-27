/**
 * @file murmur_hash3.hh
 * @brief Provides the MurmurHash3 x86 32-bit algorithm for character sequences
 */

#pragma once
#ifndef CCC_DETAIL_MURMUR_HASH3_HH
#define CCC_DETAIL_MURMUR_HASH3_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <cstdint>

#include <type_traits>

namespace ccc {
namespace detail {

inline CCC_CPP20_CONSTEXPR std::uint32_t murmur_hash3_rotate_left(std::uint32_t value, std::uint32_t count) noexcept
{
    return (value << count) | (value >> (32U - count));
}

template<typename CharT>
inline CCC_CPP20_CONSTEXPR std::uint8_t murmur_hash3_byte_at(const CharT* data, std::size_t byte_index) noexcept
{
    using unsigned_char_type = typename std::make_unsigned<CharT>::type;

    const std::uintmax_t character =
        static_cast<std::uintmax_t>(static_cast<unsigned_char_type>(data[byte_index / sizeof(CharT)]));
    const std::size_t character_byte_index = byte_index % sizeof(CharT);
    return static_cast<std::uint8_t>((character >> (character_byte_index * 8U)) & 0xffU);
}

inline CCC_CPP20_CONSTEXPR std::uint32_t murmur_hash3_finalize(std::uint32_t value) noexcept
{
    value ^= value >> 16U;
    value *= 0x85ebca6bU;
    value ^= value >> 13U;
    value *= 0xc2b2ae35U;
    value ^= value >> 16U;
    return value;
}

template<typename CharT>
inline CCC_CPP20_CONSTEXPR std::size_t murmur_hash3(const CharT* data, std::size_t length) noexcept
{
    const std::size_t byte_length = length * sizeof(CharT);
    const std::size_t block_count = byte_length / 4U;
    std::uint32_t hash = 0U;

    for (std::size_t block_index = 0; block_index < block_count; ++block_index) {
        const std::size_t byte_index = block_index * 4U;
        std::uint32_t block = static_cast<std::uint32_t>(murmur_hash3_byte_at(data, byte_index));
        block |= static_cast<std::uint32_t>(murmur_hash3_byte_at(data, byte_index + 1U)) << 8U;
        block |= static_cast<std::uint32_t>(murmur_hash3_byte_at(data, byte_index + 2U)) << 16U;
        block |= static_cast<std::uint32_t>(murmur_hash3_byte_at(data, byte_index + 3U)) << 24U;

        block *= 0xcc9e2d51U;
        block = murmur_hash3_rotate_left(block, 15U);
        block *= 0x1b873593U;

        hash ^= block;
        hash = murmur_hash3_rotate_left(hash, 13U);
        hash = hash * 5U + 0xe6546b64U;
    }

    const std::size_t tail_index = block_count * 4U;
    const std::size_t tail_length = byte_length % 4U;
    std::uint32_t tail = 0U;

    if (tail_length == 3U) {
        tail ^= static_cast<std::uint32_t>(murmur_hash3_byte_at(data, tail_index + 2U)) << 16U;
    }
    if (tail_length >= 2U) {
        tail ^= static_cast<std::uint32_t>(murmur_hash3_byte_at(data, tail_index + 1U)) << 8U;
    }
    if (tail_length >= 1U) {
        tail ^= static_cast<std::uint32_t>(murmur_hash3_byte_at(data, tail_index));
        tail *= 0xcc9e2d51U;
        tail = murmur_hash3_rotate_left(tail, 15U);
        tail *= 0x1b873593U;
        hash ^= tail;
    }

    hash ^= static_cast<std::uint32_t>(byte_length);
    return static_cast<std::size_t>(murmur_hash3_finalize(hash));
}

template<typename CharT>
inline CCC_CPP20_CONSTEXPR std::size_t c_string_length(const CharT* value) noexcept
{
    std::size_t length = 0U;
    while (value[length] != CharT()) {
        ++length;
    }
    return length;
}

}  // namespace detail
}  // namespace ccc

#endif  // !CCC_DETAIL_MURMUR_HASH3_HH
