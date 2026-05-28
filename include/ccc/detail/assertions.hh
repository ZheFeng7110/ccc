/**
 * @file assertions.hh
 * @brief
 */

#pragma once
#ifndef CCC_DETAIL_ASSERTIONS_HH
#define CCC_DETAIL_ASSERTIONS_HH

#ifdef __cpp_contracts

#define CCC_DETAIL_ASSERT(cond, msg) contract_assert((cond) && (msg))

#else

#include <cassert>

#define CCC_DETAIL_ASSERT(cond, msg) assert((cond) && (msg))

#endif

#endif  // !CCC_DETAIL_ASSERTIONS_HH
