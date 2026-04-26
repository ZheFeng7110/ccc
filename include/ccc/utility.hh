#pragma once
#ifndef CCC_UTILITY_HH
#define CCC_UTILITY_HH

#include "ccc/detail/config.hh"

#ifdef CCC_MODULE_INTERFACE_UNIT
#include <utility>
#endif

CCC_MODULE_EXPORT
namespace ccc {

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

}  // namespace ccc

#endif  // !CCC_UTILITY_HH
