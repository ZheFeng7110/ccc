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
namespace is_criterion_impl {

template<typename T>
constexpr bool criterion_type_is_nothrow_callable() noexcept
{
    CCC_MAYBE_UNUSED T v{};
    return noexcept(v.has_value()) && noexcept(v == v);
}

#ifdef __cpp_concepts

template<typename T>
concept is_valid_criterion_type_v =
    requires(T v) {
        typename T::value_type;
        { v.has_value() } -> std::convertible_to<bool>;
        T::default_error_value;
        { v == v } -> std::convertible_to<bool>;
    } && is_nothrow_default_constructible_v<T> && is_nothrow_constructible_v<T, typename T::value_type> &&
    criterion_type_is_nothrow_callable<T>();

#else

template<typename, typename = void, typename = void>
struct is_valid_criterion_type : std::false_type {
};

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
                criterion_type_is_nothrow_callable<T>()> > : std::true_type {
};

template<typename T>
constexpr bool is_valid_criterion_type_v = is_valid_criterion_type<T>::value;
#endif

template<typename T>
inline constexpr bool default_init_has_value() noexcept
{
    T v{};
    return v.has_value();
}

template<typename T>
inline constexpr bool default_error_init_has_not_value() noexcept
{
    T v{T::default_error_value};
    return !v.has_value();
}

}  // namespace is_criterion_impl
}  // namespace detail

template<typename T>
constexpr bool is_criterion_v =
    detail::is_criterion_impl::is_valid_criterion_type_v<T> && detail::is_criterion_impl::default_init_has_value<T>() &&
    detail::is_criterion_impl::default_error_init_has_not_value<T>();

}  // namespace ccc

#endif  // !CCC_EXPECTED_CRITERION_TRAITS_HH
