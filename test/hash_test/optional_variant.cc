/**
 * @file optional_variant.cc
 * @brief Tests for ccc::hash specializations of std::optional, std::variant
 *        and std::monostate (requires C++17)
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

#if (__cplusplus >= 201703L)

#if (__cplusplus >= 202002L)
static_assert(ccc::hash<std::optional<int>>()(std::optional<int>(42)) ==
                  ccc::hash<std::optional<int>>()(std::optional<int>(42)),
              "ccc::hash for optional must be constexpr in C++20");

static_assert(ccc::hash<std::monostate>()(std::monostate{}) != 0U, "monostate must not hash to 0");
#endif

TEST_CASE("Hash - Optional")
{
    using optional_int = std::optional<int>;
    using optional_string = std::optional<std::string>;

    // Empty optionals are stable
    CHECK(ccc::hash<optional_int>()(optional_int{}) == ccc::hash<optional_int>()(std::nullopt));
    CHECK(ccc::hash<optional_string>()(std::nullopt) == ccc::hash<optional_string>()(optional_string{}));

    // Engaged optionals agree with their contained value's hash family
    CHECK(ccc::hash<optional_int>()(optional_int{42}) == ccc::hash<optional_int>()(optional_int{42}));
    CHECK(ccc::hash<optional_int>()(optional_int{42}) != ccc::hash<optional_int>()(optional_int{24}));
    CHECK(ccc::hash<optional_string>()(std::string("hello")) == ccc::hash<optional_string>()(std::string("hello")));

    // Empty and engaged states differ
    CHECK(ccc::hash<optional_int>()(std::nullopt) != ccc::hash<optional_int>()(optional_int{0}));
}

TEST_CASE("Hash - Variant")
{
    using variant_int_string = std::variant<int, std::string>;

    // Same alternative and value
    CHECK(ccc::hash<variant_int_string>()(variant_int_string{42}) ==
          ccc::hash<variant_int_string>()(variant_int_string{42}));
    CHECK(ccc::hash<variant_int_string>()(variant_int_string{std::string("hello")}) ==
          ccc::hash<variant_int_string>()(variant_int_string{std::string("hello")}));

    // Same value in different alternatives must differ
    CHECK(ccc::hash<variant_int_string>()(variant_int_string{42}) !=
          ccc::hash<variant_int_string>()(variant_int_string{std::string("42")}));

    // Different values in the same alternative
    CHECK(ccc::hash<variant_int_string>()(variant_int_string{42}) !=
          ccc::hash<variant_int_string>()(variant_int_string{24}));

    // Monostate is stable and distinct
    CHECK(ccc::hash<std::monostate>()(std::monostate{}) == ccc::hash<std::monostate>()(std::monostate{}));
    CHECK(ccc::hash<std::variant<std::monostate, int>>()(std::variant<std::monostate, int>{}) !=
          ccc::hash<std::variant<std::monostate, int>>()(std::variant<std::monostate, int>{42}));
}

#endif  // C++17
