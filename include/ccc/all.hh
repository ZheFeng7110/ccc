/**
 * @file all.hh
 * @brief Header for all components in ccc
 */

#pragma once
#ifndef CCC_ALL_HH
#define CCC_ALL_HH

#include "ccc/detail/config.hh"

#include "ccc/utility.hh"
#include "ccc/resource_guard.hh"
#include "ccc/hash.hh"
#include "ccc/pipe_operator_helper.hh"
#include "ccc/when.hh"

#if __cplusplus >= 202002L
#include "ccc/tassert.hh"
#endif

#endif  // !CCC_ALL_HH
