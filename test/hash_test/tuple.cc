/**
 * @file tuple.cc
 * @brief Tests for ccc::hash specializations of std::pair and std::tuple
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

#if (__cplusplus >= 202002L)
static_assert(ccc::hash<std::pair<int, int>>()(std::make_pair(1, 2)) ==
                  ccc::hash<std::pair<int, int>>()(std::make_pair(1, 2)),
              "ccc::hash for pair must be constexpr in C++20");

static_assert(ccc::hash<std::tuple<>>()(std::tuple<>{}) == 0U, "empty tuple must hash to 0");
#endif

TEST_CASE("Hash - Pair")
{
    using int_pair = std::pair<int, int>;
    using string_pair = std::pair<int, std::string>;

    CHECK(ccc::hash<int_pair>()(int_pair{1, 2}) == ccc::hash<int_pair>()(int_pair{1, 2}));
    CHECK(ccc::hash<int_pair>()(int_pair{1, 2}) != ccc::hash<int_pair>()(int_pair{2, 1}));
    CHECK(ccc::hash<int_pair>()(int_pair{1, 2}) != ccc::hash<int_pair>()(int_pair{1, 3}));

    CHECK(ccc::hash<string_pair>()(string_pair{1, "hello"}) == ccc::hash<string_pair>()(string_pair{1, "hello"}));
    CHECK(ccc::hash<string_pair>()(string_pair{1, "hello"}) != ccc::hash<string_pair>()(string_pair{1, "world"}));
}

TEST_CASE("Hash - Tuple")
{
    using int_tuple = std::tuple<int, int, int>;
    using mixed_tuple = std::tuple<int, std::string, double>;

    CHECK(ccc::hash<int_tuple>()(int_tuple{1, 2, 3}) == ccc::hash<int_tuple>()(int_tuple{1, 2, 3}));
    CHECK(ccc::hash<int_tuple>()(int_tuple{1, 2, 3}) != ccc::hash<int_tuple>()(int_tuple{3, 2, 1}));
    CHECK(ccc::hash<int_tuple>()(int_tuple{1, 2, 3}) != ccc::hash<std::pair<int, int>>()(std::make_pair(1, 2)));

    CHECK(ccc::hash<mixed_tuple>()(mixed_tuple{1, "hello", 3.5}) ==
          ccc::hash<mixed_tuple>()(mixed_tuple{1, "hello", 3.5}));
    CHECK(ccc::hash<mixed_tuple>()(mixed_tuple{1, "hello", 3.5}) !=
          ccc::hash<mixed_tuple>()(mixed_tuple{1, "world", 3.5}));

    // Empty tuple is stable
    CHECK(ccc::hash<std::tuple<>>()(std::tuple<>{}) == ccc::hash<std::tuple<>>()(std::tuple<>{}));

    // Nested composites
    using nested_tuple = std::tuple<std::pair<int, int>, std::string>;
    CHECK(ccc::hash<nested_tuple>()(nested_tuple{std::make_pair(1, 2), "x"}) ==
          ccc::hash<nested_tuple>()(nested_tuple{std::make_pair(1, 2), "x"}));
}
