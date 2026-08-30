/**
 * @file system_error.cc
 * @brief Tests for ccc::hash specializations of std::error_code and std::error_condition
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <system_error>
#include <type_traits>

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

TEST_CASE("Hash - Error code")
{
    using error_code = std::error_code;

    CHECK(ccc::hash<error_code>()(error_code{1, std::generic_category()}) ==
          ccc::hash<error_code>()(error_code{1, std::generic_category()}));
    CHECK(ccc::hash<error_code>()(error_code{1, std::generic_category()}) !=
          ccc::hash<error_code>()(error_code{2, std::generic_category()}));
    CHECK(ccc::hash<error_code>()(error_code{1, std::generic_category()}) !=
          ccc::hash<error_code>()(error_code{1, std::system_category()}));
    CHECK(ccc::hash<error_code>()(error_code{}) == ccc::hash<error_code>()(error_code{}));
}

TEST_CASE("Hash - Error condition")
{
    using error_condition = std::error_condition;

    CHECK(ccc::hash<error_condition>()(error_condition{1, std::generic_category()}) ==
          ccc::hash<error_condition>()(error_condition{1, std::generic_category()}));
    CHECK(ccc::hash<error_condition>()(error_condition{1, std::generic_category()}) !=
          ccc::hash<error_condition>()(error_condition{2, std::generic_category()}));
    CHECK(ccc::hash<error_condition>()(error_condition{}) == ccc::hash<error_condition>()(error_condition{}));
}
