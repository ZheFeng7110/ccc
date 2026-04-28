#pragma once
#ifndef CCC_DETAIL_CONFIG_HH
#define CCC_DETAIL_CONFIG_HH

#if (__cplusplus < 201403L)
#error "Library `ccc` requires C++14 or higher."
#endif

#if (__cplusplus >= 202002L)
#define CCC_CPP20_CONSTEXPR constexpr
#else
#define CCC_CPP20_CONSTEXPR
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

#endif  // !CCC_DETAIL_CONFIG_HH
