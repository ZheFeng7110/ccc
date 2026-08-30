/**
 * @file basic_types.cc
 * @brief Tests for ccc::hash basic types
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>

#if (__cplusplus >= 201703L)
#include <string_view>
#endif

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

namespace {

enum class TestEnum : unsigned short {
    value = 0x1234U,
};

template<std::size_t Size>
struct unsigned_integer_of_size {
    using type = typename std::conditional<
        Size == sizeof(unsigned char),
        unsigned char,
        typename std::conditional<
            Size == sizeof(unsigned short),
            unsigned short,
            typename std::conditional<
                Size == sizeof(unsigned int),
                unsigned int,
                typename std::conditional<Size == sizeof(unsigned long), unsigned long, unsigned long long>::type>::
                type>::type>::type;
};

template<typename To, typename From>
To bit_copy(const From& from)
{
    static_assert(sizeof(To) == sizeof(From), "Types must have the same size");

    To to;
    std::memcpy(&to, &from, sizeof(to));
    return to;
}

template<typename T>
std::size_t expected_scalar_hash(const T& value, std::true_type)
{
    return bit_copy<std::size_t>(value);
}

template<typename T>
std::size_t expected_scalar_hash(const T& value, std::false_type)
{
    using unsigned_type = typename unsigned_integer_of_size<sizeof(T)>::type;
    return bit_copy<unsigned_type>(value);
}

template<typename T>
std::size_t expected_scalar_hash(const T& value)
{
    return expected_scalar_hash(value, std::integral_constant<bool, sizeof(T) == sizeof(std::size_t)>());
}

#if (__cplusplus >= 202002L)
static_assert(ccc::hash<unsigned int>()(114514U) == ccc::hash<unsigned int>()(114514U),
              "ccc::hash must be constexpr in C++20");

static_assert(ccc::hash<std::string_view>()(std::string_view("hello")) == 1568626408U,
              "ccc::hash must use MurmurHash3 for strings");
#endif

}  // namespace

TEST_CASE("Hash - Scalar types")
{
    CHECK(ccc::hash<bool>()(false) == expected_scalar_hash(false));
    CHECK(ccc::hash<bool>()(true) == expected_scalar_hash(true));
    CHECK(ccc::hash<int>()(-114514) == expected_scalar_hash(-114514));
    CHECK(ccc::hash<unsigned int>()(114514U) == expected_scalar_hash(114514U));
    CHECK(ccc::hash<float>()(114.514F) == expected_scalar_hash(114.514F));
    CHECK(ccc::hash<double>()(114514.1919) == expected_scalar_hash(114514.1919));
}

TEST_CASE("Hash - Pointer")
{
    int value = 114514;
    int* pointer = &value;
    int* null_pointer = nullptr;

    CHECK(ccc::hash<int*>()(pointer) == expected_scalar_hash(pointer));
    CHECK(ccc::hash<int*>()(null_pointer) == expected_scalar_hash(null_pointer));
}

TEST_CASE("Hash - Enumeration")
{
    CHECK(ccc::hash<TestEnum>()(TestEnum::value) == expected_scalar_hash(static_cast<unsigned short>(TestEnum::value)));
}

TEST_CASE("Hash - Strings")
{
    const char* text = "hello";
    const wchar_t* wide_text = L"hello";
    const char16_t* utf16_text = u"hello";
    const char32_t* utf32_text = U"hello";

    CHECK(ccc::hash<const char*>()(text) == 1568626408U);
    CHECK(ccc::hash<const char*>()(text) == ccc::hash<std::string>()(text));
    CHECK(0U == ccc::hash<const char*>()(nullptr));
    CHECK(ccc::hash<const wchar_t*>()(wide_text) == ccc::hash<std::wstring>()(wide_text));
    CHECK(ccc::hash<const char16_t*>()(utf16_text) == ccc::hash<std::u16string>()(utf16_text));
    CHECK(ccc::hash<const char32_t*>()(utf32_text) == ccc::hash<std::u32string>()(utf32_text));

#if (__cplusplus >= 201703L)
    const std::string_view string_view{text};
    CHECK(ccc::hash<std::string_view>()(string_view) == ccc::hash<std::string>()(text));
#endif
}
