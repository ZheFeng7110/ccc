/**
 * @file assertions.hh
 * @brief
 */

#pragma once
#ifndef CCC_DETAIL_ASSERTIONS_HH
#define CCC_DETAIL_ASSERTIONS_HH

#if (__cplusplus >= 202002L)

#define CCC_DETAIL_ASSERT(cond, msg) ::ccc::contracts::require((cond), (msg))

#else

#include <cassert>

#define CCC_DETAIL_ASSERT(cond, msg) assert((cond) && (msg))

#endif

#endif  // !CCC_DETAIL_ASSERTIONS_HH
