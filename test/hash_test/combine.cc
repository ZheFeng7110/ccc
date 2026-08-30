/**
 * @file combine.cc
 * @brief Tests for ccc::hash_combine
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <string>
#include <type_traits>

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

#if (__cplusplus >= 202002L)
static_assert(ccc::hash_combine(1U, 42) == ccc::hash_combine(1U, 42), "hash_combine must be constexpr in C++20");
static_assert(ccc::hash_combine(1U, 42, 24) == ccc::hash_combine(ccc::hash_combine(1U, 42), 24),
              "variadic hash_combine must fold left");
#endif

namespace {

constexpr std::size_t kBaseSeed = 0x12345678U;

}  // namespace

TEST_CASE("Hash - Combine single value")
{
    // Deterministic
    CHECK(ccc::hash_combine(kBaseSeed, 42) == ccc::hash_combine(kBaseSeed, 42));
    CHECK(ccc::hash_combine(kBaseSeed, std::string("hello")) == ccc::hash_combine(kBaseSeed, std::string("hello")));

    // Depends on the seed
    CHECK(ccc::hash_combine(kBaseSeed, 42) != ccc::hash_combine(kBaseSeed + 1U, 42));

    // Depends on the value
    CHECK(ccc::hash_combine(kBaseSeed, 42) != ccc::hash_combine(kBaseSeed, 24));

    // Works with any hashable type
    CHECK(ccc::hash_combine(kBaseSeed, 3.5) == ccc::hash_combine(kBaseSeed, 3.5));
}

TEST_CASE("Hash - Combine multiple values")
{
    // Variadic form must fold left
    CHECK(ccc::hash_combine(kBaseSeed, 42, 24) == ccc::hash_combine(ccc::hash_combine(kBaseSeed, 42), 24));

    // Order-sensitive for ints (element order matters)
    CHECK(ccc::hash_combine(kBaseSeed, 42, 24) != ccc::hash_combine(kBaseSeed, 24, 42));

    // Same content through different types must produce the same hash
    const std::string text{"combine"};
    CHECK(ccc::hash_combine(kBaseSeed, text, 42) == ccc::hash_combine(kBaseSeed, std::string("combine"), 42));
}
