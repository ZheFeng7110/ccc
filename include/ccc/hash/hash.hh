/**
 * @file hash.hh
 * @brief Hash function objects(Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_HASH_HH
#define CCC_HASH_HASH_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <type_traits>
#include <utility>

#include "ccc/detail/bit_cast_impl.hh"

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
                type>::type>::type;

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

template<typename T>
inline CCC_CPP20_CONSTEXPR std::size_t hash_arithmetic(const T& value) noexcept
{
    if (std::is_floating_point<T>::value && value == T(0)) {
        return 0U;
    }
    return hash_scalar(value, std::integral_constant<bool, sizeof(T) == sizeof(std::size_t)>());
}

}  // namespace detail

template<typename T, typename = void>
struct hash {
    hash() = delete;
    hash(const hash&) = delete;

    hash& operator=(const hash&) = delete;
};

template<typename T>
struct hash<T, typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const T& value) const noexcept
    {
        return detail::hash_arithmetic(value);
    }
};

template<typename T>
struct hash<T, typename std::enable_if<std::is_enum<T>::value>::type> {
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
struct hash<std::nullptr_t, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(std::nullptr_t) const noexcept
    {
        return 0U;
    }
};

namespace detail {

// C++17 void_t, scoped to the hash component to keep it self-contained
template<typename...>
struct hash_void {
    using type = void;
};

template<typename... Ts>
using hash_void_t = typename hash_void<Ts...>::type;

// True when `ccc::hash<T>` is an enabled specialization (i.e. callable)
template<typename T, typename = void>
struct is_hashable : std::false_type {
};

template<typename T>
struct is_hashable<T, hash_void_t<decltype(std::declval<hash<T>&>()(std::declval<const T&>()))>> : std::true_type {
};

template<typename T, typename = void>
struct is_nothrow_hashable : std::false_type {
};

template<typename T>
struct is_nothrow_hashable<T, hash_void_t<decltype(std::declval<hash<T>&>()(std::declval<const T&>()))>>
    : std::integral_constant<bool, noexcept(std::declval<hash<T>&>()(std::declval<const T&>()))> {
};

// Conjunctions over a pack of element types, for composite specializations
template<typename... Ts>
struct all_hashable : std::true_type {
};

template<typename T, typename... Rest>
struct all_hashable<T, Rest...> : std::integral_constant<bool, is_hashable<T>::value && all_hashable<Rest...>::value> {
};

template<typename... Ts>
struct all_nothrow_hashable : std::true_type {
};

template<typename T, typename... Rest>
struct all_nothrow_hashable<T, Rest...>
    : std::integral_constant<bool, is_nothrow_hashable<T>::value && all_nothrow_hashable<Rest...>::value> {
};

}  // namespace detail

}  // namespace ccc

#endif  // !CCC_HASH_HASH_HH
