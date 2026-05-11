#ifndef TEST_USE_MODULE
#include "ccc/expected.hh"
#endif

#include "test_pch.hh"

#include <exception>
#include <initializer_list>
#include <type_traits>
#include <utility>

#ifdef TEST_USE_MODULE
import ccc.expected;
#endif

namespace {

struct Point {
    int x;
    int y;

    constexpr Point(const int x_, const int y_) noexcept : x(x_), y(y_) {}

    [[nodiscard]] constexpr int sum() const noexcept
    {
        return x + y;
    }

    constexpr bool operator==(const Point& other) const noexcept
    {
        return x == other.x && y == other.y;
    }
};

struct ListValue {
    int sum;
    int extra;

    constexpr ListValue(std::initializer_list<int> values, const int extra_) noexcept : sum(0), extra(extra_)
    {
        for (const int value : values) {
            sum += value;
        }
    }

    constexpr bool operator==(const ListValue& other) const noexcept
    {
        return sum == other.sum && extra == other.extra;
    }
};

}  // namespace

TEST(Expected, DefaultConstructsValue)
{
    ccc::expected<int, int> exp;

    EXPECT_TRUE(exp.has_value());
    EXPECT_TRUE(exp);
    EXPECT_EQ(0, *exp);
    EXPECT_NO_THROW(exp.value());
    EXPECT_EQ(0, exp.value());
}

TEST(Expected, ConstructFromValue)
{
    ccc::expected<int, int> exp(42);

    EXPECT_TRUE(exp.has_value());
    EXPECT_EQ(42, *exp);
    EXPECT_EQ(42, exp.value());
}

TEST(Expected, ConstructValueInPlace)
{
    ccc::expected<Point, int> exp(ccc::in_place, 1, 2);

    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ((Point{1, 2}), *exp);
    EXPECT_EQ(3, exp->sum());
}

TEST(Expected, ConstructValueInPlaceWithInitializerList)
{
    ccc::expected<ListValue, int> exp(ccc::in_place, {1, 2, 3}, 4);

    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ((ListValue{{1, 2, 3}, 4}), *exp);
}

TEST(Expected, ConstructFromUnexpected)
{
    const ccc::unexpected<int> error(7);
    ccc::expected<int, int> from_lvalue(error);

    ccc::expected<int, int> from_rvalue(ccc::unexpected<int>(9));

    ccc::unexpected<int> xerror(114514);
    ccc::expected<int, int> from_xvalue(std::move(xerror));

    EXPECT_FALSE(from_lvalue.has_value());
    EXPECT_EQ(7, from_lvalue.error());
    EXPECT_FALSE(from_rvalue.has_value());
    EXPECT_EQ(9, from_rvalue.error());
    EXPECT_FALSE(from_xvalue.has_value());
    EXPECT_EQ(114514, from_xvalue.error());
}

TEST(Expected, ConstructErrorInPlace)
{
    ccc::expected<int, int> exp(ccc::unexpect, 13);

    EXPECT_FALSE(exp.has_value());
    EXPECT_FALSE(static_cast<bool>(exp));
    EXPECT_EQ(13, exp.error());
}

TEST(Expected, ConstructErrorInPlaceWithInitializerList)
{
    ccc::expected<int, ListValue> exp(ccc::unexpect, {2, 4, 6}, 8);

    ASSERT_FALSE(exp.has_value());
    EXPECT_EQ((ListValue{{2, 4, 6}, 8}), exp.error());
}

TEST(Expected, CopyAndMoveConstruct)
{
    ccc::expected<int, int> value(42);
    ccc::expected<int, int> copied_value(value);
    ccc::expected<int, int> moved_value(std::move(value));

    EXPECT_TRUE(copied_value.has_value());
    EXPECT_EQ(42, *copied_value);
    EXPECT_TRUE(moved_value.has_value());
    EXPECT_EQ(42, *moved_value);

    ccc::expected<int, int> error(ccc::unexpect, 7);
    ccc::expected<int, int> copied_error(error);
    ccc::expected<int, int> moved_error(std::move(error));

    EXPECT_FALSE(copied_error.has_value());
    EXPECT_EQ(7, copied_error.error());
    EXPECT_FALSE(moved_error.has_value());
    EXPECT_EQ(7, moved_error.error());
}

TEST(Expected, ObserversPreserveReferenceQualifiers)
{
    ccc::expected<int, int> exp(42);
    const ccc::expected<int, int> const_exp(43);
    ccc::expected<int, int> err(ccc::unexpect, 7);

    EXPECT_EQ(42, *exp);
    EXPECT_EQ(43, *const_exp);
    EXPECT_EQ(42, *std::move(exp));
    EXPECT_EQ(7, std::move(err).error());
}

TEST(Expected, ValueThrowsBadExpectedAccessWhenError)
{
    ccc::expected<int, int> exp(ccc::unexpect, 7);

    try {
        (void)exp.value();
        FAIL() << "value() should throw for an error state";
    }
    catch (const ccc::bad_expected_access<int>& ex) {
        EXPECT_EQ(7, ex.error());
        EXPECT_STREQ("bad access to ccc::expected without expected value(ErrorType = non-void)", ex.what());
    }
}

TEST(Expected, ValueOr)
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    EXPECT_EQ(42, value.value_or(100));
    EXPECT_EQ(100, error.value_or(100));
    EXPECT_EQ(42, (ccc::expected<int, int>(42).value_or(100)));
    EXPECT_EQ(100, (ccc::expected<int, int>(ccc::unexpect, 7).value_or(100)));
}

TEST(Expected, ErrorOr)
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    EXPECT_EQ(100, value.error_or(100));
    EXPECT_EQ(7, error.error_or(100));
    EXPECT_EQ(100, (ccc::expected<int, int>(42).error_or(100)));
    EXPECT_EQ(7, (ccc::expected<int, int>(ccc::unexpect, 7).error_or(100)));
}

TEST(Expected, AssignExpected)
{
    ccc::expected<int, int> target(1);
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    target = value;
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(42, *target);

    target = error;
    ASSERT_FALSE(target.has_value());
    EXPECT_EQ(7, target.error());

    target.assign(value);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(42, *target);
}

TEST(Expected, MoveAssignExpected)
{
    ccc::expected<int, int> target(1);
    ccc::expected<int, int> value(42);
    ccc::expected<int, int> error(ccc::unexpect, 7);

    target = std::move(value);
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(42, *target);

    target = std::move(error);
    ASSERT_FALSE(target.has_value());
    EXPECT_EQ(7, target.error());
}

TEST(Expected, AssignValue)
{
    // ReSharper disable once CppDFAUnusedValue
    ccc::expected<int, int> exp(ccc::unexpect, 7);

    exp = 42;
    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ(42, *exp);

    exp.assign(100);
    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ(100, *exp);
}

TEST(Expected, AssignUnexpected)
{
    ccc::expected<int, int> exp(42);

    exp.assign(ccc::unexpected<int>(7));
    ASSERT_FALSE(exp.has_value());
    EXPECT_EQ(7, exp.error());

    exp = ccc::unexpected<int>(9);
    ASSERT_FALSE(exp.has_value());
    EXPECT_EQ(9, exp.error());
}

TEST(Expected, EmplaceValue)
{
    ccc::expected<Point, int> exp(ccc::unexpect, 7);

    Point& value = exp.emplace(3, 4);
    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ((Point{3, 4}), value);
    EXPECT_EQ((Point{3, 4}), *exp);
}

TEST(Expected, EmplaceValueWithInitializerList)
{
    ccc::expected<ListValue, int> exp(ccc::unexpect, 7);

    ListValue& value = exp.emplace({1, 2, 3}, 4);
    ASSERT_TRUE(exp.has_value());
    EXPECT_EQ((ListValue{{1, 2, 3}, 4}), value);
    EXPECT_EQ((ListValue{{1, 2, 3}, 4}), *exp);
}

TEST(Expected, SwapValueWithValue)
{
    ccc::expected<int, int> lhs(1);
    ccc::expected<int, int> rhs(2);

    lhs.swap(rhs);
    ASSERT_TRUE(lhs.has_value());
    ASSERT_TRUE(rhs.has_value());
    EXPECT_EQ(2, *lhs);
    EXPECT_EQ(1, *rhs);
}

TEST(Expected, SwapErrorWithError)
{
    ccc::expected<int, int> lhs(ccc::unexpect, 1);
    ccc::expected<int, int> rhs(ccc::unexpect, 2);

    swap(lhs, rhs);
    ASSERT_FALSE(lhs.has_value());
    ASSERT_FALSE(rhs.has_value());
    EXPECT_EQ(2, lhs.error());
    EXPECT_EQ(1, rhs.error());
}

TEST(Expected, SwapValueWithError)
{
    ccc::expected<int, int> value(42);
    ccc::expected<int, int> error(ccc::unexpect, 7);

    value.swap(error);
    ASSERT_FALSE(value.has_value());
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(7, value.error());
    EXPECT_EQ(42, *error);
}

TEST(Expected, CompareWithExpected)
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> same_value(42);
    const ccc::expected<int, int> other_value(100);
    const ccc::expected<int, int> error(ccc::unexpect, 7);
    const ccc::expected<int, int> same_error(ccc::unexpect, 7);
    const ccc::expected<int, int> other_error(ccc::unexpect, 9);

    EXPECT_TRUE(value == same_value);
    EXPECT_FALSE(value == other_value);
    EXPECT_FALSE(value == error);
    EXPECT_TRUE(error == same_error);
    EXPECT_FALSE(error == other_error);
}

TEST(Expected, CompareWithUnexpected)
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    EXPECT_FALSE(value == ccc::unexpected<int>(7));
    EXPECT_TRUE(error == ccc::unexpected<int>(7));
    EXPECT_FALSE(error == ccc::unexpected<int>(9));
}

TEST(Expected, CompareWithValue)
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    EXPECT_TRUE(value == 42);
    EXPECT_FALSE(value == 100);
    EXPECT_FALSE(error == 42);
    EXPECT_TRUE(error == ccc::unexpected<int>(7));
}

TEST(Expected, BadExpectedAccessStoresError)
{
    ccc::bad_expected_access<int> access(7);
    const std::exception& base = access;

    EXPECT_EQ(7, access.error());
    access.error() = 9;
    EXPECT_EQ(9, access.error());
    EXPECT_STREQ("bad access to ccc::expected without expected value(ErrorType = non-void)", base.what());
}

#if (__cplusplus >= 202002L)
namespace {

constexpr bool constexpr_expected_value_works()
{
    const ccc::expected<int, int> exp(42);
    return exp.has_value() && static_cast<bool>(exp) && *exp == 42 && exp.value() == 42 && exp.value_or(7) == 42;
}

constexpr bool constexpr_expected_error_works()
{
    const ccc::expected<int, int> exp(ccc::unexpect, 7);
    return !exp.has_value() && !static_cast<bool>(exp) && exp.error() == 7 && exp.value_or(42) == 42 &&
           exp.error_or(9) == 7;
}

static_assert(constexpr_expected_value_works(), "expected value state should be usable in constexpr contexts");
static_assert(constexpr_expected_error_works(), "expected error state should be usable in constexpr contexts");

}  // namespace
#endif
