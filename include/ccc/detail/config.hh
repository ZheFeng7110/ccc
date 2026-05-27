#pragma once
#ifndef CCC_DETAIL_CONFIG_HH
#define CCC_DETAIL_CONFIG_HH

#if (__cplusplus < 201103L)
#error "Library `ccc` requires C++11 or higher."
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

#ifdef CCC_MODULE_INTERFACE_UNIT

/* clang-format off */
#define CCC_MODULE_EXPORT       export
#define CCC_MODULE_EXPORT_BEGIN export {
#define CCC_MODULE_EXPORT_END   }
/* clang-format on */

#else  // CCC_MODULE_INTERFACE_UNIT

#define CCC_MODULE_EXPORT
#define CCC_MODULE_EXPORT_BEGIN
#define CCC_MODULE_EXPORT_END

#endif  // !CCC_MODULE_INTERFACE_UNIT

#ifdef _MSC_VER
#define CCC_WARNING(msg) __pragma(message(msg))
#elif (defined(__clang__) || defined(__GNUC__))
#define CCC_DETAIL_WARNING_TO_STRING_(x) #x
#define CCC_WARNING(msg)                 _Pragma(CCC_DETAIL_WARNING_TO_STRING_(GCC warning msg))
#else
#define CCC_WARNING(msg) static_assert(false, msg);
#endif

#endif  // !CCC_DETAIL_CONFIG_HH
