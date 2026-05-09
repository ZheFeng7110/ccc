/**
 * @file base.hh
 * @brief
 */

#pragma once
#ifndef CCC_EXPECTED_BASE_HH
#define CCC_EXPECTED_BASE_HH

#include "ccc/detail/config.hh"

#ifndef CCC_MODULE_INTERFACE_UNIT
#include "ccc/utility.hh"

#include <cstddef>

#include <array>
#include <tuple>
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <initializer_list>

#if (__cplusplus >= 202002L)
#include <compare>
#include <concepts>
#endif
#endif

#endif  // !CCC_EXPECTED_BASE_HH
