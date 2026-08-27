/**
 * @file hash.hh
 * @brief Hash function objects(Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_HASH_HH
#define CCC_HASH_HASH_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <string>
#include <type_traits>

#if (__cplusplus >= 201703L)
#include <string_view>
#endif

#include "ccc/detail/bit_cast_impl.hh"
#include "ccc/detail/murmur_hash3.hh"

namespace ccc {
namespace detail {

template<std::size_t Size>
struct unsigned_integer_of_size {
    using type = typename std::conditional<
        Size == sizeof(unsigned char),
        unsigned char,
        typename std::conditional<
            Size == sizeof(unsigned short),
            unsigned short,
            typename std::conditional<
                Size == sizeof(unsigned int),
                unsigned int,
                typename std::conditional<Size == sizeof(unsigned long), unsigned long, unsigned long long>::type>::
                type>::type>::type type;

    static_assert(sizeof(type) == Size, "No standard unsigned integer type has the required size");
};

template<typename T>
inline CCC_CPP20_CONSTEXPR std::size_t hash_scalar(const T& value, std::true_type) noexcept
{
    return bit_cast_impl<std::size_t>(value);
}

template<typename T>
inline CCC_CPP20_CONSTEXPR std::size_t hash_scalar(const T& value, std::false_type) noexcept
{
    using unsigned_type = typename unsigned_integer_of_size<sizeof(T)>::type;
    return bit_cast_impl<unsigned_type>(value);
}

template<typename CharT>
struct c_string_hash {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const CharT* value) const noexcept
    {
        return value == nullptr ? 0U : murmur_hash3(value, c_string_length(value));
    }
};

}  // namespace detail

template<typename T, typename = void>
struct hash {
    hash() = delete;
    hash(const hash&) = delete;

    hash& operator=(const hash&) = delete;
};

template<typename T>
struct hash<T, typename = std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const T& value) const noexcept
    {
        return detail::hash_scalar(value, std::integral_constant<bool, sizeof(T) == sizeof(std::size_t)>());
    }
};

template<typename T>
struct hash<T, typename = std::enable_if<std::is_enum<T>::value>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const T& value) const noexcept
    {
        using underlying_type = typename std::underlying_type<T>::type;
        return hash<underlying_type>()(static_cast<underlying_type>(value));
    }
};

template<typename T>
struct hash<T*, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(T* value) const noexcept
    {
        return detail::hash_scalar(value, std::integral_constant<bool, sizeof(T*) == sizeof(std::size_t)>());
    }
};

template<>
struct hash<const char*, void> : detail::c_string_hash<char> {
};

template<>
struct hash<const wchar_t*, void> : detail::c_string_hash<wchar_t> {
};

template<>
struct hash<const char16_t*, void> : detail::c_string_hash<char16_t> {
};

template<>
struct hash<const char32_t*, void> : detail::c_string_hash<char32_t> {
};

#if defined(__cpp_char8_t)
template<>
struct hash<const char8_t*, void> : detail::c_string_hash<char8_t> {
};
#endif

template<typename CharT, typename Traits, typename Allocator>
struct hash<std::basic_string<CharT, Traits, Allocator>, void> {
    using string_type = std::basic_string<CharT, Traits, Allocator>;

    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const string_type& value) const noexcept
    {
        return detail::murmur_hash3(value.data(), value.size());
    }
};

#if (__cplusplus >= 201703L)
template<typename CharT, typename Traits>
struct hash<std::basic_string_view<CharT, Traits>, void> {
    using string_view_type = std::basic_string_view<CharT, Traits>;

    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const string_view_type& value) const noexcept
    {
        return detail::murmur_hash3(value.data(), value.size());
    }
};
#endif

}  // namespace ccc

#endif  // !CCC_HASH_HASH_HH
