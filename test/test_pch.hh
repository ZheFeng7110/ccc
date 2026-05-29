/**
 * @file test_pch.hh
 * @brief
 */

#pragma once
#ifndef CCC_TEST_TEST_PCH_HH
#define CCC_TEST_TEST_PCH_HH

#if defined(__APPLE__) && defined(__GNUC__) && !defined(__clang__)
// Workaround: GCC on macOS with SDK >= 15 — <string.h> uses rsize_t from
// C11 Annex K (<sys/_types/_rsize_t.h>) via _DARWIN_C_SOURCE, but GCC's libstdc++ does not set
// _DARWIN_C_SOURCE in C++20+ mode.  Define it here before any
// system header is pulled in (this PCH is injected via -include).
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE 1
#endif
#endif

#ifdef __cplusplus
#include <gtest/gtest.h>
#endif

#endif  // !CCC_TEST_TEST_PCH_HH
