/**
 * @file string.cc
 * @brief Tests for ccc::hash string specializations
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <string>
#include <type_traits>

#if (__cplusplus >= 201703L)
#include <string_view>
#endif

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

namespace {

template<typename CharT>
std::basic_string<CharT> widen(const char* text)
{
    std::basic_string<CharT> result;
    while (*text != '\0') {
        result.push_back(static_cast<CharT>(*text));
        ++text;
    }
    return result;
}

#if (__cplusplus >= 202002L)
static_assert(ccc::hash<std::string>()("hello") == 1568626408U, "ccc::hash must use MurmurHash3 for strings");
static_assert(ccc::hash<std::string_view>()("hello") == 1568626408U, "ccc::hash must use MurmurHash3 for strings");
#endif

}  // namespace

TEST_CASE("Hash - Strings")
{
    const std::string str{"hello"};
    CHECK(ccc::hash<std::string>()(str) == 1568626408U);
    CHECK(ccc::hash<std::string>()("hello") == 1568626408U);
    CHECK(ccc::hash<std::string>()("") == ccc::hash<std::string>()(""));
    CHECK(ccc::hash<std::string>()("hello") != ccc::hash<std::string>()("hellp"));

#if (__cplusplus >= 201703L)
    const std::string_view string_view{str};
    CHECK(ccc::hash<std::string_view>()(string_view) == 1568626408U);

    const std::wstring wstr = widen<wchar_t>("hello");
    CHECK(ccc::hash<std::wstring>()(wstr) == ccc::hash<std::wstring>()(wstr));
    CHECK(ccc::hash<std::wstring>()(wstr) != ccc::hash<std::wstring>()(widen<wchar_t>("hellp")));
#endif
}
