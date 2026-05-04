/**
 * @file expected.hh
 * @brief
 */

#pragma once
#ifndef CCC_EXPECTED_HH
#define CCC_EXPECTED_HH

#if (__cplusplus >= 201402L)

#include "ccc/detail/config.hh"

#include "ccc/expected/base.hh"
#include "ccc/expected/criterion_traits.hh"
#include "ccc/expected/unexpected.hh"
#include "ccc/expected/expected_impl.hh"

#else

#ifndef CCC_SUPRESS_WARNINGS
#warning "ccc::expected requires C++14 or later"
#endif

#endif

#endif  // !CCC_EXPECTED_HH
