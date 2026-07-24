/**
 * @file bit_cast_impl.hh
 * @brief Provides 'bit_cast' for C++ <= 20
 */

// ReSharper disable CppRedundantInlineSpecifier

#pragma once
#ifndef CCC_PIPE_OPERATOR_HELPER_BIT_CAST_IMPL_HH
#define CCC_PIPE_OPERATOR_HELPER_BIT_CAST_IMPL_HH

#include "ccc/detail/config.hh"

#if (__cplusplus >= 202002L)  // C++20
#include <bit>
#else
#include <cstring>
#include <type_traits>
#endif

namespace ccc {
namespace pipe_operator_helper {
namespace detail {

template<typename To, typename From>
inline CCC_CPP20_CONSTEXPR To bit_cast_impl(const From& from) noexcept
{
#if (__cplusplus >= 202002L)  // C++20
    return std::bit_cast<To>(from);
#else
    static_assert(sizeof(From) == sizeof(To), "The size of type `From` should be equals to type `To`");
    static_assert(std::is_trivially_copyable<From>::value, "The type `From` should be trivially copyable");
    static_assert(std::is_trivially_copyable<To>::value, "The type `To` should be trivially copyable");
    static_assert(std::is_trivially_constructible<To>::value,
                  "This implementation additionally requires destination type to be trivially constructible");

    To to;
    std::memcpy(&to, &from, sizeof(To));
    return to;
#endif
}

}  // namespace detail
}  // namespace pipe_operator_helper
}  // namespace ccc

#endif  // !CCC_PIPE_OPERATOR_HELPER_BIT_CAST_IMPL_HH
