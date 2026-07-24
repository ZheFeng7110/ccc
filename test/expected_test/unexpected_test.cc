#ifndef TEST_USE_MODULE
#include "ccc/expected.hh"
#endif

#include "test_pch.hh"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

#ifdef TEST_USE_MODULE
import ccc.utility;
import ccc.expected;
#endif

namespace {

struct PairValue {
    int first;
    int second;

    constexpr PairValue(const int first_, const int second_) noexcept : first(first_), second(second_) {}

    constexpr bool operator==(const PairValue& other) const noexcept
    {
        return first == other.first && second == other.second;
    }
};

}  // namespace

TEST_CASE("Unexpected - ConstructFromErrorValue")
{
    ccc::unexpected<int> unexp(42);

    CHECK(42 == unexp.error());

    unexp.error() = 7;
    CHECK(7 == unexp.error());

    const ccc::unexpected<int> const_unexp(11);
    CHECK(11 == const_unexp.error());
}

TEST_CASE("Unexpected - ConstructInPlace")
{
    ccc::unexpected<PairValue> unexp(ccc::in_place, 1, 2);

    CHECK((PairValue{1, 2}) == unexp.error());
}

TEST_CASE("Unexpected - ConstructInPlaceWithInitializerList")
{
    ccc::unexpected<std::vector<int>> unexp(ccc::in_place, {1, 2, 3});

    REQUIRE(unexp.error().size() == 3u);
    CHECK(1 == unexp.error()[0]);
    CHECK(2 == unexp.error()[1]);
    CHECK(3 == unexp.error()[2]);
}

TEST_CASE("Unexpected - ConstructWithInitializerList")
{
    ccc::unexpected<std::vector<int>> unexp({1, 2, 3});

    REQUIRE(unexp.error().size() == 3u);
    CHECK(1 == unexp.error()[0]);
    CHECK(2 == unexp.error()[1]);
    CHECK(3 == unexp.error()[2]);
}

TEST_CASE("Unexpected - CopyAndMoveConstruct")
{
    ccc::unexpected<std::string> original("error");

    ccc::unexpected<std::string> copied(original);
    CHECK("error" == copied.error());

    ccc::unexpected<std::string> moved(std::move(original));
    CHECK("error" == moved.error());
}

TEST_CASE("Unexpected - ErrorRefQualifiers")
{
    ccc::unexpected<int> unexp(42);
    CHECK(42 == unexp.error());
    CHECK(42 == std::move(unexp).error());
}

TEST_CASE("Unexpected - SwapMemberAndAdl")
{
    ccc::unexpected<int> lhs(1);
    ccc::unexpected<int> rhs(2);

    lhs.swap(rhs);
    CHECK(2 == lhs.error());
    CHECK(1 == rhs.error());

    using std::swap;
    swap(lhs, rhs);
    CHECK(1 == lhs.error());
    CHECK(2 == rhs.error());
}

TEST_CASE("Unexpected - EqualityAndOrdering")
{
    const ccc::unexpected<int> one(1);
    const ccc::unexpected<int> another_one(1);
    const ccc::unexpected<int> two(2);

    CHECK(one == another_one);
    CHECK_FALSE(one == two);
    CHECK(one != two);
    CHECK(one < two);
    CHECK(one <= another_one);
    CHECK(two > one);
    CHECK(two >= another_one);
}

#ifdef __cpp_deduction_guides
TEST_CASE("Unexpected - ClassTemplateArgumentDeduction")
{
    ccc::unexpected unexp(42);

    static_assert(std::is_same_v<decltype(unexp), ccc::unexpected<int>>);
    CHECK(42 == unexp.error());
}
#endif

namespace {

constexpr bool constexpr_unexpected_works()
{
    const ccc::unexpected<int> unexp(42);
    const ccc::unexpected<int> same(42);
    const ccc::unexpected<int> other(7);

    // ReSharper disable twice CppRedundantBooleanExpressionArgument
    return unexp.error() == 42 && unexp == same && unexp != other;
}

static_assert(constexpr_unexpected_works(), "unexpected should be usable in constexpr contexts");

}  // namespace
