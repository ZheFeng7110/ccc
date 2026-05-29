/**
 * @file test_pch.hh
 * @brief
 */

#pragma once
#ifndef CCC_TEST_TEST_PCH_HH
#define CCC_TEST_TEST_PCH_HH

#if defined(__APPLE__) && defined(__GNUC__) && !defined(__clang__)
// Workaround: GCC on macOS with SDK >= 15 — <string.h> references rsize_t
// from Annex K which GCC's libstdc++ does not declare.  Including
// <sys/types.h> forces the type chain to resolve before gtest pulls in
// <string.h> via its portability header.
#include <sys/types.h>
#endif

#ifdef __cplusplus
#include <gtest/gtest.h>
#endif

#endif  // !CCC_TEST_TEST_PCH_HH