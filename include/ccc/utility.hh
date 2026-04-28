#pragma once
#ifndef CCC_UTILITY_HH
#define CCC_UTILITY_HH

#include "ccc/detail/config.hh"

#ifdef CCC_MODULE_INTERFACE_UNIT
#include <utility>
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
struct negation : std::bool_constant<!static_cast<bool>(B::value)> {
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

CCC_MODULE_EXPORT_END

}  // namespace ccc

#endif  // !CCC_UTILITY_HH
