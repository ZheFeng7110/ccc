#pragma once
#ifndef CCC_DETAIL_CONFIG_HH
#define CCC_DETAIL_CONFIG_HH

#if (__cplusplus < 201103L)
#error "Library `ccc` requires C++11 or higher."
#endif

#if (__cpp_constexpr >= 202400L)
#define CCC_CPP26_CONSTEXPR constexpr
#else
#define CCC_CPP26_CONSTEXPR
#endif

#if (__cplusplus >= 202002L)
#define CCC_CPP20_CONSTEXPR constexpr
#else
#define CCC_CPP20_CONSTEXPR
#endif

#if (__cplusplus >= 201703L)
#define CCC_CPP17_CONSTEXPR constexpr
#else
#define CCC_CPP17_CONSTEXPR
#endif

#if (__cplusplus >= 202002L)
#define CCC_CONDITIONAL_EXPLICIT(condition) explicit(condition)
#else
#define CCC_CONDITIONAL_EXPLICIT(condition) explicit
#endif

#if (__cplusplus >= 201703L)
#define CCC_NO_DISCARD [[nodiscard]]
#else
#define CCC_NO_DISCARD
#endif

#if (__cplusplus >= 201703L)
#define CCC_MAYBE_UNUSED [[maybe_unused]]
#else
#define CCC_MAYBE_UNUSED
#endif

#ifndef CCC_USE_MODULES
#define CCC_USE_MODULES 0
#endif

#ifdef _MSC_VER
#define CCC_WARNING(msg) __pragma(message(msg))
#elif (defined(__clang__) || defined(__GNUC__))
#define CCC_DETAIL_WARNING_TO_STRING_(x) #x
#define CCC_WARNING(msg)                 _Pragma(CCC_DETAIL_WARNING_TO_STRING_(GCC warning msg))
#else
#define CCC_WARNING(msg) static_assert(false, msg);
#endif

#if (__cplusplus >= 202302L)
#define CCC_IF_IN_CONSTEVAL     if consteval
#define CCC_IF_NOT_IN_CONSTEVAL if !consteval
#elif (__cplusplus >= 202002L)
#define CCC_IF_IN_CONSTEVAL     if (std::is_constant_evaluated())
#define CCC_IF_NOT_IN_CONSTEVAL if (!std::is_constant_evaluated())
#else
#define CCC_IF_IN_CONSTEVAL     if (false)
#define CCC_IF_NOT_IN_CONSTEVAL if (true)
#endif

#endif  // !CCC_DETAIL_CONFIG_HH
