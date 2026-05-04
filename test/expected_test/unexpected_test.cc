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

TEST(Unexpected, ConstructFromErrorValue)
{
    ccc::unexpected<int> unexp(42);

    EXPECT_EQ(42, unexp.error());

    unexp.error() = 7;
    EXPECT_EQ(7, unexp.error());

    const ccc::unexpected<int> const_unexp(11);
    EXPECT_EQ(11, const_unexp.error());
}

TEST(Unexpected, ConstructInPlace)
{
    ccc::unexpected<PairValue> unexp(ccc::in_place, 1, 2);

    EXPECT_EQ((PairValue{1, 2}), unexp.error());
}

TEST(Unexpected, ConstructInPlaceWithInitializerList)
{
    ccc::unexpected<std::vector<int>> unexp(ccc::in_place, {1, 2, 3});

    ASSERT_EQ(3u, unexp.error().size());
    EXPECT_EQ(1, unexp.error()[0]);
    EXPECT_EQ(2, unexp.error()[1]);
    EXPECT_EQ(3, unexp.error()[2]);
}

TEST(Unexpected, ConstructWithInitializerList)
{
    ccc::unexpected<std::vector<int>> unexp({1, 2, 3});

    ASSERT_EQ(3u, unexp.error().size());
    EXPECT_EQ(1, unexp.error()[0]);
    EXPECT_EQ(2, unexp.error()[1]);
    EXPECT_EQ(3, unexp.error()[2]);
}

TEST(Unexpected, CopyAndMoveConstruct)
{
    ccc::unexpected<std::string> original("error");

    ccc::unexpected<std::string> copied(original);
    EXPECT_EQ("error", copied.error());

    ccc::unexpected<std::string> moved(std::move(original));
    EXPECT_EQ("error", moved.error());
}

TEST(Unexpected, ErrorRefQualifiers)
{
    ccc::unexpected<int> unexp(42);
    EXPECT_EQ(42, unexp.error());
    EXPECT_EQ(42, std::move(unexp).error());
}

TEST(Unexpected, SwapMemberAndAdl)
{
    ccc::unexpected<int> lhs(1);
    ccc::unexpected<int> rhs(2);

    lhs.swap(rhs);
    EXPECT_EQ(2, lhs.error());
    EXPECT_EQ(1, rhs.error());

    using std::swap;
    swap(lhs, rhs);
    EXPECT_EQ(1, lhs.error());
    EXPECT_EQ(2, rhs.error());
}

TEST(Unexpected, EqualityAndOrdering)
{
    const ccc::unexpected<int> one(1);
    const ccc::unexpected<int> another_one(1);
    const ccc::unexpected<int> two(2);

    EXPECT_TRUE(one == another_one);
    EXPECT_FALSE(one == two);
    EXPECT_TRUE(one != two);
    EXPECT_TRUE(one < two);
    EXPECT_TRUE(one <= another_one);
    EXPECT_TRUE(two > one);
    EXPECT_TRUE(two >= another_one);
}

#ifdef __cpp_deduction_guides
TEST(Unexpected, ClassTemplateArgumentDeduction)
{
    ccc::unexpected unexp(42);

    static_assert(std::is_same_v<decltype(unexp), ccc::unexpected<int>>);
    EXPECT_EQ(42, unexp.error());
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
