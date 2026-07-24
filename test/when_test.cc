/**
 * @file when_test.cc
 * @brief Unit tests for ccc::when
 */

#ifndef TEST_USE_MODULE
#include "ccc/when.hh"
#endif

#include "test_pch.hh"

#include <string>

#ifdef TEST_USE_MODULE
import ccc.when;
#endif

namespace {

struct point {
    int x = 0;
    int y = 0;

    constexpr bool operator==(const point& other) const noexcept
    {
        return x == other.x && y == other.y;
    }
};

}  // namespace

TEST_CASE("When - MatchesFirstCase")
{
    const int value = 1;
    const int result = ccc::when(value, 1, [] { return 10; }, 2, [] { return 20; });
    CHECK(result == 10);
}

TEST_CASE("When - MatchesLaterCase")
{
    const int value = 3;
    const int result = ccc::when(value, 1, [] { return 10; }, 2, [] { return 20; }, 3, [] { return 30; });
    CHECK(result == 30);
}

TEST_CASE("When - FallsThroughToDefault")
{
    const int value = 42;
    const int result = ccc::when(value, 1, [] { return 10; }, ccc::default_tag, [] { return 99; });
    CHECK(result == 99);
}

TEST_CASE("When - ReturnsDefaultConstructedValueWhenNoMatch")
{
    const int value = 7;
    const int result = ccc::when(value, 1, [] { return 10; }, 2, [] { return 20; });
    CHECK(result == 0);
}

TEST_CASE("When - WorksWithCustomComparableType")
{
    const point value{2, 3};
    const int result = ccc::when(
        value,
        point{0, 0},
        [] { return 0; },
        point{2, 3},
        [] { return 1; },
        ccc::default_tag,
        [] { return -1; });
    CHECK(result == 1);
}

TEST_CASE("When - WorksWithString")
{
    const std::string value = "world";
    const std::string result = ccc::when(
        value,
        std::string{"hello"},
        [] { return std::string{"greeting"}; },
        std::string{"world"},
        [] { return std::string{"earth"}; },
        ccc::default_tag,
        [] { return std::string{"unknown"}; });
    CHECK(result == "earth");
}

TEST_CASE("When - WorksWithVoidReturn")
{
    int side_effect = 0;
    const int value = 2;
    ccc::when(value, 1, [&] { side_effect = 10; }, 2, [&] { side_effect = 20; });
    CHECK(side_effect == 20);
}

TEST_CASE("When - DefaultBranchWithVoidReturn")
{
    int side_effect = 0;
    const int value = 99;
    ccc::when(value, 1, [&] { side_effect = 10; }, ccc::default_tag, [&] { side_effect = 30; });
    CHECK(side_effect == 30);
}

TEST_CASE("When - IsConstexpr")
{
    constexpr int value = 2;
    constexpr int result =
        ccc::when(value, 1, [] { return 1; }, 2, [] { return 4; }, ccc::default_tag, [] { return 0; });
    static_assert(result == 4, "when should be usable in constant expressions");
    CHECK(result == 4);
}
