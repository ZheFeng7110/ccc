/**
 * @file criterion_traits.hh
 * @brief
 */

#pragma once
#ifndef CCC_EXPECTED_CRITERION_TRAITS_HH
#define CCC_EXPECTED_CRITERION_TRAITS_HH

#include "ccc/expected/base.hh"

namespace ccc {

namespace detail {

template<typename, typename = void, typename = void>
struct is_valid_criterion_type : std::false_type {
};

template<typename T>
constexpr bool criterion_type_is_nothrow_callable() noexcept
{
    CCC_MAYBE_UNUSED T v{};
    return noexcept(v.has_value()) && noexcept(v == v);
}

template<typename T>
struct is_valid_criterion_type<
    T,
    void_t<typename T::value_type,
           decltype(std::declval<T>().has_value()),
           decltype(T::default_error_value),
           decltype(std::declval<T>() == std::declval<T>())>,
    enable_if_t<is_nothrow_default_constructible_v<T> && is_nothrow_constructible_v<T, typename T::value_type> &&
                is_convertible_v<decltype(std::declval<T>().has_value()), bool> &&
                is_convertible_v<decltype(std::declval<T>() == std::declval<T>()), bool> &&
                criterion_type_is_nothrow_callable<T>()>> : std::true_type {
};

template<typename T>
constexpr bool is_valid_criterion_type_v = is_valid_criterion_type<T>::value;

template<typename T>
inline constexpr bool criterion_type_default_init_has_value() noexcept
{
    T v{};
    return v.has_value();
}

template<typename T>
inline constexpr bool criterion_type_default_error_init_has_not_value() noexcept
{
    T v{T::default_error_value};
    return !v.has_value();
}

}  // namespace detail

CCC_MODULE_EXPORT_BEGIN

template<typename T>
struct is_criterion
    : std::bool_constant<detail::is_valid_criterion_type_v<T> && detail::criterion_type_default_init_has_value<T>() &&
                         detail::criterion_type_default_error_init_has_not_value<T>()> {
};

template<typename T>
constexpr bool is_criterion_v = is_criterion<T>::value;

CCC_MODULE_EXPORT_END

}  // namespace ccc

#endif  // !CCC_EXPECTED_CRITERION_TRAITS_HH
