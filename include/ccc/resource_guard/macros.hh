/**
 * @file macros.hh
 * @brief Macros for resource guard utilities.
 */

#pragma once
#ifndef CCC_RESOURCE_GUARD_MACROS_HH
#define CCC_RESOURCE_GUARD_MACROS_HH

#include "ccc/resource_guard/impl.hh"

#if (__cplusplus >= 201703L)

#define CCC_DETAIL_DEFER_NAME_(line) CCC_DETAIL_DEFER_NAME_IMPL_(line)
#define CCC_DETAIL_DEFER_NAME_IMPL_(line) ccc_detail_defer_name_##line##_

// Usage: CCC_DEFER({ resource.release(); });
#define CCC_DEFER(...) ::ccc::defer CCC_DETAIL_DEFER_NAME_(__LINE__) { [&] __VA_ARGS__ }

#endif  // (__cplusplus >= 201703L)

#endif  // !CCC_RESOURCE_GUARD_MACROS_HH
