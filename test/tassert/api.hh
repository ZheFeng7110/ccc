#pragma once
#ifndef CCC_TEST_TASSERT_API_HH
#define CCC_TEST_TASSERT_API_HH

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(TEST_IMPORT_ALL)
import ccc;
#elif defined(TEST_IMPORT_TASSERT)
import ccc.tassert;
#else
#include "ccc/tassert.hh"
#endif

inline constexpr ccc::tassert_tag active{"active"};
inline constexpr ccc::tassert_tag disabled{"disabled"};

#endif  // !CCC_TEST_TASSERT_API_HH
