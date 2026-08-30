/**
 * @file test_pch.hh
 * @brief
 */

#pragma once
#ifndef CCC_TEST_TEST_PCH_HH
#define CCC_TEST_TEST_PCH_HH

#if defined(__APPLE__) && defined(__GNUC__) && !defined(__clang__)
// Workaround: GCC on macOS with SDK >= 15 -- <string.h> uses rsize_t from
// C11 Annex K (<sys/_types/_rsize_t.h>), but GCC's libstdc++ does not set
// __STDC_WANT_LIB_EXT1__ in C++20+ mode.  Define it here before any
// system header is pulled in (this PCH is injected via -include).
#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif
#endif

#if defined(__clang__) && __clang_major__ >= 22
// Catch2 v2's TEST_CASE expands __COUNTER__, which clang 22 reports as a
// C2y extension under -Wpedantic
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#ifdef __cplusplus

// Catch2 v2.13.10 uses std::nothrow (catch.hpp) without including <new>;
// libc++ 22 no longer provides it transitively in C++23 mode.
#include <new>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 5311)
#endif

#include <catch2/catch.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

#endif  // !CCC_TEST_TEST_PCH_HH
