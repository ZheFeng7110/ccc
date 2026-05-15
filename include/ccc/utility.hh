// ReSharper disable CppRedundantTypenameKeyword
// ReSharper disable CppUseTypeTraitAlias

#pragma once
#ifndef CCC_UTILITY_HH
#define CCC_UTILITY_HH

#include "ccc/detail/config.hh"

#ifndef CCC_MODULE_INTERFACE_UNIT
#include <cstddef>

#include <utility>
#include <tuple>
#include <type_traits>
#include <memory>
#endif

namespace ccc {

CCC_MODULE_EXPORT_BEGIN

// C++17 in_place_t tag
#if (__cplusplus < 201703L)
struct in_place_t {
    explicit in_place_t() = default;
};
constexpr in_place_t in_place{};
#else
using in_place_t = std::in_place_t;
constexpr in_place_t in_place{std::in_place};
#endif

// C++17 void_t
#if (__cplusplus < 201703L)
template<typename...>
struct void_type {
    using type = void;
};

template<typename... Ts>
using void_t = typename void_type<Ts...>::type;
#else
template<typename... Ts>
using void_t = std::void_t<Ts...>;
#endif

#if (__cplusplus < 201703L)

template<typename...>
struct conjunction : std::true_type {
};

template<typename B1>
struct conjunction<B1> : B1 {
};

template<typename B1, typename... Bn>
struct conjunction<B1, Bn...> : std::conditional_t<static_cast<bool>(B1::value), conjunction<Bn...>, B1> {
};

template<typename...>
struct disjunction : std::false_type {
};

template<typename B1>
struct disjunction<B1> : B1 {
};

template<typename B1, typename... Bn>
struct disjunction<B1, Bn...> : std::conditional_t<static_cast<bool>(B1::value), B1, disjunction<Bn...>> {
};

template<typename B>
struct negation : std::integral_constant<bool, !static_cast<bool>(B::value)> {
};

#else
template<typename... Bs>
using conjunction = std::conjunction<Bs...>;

template<typename... Bs>
using disjunction = std::disjunction<Bs...>;

template<typename B>
using negation = std::negation<B>;
#endif

template<typename... Bs>
constexpr bool conjunction_v = conjunction<Bs...>::value;

template<typename... Bs>
constexpr bool disjunction_v = disjunction<Bs...>::value;

template<typename Bs>
constexpr bool negation_v = negation<Bs>::value;

#if (__cplusplus >= 202002L)
template<typename T>
using type_identity = std::type_identity<T>;
#else
template<typename T>
struct type_identity {
    using type = T;
};
#endif

#if (__cplusplus >= 202002L)
template<typename T>
using is_bounded_array = std::is_bounded_array<T>;

template<typename T>
using is_unbounded_array = std::is_unbounded_array<T>;
#else
template<typename T>
struct is_bounded_array : std::false_type {
};
template<typename T, std::size_t N>
struct is_bounded_array<T[N]> : std::true_type {
};

template<typename T>
struct is_unbounded_array : std::false_type {
};
template<typename T>
struct is_unbounded_array<T[]> : std::true_type {
};
#endif

#if (__cplusplus >= 201703L)
template<typename T, typename U>
using is_swappable_with = std::is_swappable_with<T, U>;

template<typename T>
using is_swappable = std::is_swappable<T>;

template<typename T, typename U>
using is_nothrow_swappable_with = std::is_nothrow_swappable_with<T, U>;

template<typename T>
using is_nothrow_swappable = std::is_nothrow_swappable<T>;
#else
CCC_MODULE_EXPORT_END

namespace detail {
namespace swappable_impl {

using std::swap;

template<typename T, typename U, typename = void>
struct is_swappable_with_impl : std::false_type {
};

template<typename T, typename U>
struct is_swappable_with_impl<
    T,
    U,
    void_t<decltype(swap(std::declval<T>(), std::declval<U>())), decltype(swap(std::declval<U>(), std::declval<T>()))>>
    : std::true_type {
};

template<typename T, typename U, bool = is_swappable_with_impl<T, U>::value>
struct is_nothrow_swappable_with_impl : std::false_type {
};

template<typename T, typename U>
struct is_nothrow_swappable_with_impl<T, U, true>
    : std::bool_constant<noexcept(swap(std::declval<T>(), std::declval<U>())) &&
                         noexcept(swap(std::declval<U>(), std::declval<T>()))> {
};

}  // namespace swappable_impl
}  // namespace detail

CCC_MODULE_EXPORT_BEGIN

template<typename T, typename U>
using is_swappable_with = detail::swappable_impl::is_swappable_with_impl<T, U>;

template<typename T>
using is_swappable = is_swappable_with<T&, T&>;

template<typename T, typename U>
using is_nothrow_swappable_with = detail::swappable_impl::is_nothrow_swappable_with_impl<T, U>;

template<typename T>
using is_nothrow_swappable = is_nothrow_swappable_with<T&, T&>;
#endif

template<bool b, typename T = void>
using enable_if_t = typename std::enable_if<b, T>::type;

template<bool b, typename IfTrue, typename IfFalse>
using conditional_t = typename std::conditional<b, IfTrue, IfFalse>::type;

#if (__cplusplus < 202002L)
template<typename T>
struct remove_cvref {
    using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};
#else
template<typename T>
using remove_cvref = std::remove_cvref<T>;
#endif

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

#ifdef __cpp_lib_is_nothrow_convertible
template<typename From, typename To>
using is_nothrow_convertible = std::is_nothrow_convertible<From, To>;
#else
template<typename From, typename To, typename = void>
struct is_nothrow_convertible : conjunction<std::is_void<From>, std::is_void<To>> {
};

template<typename From, typename To>
struct is_nothrow_convertible<
    From,
    To,
    enable_if_t<std::is_void<void_t<decltype(static_cast<To (*)()>(nullptr)),
                                    decltype(std::declval<void (&)(To) noexcept>()(std::declval<From>()))>>::value &&
                noexcept(std::declval<void (&)(To) noexcept>()(std::declval<From>()))>> : std::true_type {
};
#endif

template<typename T>
using remove_cv_t = typename std::remove_cv<T>::type;
template<typename T>
using remove_const_t = typename std::remove_const<T>::type;
template<typename T>
using remove_volatile_t = typename std::remove_volatile<T>::type;
template<typename T>
using add_cv_t = typename std::add_cv<T>::type;
template<typename T>
using add_const_t = typename std::add_const<T>::type;
template<typename T>
using add_volatile_t = typename std::add_volatile<T>::type;

template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;
template<typename T>
using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;
template<typename T>
using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;

template<typename T>
using make_signed_t = typename std::make_signed<T>::type;
template<typename T>
using make_unsigned_t = typename std::make_unsigned<T>::type;

template<typename T>
using remove_extent_t = typename std::remove_extent<T>::type;
template<typename T>
using remove_all_extents_t = typename std::remove_all_extents<T>::type;

template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;
template<typename T>
using add_pointer_t = typename std::add_pointer<T>::type;

template<typename T>
using decay_t = typename std::decay<T>::type;
template<typename... Ts>
using common_type_t = typename std::common_type<Ts...>::type;
template<typename T>
using underlying_type_t = typename std::underlying_type<T>::type;

template<typename T>
using type_identity_t = typename type_identity<T>::type;

template<typename T>
constexpr bool is_void_v = std::is_void<T>::value;
template<typename T>
constexpr bool is_null_pointer_v = std::is_null_pointer<T>::value;
template<typename T>
constexpr bool is_integral_v = std::is_integral<T>::value;
template<typename T>
constexpr bool is_floating_point_v = std::is_floating_point<T>::value;
template<typename T>
constexpr bool is_array_v = std::is_array<T>::value;
template<typename T>
constexpr bool is_enum_v = std::is_enum<T>::value;
template<typename T>
constexpr bool is_union_v = std::is_union<T>::value;
template<typename T>
constexpr bool is_class_v = std::is_class<T>::value;
template<typename T>
constexpr bool is_function_v = std::is_function<T>::value;
template<typename T>
constexpr bool is_pointer_v = std::is_pointer<T>::value;
template<typename T>
constexpr bool is_lvalue_reference_v = std::is_lvalue_reference<T>::value;
template<typename T>
constexpr bool is_rvalue_reference_v = std::is_rvalue_reference<T>::value;
template<typename T>
constexpr bool is_member_object_pointer_v = std::is_member_object_pointer<T>::value;
template<typename T>
constexpr bool is_member_function_pointer_v = std::is_member_function_pointer<T>::value;

template<typename T>
constexpr bool is_fundamental_v = std::is_fundamental<T>::value;
template<typename T>
constexpr bool is_arithmetic_v = std::is_arithmetic<T>::value;
template<typename T>
constexpr bool is_scalar_v = std::is_scalar<T>::value;
template<typename T>
constexpr bool is_object_v = std::is_object<T>::value;
template<typename T>
constexpr bool is_compound_v = std::is_compound<T>::value;
template<typename T>
constexpr bool is_reference_v = std::is_reference<T>::value;
template<typename T>
constexpr bool is_member_pointer_v = std::is_member_pointer<T>::value;

template<typename T>
constexpr bool is_const_v = std::is_const<T>::value;
template<typename T>
constexpr bool is_volatile_v = std::is_volatile<T>::value;
template<typename T>
constexpr bool is_trivial_v = std::is_trivial<T>::value;
template<typename T>
constexpr bool is_trivially_copyable_v = std::is_trivially_copyable<T>::value;
template<typename T>
constexpr bool is_standard_layout_v = std::is_standard_layout<T>::value;
template<typename T>
constexpr bool is_empty_v = std::is_empty<T>::value;
template<typename T>
constexpr bool is_polymorphic_v = std::is_polymorphic<T>::value;
template<typename T>
constexpr bool is_abstract_v = std::is_abstract<T>::value;
template<typename T>
constexpr bool is_final_v = std::is_final<T>::value;
template<typename T>
constexpr bool is_signed_v = std::is_signed<T>::value;
template<typename T>
constexpr bool is_unsigned_v = std::is_unsigned<T>::value;
template<typename T>
constexpr bool is_bounded_array_v = is_bounded_array<T>::value;
template<typename T>
constexpr bool is_unbounded_array_v = is_unbounded_array<T>::value;

template<typename T, typename... Args>
constexpr bool is_constructible_v = std::is_constructible<T, Args...>::value;
template<typename T, typename... Args>
constexpr bool is_trivially_constructible_v = std::is_trivially_constructible<T, Args...>::value;
template<typename T, typename... Args>
constexpr bool is_nothrow_constructible_v = std::is_nothrow_constructible<T, Args...>::value;
template<typename T>
constexpr bool is_default_constructible_v = std::is_default_constructible<T>::value;
template<typename T>
constexpr bool is_trivially_default_constructible_v = std::is_trivially_default_constructible<T>::value;
template<typename T>
constexpr bool is_nothrow_default_constructible_v = std::is_nothrow_default_constructible<T>::value;
template<typename T>
constexpr bool is_copy_constructible_v = std::is_copy_constructible<T>::value;
template<typename T>
constexpr bool is_trivially_copy_constructible_v = std::is_trivially_copy_constructible<T>::value;
template<typename T>
constexpr bool is_nothrow_copy_constructible_v = std::is_nothrow_copy_constructible<T>::value;
template<typename T>
constexpr bool is_move_constructible_v = std::is_move_constructible<T>::value;
template<typename T>
constexpr bool is_trivially_move_constructible_v = std::is_trivially_move_constructible<T>::value;
template<typename T>
constexpr bool is_nothrow_move_constructible_v = std::is_nothrow_move_constructible<T>::value;
template<typename Left, typename Right>
constexpr bool is_assignable_v = std::is_assignable<Left, Right>::value;
template<typename Left, typename Right>
constexpr bool is_trivially_assignable_v = std::is_trivially_assignable<Left, Right>::value;
template<typename Left, typename Right>
constexpr bool is_nothrow_assignable_v = std::is_nothrow_assignable<Left, Right>::value;
template<typename T>
constexpr bool is_copy_assignable_v = std::is_copy_assignable<T>::value;
template<typename T>
constexpr bool is_trivially_copy_assignable_v = std::is_trivially_copy_assignable<T>::value;
template<typename T>
constexpr bool is_nothrow_copy_assignable_v = std::is_nothrow_copy_assignable<T>::value;
template<typename T>
constexpr bool is_move_assignable_v = std::is_move_assignable<T>::value;
template<typename T>
constexpr bool is_trivially_move_assignable_v = std::is_trivially_move_assignable<T>::value;
template<typename T>
constexpr bool is_nothrow_move_assignable_v = std::is_nothrow_move_assignable<T>::value;
template<typename T>
constexpr bool is_destructible_v = std::is_destructible<T>::value;
template<typename T>
constexpr bool is_trivially_destructible_v = std::is_trivially_destructible<T>::value;
template<typename T>
constexpr bool is_nothrow_destructible_v = std::is_nothrow_destructible<T>::value;
template<typename T>
constexpr bool has_virtual_destructor_v = std::has_virtual_destructor<T>::value;
template<typename T, typename U>
constexpr bool is_swappable_with_v = is_swappable_with<T, U>::value;
template<typename T>
constexpr bool is_swappable_v = is_swappable<T>::value;
template<typename T, typename U>
constexpr bool is_nothrow_swappable_with_v = is_nothrow_swappable_with<T, U>::value;
template<typename T>
constexpr bool is_nothrow_swappable_v = is_nothrow_swappable<T>::value;

template<typename T, typename U>
constexpr bool is_same_v = std::is_same<T, U>::value;
template<typename Base, typename Derived>
constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;
template<typename From, typename To>
constexpr bool is_convertible_v = std::is_convertible<From, To>::value;
template<typename From, typename To>
constexpr bool is_nothrow_convertible_v = is_nothrow_convertible<From, To>::value;

template<typename Tuple>
constexpr std::size_t tuple_size_v = std::tuple_size<Tuple>::value;
template<std::size_t I, typename Tuple>
using tuple_element_t = typename std::tuple_element<I, Tuple>::type;

CCC_MODULE_EXPORT_END

#if (__cplusplus < 202002L)
namespace detail {

struct construct_at_is_array_type_tag {
};
struct construct_at_is_not_array_type_tag {
};

template<typename T, typename... Args>
inline T* construct_at(construct_at_is_array_type_tag, T* location, Args&&...)
{
    static_assert(std::is_array<T>::value,
                  "Internal Error: construct_at with construct_at_array_type_tag can only be used for array types");
    static_assert(sizeof...(Args) == 0,
                  "construct_at for array types must not use any arguments to initialize the array");

    void* const address = location;
    return ::new (address) T[1]();
}

template<typename T, typename... Args>
inline T* construct_at(construct_at_is_not_array_type_tag, T* location, Args&&... args)
{
    static_assert(
        !std::is_array<T>::value,
        "Internal Error: construct_at with construct_at_is_not_array_type_tag can only be used for non-array types");
    void* const address = location;
    return ::new (address) T(std::forward<Args>(args)...);
}

}  // namespace detail
#endif  // (__cplusplus < 202002L)

CCC_MODULE_EXPORT_BEGIN

template<typename T, typename... Args>
inline CCC_CPP20_CONSTEXPR T* construct_at(T* location,
                                           Args&&... args) noexcept(noexcept(::new ((void*)0)
                                                                                 T(std::declval<Args>()...)))
{
#if (__cplusplus < 202002L)
    static_assert(!is_unbounded_array_v<T>, "construct_at does not support unbounded array types");
    return detail::construct_at(conditional_t<is_array_v<T>,
                                              detail::construct_at_is_array_type_tag,
                                              detail::construct_at_is_not_array_type_tag>{},
                                location,
                                std::forward<Args>(args)...);
#else
    return std::construct_at(location, std::forward<Args>(args)...);
#endif
}

template<typename T>
inline CCC_CPP20_CONSTEXPR void destroy_at(T* location)
{
#if (__cplusplus < 201703L)
    static_assert(!std::is_array<T>::value, "destroy_at does not support array types before C++20");
    location->~T();
#else
    std::destroy_at(location);
#endif
}

template<typename ForwardIt>
CCC_CPP20_CONSTEXPR void destroy(ForwardIt first, ForwardIt last)
{
#if (__cplusplus < 201703L)
    for (; first != last; ++first) {
        destroy_at(std::addressof(*first));
    }
#else
    std::destroy(first, last);
#endif
}

template<typename T>
inline constexpr add_const_t<T>& as_const(T& t) noexcept
{
    return t;
}

template<typename T>
inline constexpr add_const_t<T>& as_const(const T&& t) noexcept = delete;

CCC_MODULE_EXPORT_END

}  // namespace ccc

#endif  // !CCC_UTILITY_HH
