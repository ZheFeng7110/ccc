/**
 * @file base.hh
 * @brief
 */

#pragma once
#ifndef CCC_EXPECTED_BASE_HH
#define CCC_EXPECTED_BASE_HH

#include "ccc/detail/config.hh"

#include "ccc/utility.hh"

#ifdef CCC_MODULE_INTERFACE_UNIT
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>
#include <algorithm>

#if (__cplusplus >= 202002L)
#include <compare>
#endif
#endif

#endif  // !CCC_EXPECTED_BASE_HH
