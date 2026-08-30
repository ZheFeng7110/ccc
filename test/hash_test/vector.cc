/**
 * @file vector.cc
 * @brief Tests for ccc::hash specializations of std::vector and std::vector<bool>
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

#if (__cplusplus >= 202002L)
static_assert(ccc::hash<std::vector<int>>()({1, 2, 3}) == ccc::hash<std::vector<int>>()({1, 2, 3}),
              "ccc::hash for vector must be constexpr in C++20");
#endif

TEST_CASE("Hash - Vector")
{
    using int_vector = std::vector<int>;
    using string_vector = std::vector<std::string>;

    CHECK(ccc::hash<int_vector>()(int_vector{1, 2, 3}) == ccc::hash<int_vector>()(int_vector{1, 2, 3}));
    CHECK(ccc::hash<int_vector>()(int_vector{1, 2, 3}) != ccc::hash<int_vector>()(int_vector{3, 2, 1}));
    CHECK(ccc::hash<int_vector>()(int_vector{1, 2}) != ccc::hash<int_vector>()(int_vector{1, 2, 0}));
    CHECK(ccc::hash<int_vector>()(int_vector{}) == ccc::hash<int_vector>()(int_vector{}));

    CHECK(ccc::hash<string_vector>()(string_vector{"a", "b"}) == ccc::hash<string_vector>()(string_vector{"a", "b"}));
    CHECK(ccc::hash<string_vector>()(string_vector{"a", "b"}) != ccc::hash<string_vector>()(string_vector{"a", "c"}));

    // Nested vectors
    using nested_vector = std::vector<std::vector<int>>;
    CHECK(ccc::hash<nested_vector>()(nested_vector{{1, 2}, {3}}) ==
          ccc::hash<nested_vector>()(nested_vector{{1, 2}, {3}}));
    CHECK(ccc::hash<nested_vector>()(nested_vector{{1, 2}, {3}}) !=
          ccc::hash<nested_vector>()(nested_vector{{1}, {2, 3}}));
}

TEST_CASE("Hash - Vector of bool")
{
    using bool_vector = std::vector<bool>;

    CHECK(ccc::hash<bool_vector>()(bool_vector{true, false, true}) ==
          ccc::hash<bool_vector>()(bool_vector{true, false, true}));
    CHECK(ccc::hash<bool_vector>()(bool_vector{true, false, true}) !=
          ccc::hash<bool_vector>()(bool_vector{true, false}));
    CHECK(ccc::hash<bool_vector>()(bool_vector{true, false, true}) !=
          ccc::hash<bool_vector>()(bool_vector{false, true, true}));
    CHECK(ccc::hash<bool_vector>()(bool_vector{}) == ccc::hash<bool_vector>()(bool_vector{}));

    // Length is mixed into the hash: {1,0} vs {1,0,0} differ
    CHECK(ccc::hash<bool_vector>()(bool_vector{true, false}) !=
          ccc::hash<bool_vector>()(bool_vector{true, false, false}));
}
