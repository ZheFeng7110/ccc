#ifndef TEST_USE_MODULE
#include "ccc/expected.hh"
#endif

#include "test_pch.hh"

#include <exception>
#include <initializer_list>
#include <type_traits>
#include <utility>

#ifdef TEST_USE_MODULE
import ccc.utility;
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

TEST_CASE("Expected - DefaultConstructsValue")
{
    ccc::expected<int, int> exp;

    CHECK(exp.has_value());
    CHECK(exp);
    CHECK(0 == *exp);
    CHECK_NOTHROW(exp.value());
    CHECK(0 == exp.value());
}

TEST_CASE("Expected - ConstructFromValue")
{
    ccc::expected<int, int> exp(42);

    CHECK(exp.has_value());
    CHECK(42 == *exp);
    CHECK(42 == exp.value());
}

TEST_CASE("Expected - ConstructValueInPlace")
{
    ccc::expected<Point, int> exp(ccc::in_place, 1, 2);

    REQUIRE(exp.has_value());
    CHECK((Point{1, 2}) == *exp);
    CHECK(3 == exp->sum());
}

TEST_CASE("Expected - ConstructValueInPlaceWithInitializerList")
{
    ccc::expected<ListValue, int> exp(ccc::in_place, {1, 2, 3}, 4);

    REQUIRE(exp.has_value());
    CHECK((ListValue{{1, 2, 3}, 4}) == *exp);
}

TEST_CASE("Expected - ConstructFromUnexpected")
{
    const ccc::unexpected<int> error(7);
    ccc::expected<int, int> from_lvalue(error);

    ccc::expected<int, int> from_rvalue(ccc::unexpected<int>(9));

    ccc::unexpected<int> xerror(114514);
    ccc::expected<int, int> from_xvalue(std::move(xerror));

    CHECK_FALSE(from_lvalue.has_value());
    CHECK(7 == from_lvalue.error());
    CHECK_FALSE(from_rvalue.has_value());
    CHECK(9 == from_rvalue.error());
    CHECK_FALSE(from_xvalue.has_value());
    CHECK(114514 == from_xvalue.error());
}

TEST_CASE("Expected - ConstructErrorInPlace")
{
    ccc::expected<int, int> exp(ccc::unexpect, 13);

    CHECK_FALSE(exp.has_value());
    CHECK_FALSE(static_cast<bool>(exp));
    CHECK(13 == exp.error());
}

TEST_CASE("Expected - ConstructErrorInPlaceWithInitializerList")
{
    ccc::expected<int, ListValue> exp(ccc::unexpect, {2, 4, 6}, 8);

    REQUIRE_FALSE(exp.has_value());
    CHECK((ListValue{{2, 4, 6}, 8}) == exp.error());
}

TEST_CASE("Expected - CopyAndMoveConstruct")
{
    ccc::expected<int, int> value(42);
    ccc::expected<int, int> copied_value(value);
    ccc::expected<int, int> moved_value(std::move(value));

    CHECK(copied_value.has_value());
    CHECK(42 == *copied_value);
    CHECK(moved_value.has_value());
    CHECK(42 == *moved_value);

    ccc::expected<int, int> error(ccc::unexpect, 7);
    ccc::expected<int, int> copied_error(error);
    ccc::expected<int, int> moved_error(std::move(error));

    CHECK_FALSE(copied_error.has_value());
    CHECK(7 == copied_error.error());
    CHECK_FALSE(moved_error.has_value());
    CHECK(7 == moved_error.error());
}

TEST_CASE("Expected - ObserversPreserveReferenceQualifiers")
{
    ccc::expected<int, int> exp(42);
    const ccc::expected<int, int> const_exp(43);
    ccc::expected<int, int> err(ccc::unexpect, 7);

    CHECK(42 == *exp);
    CHECK(43 == *const_exp);
    CHECK(42 == *std::move(exp));
    CHECK(7 == std::move(err).error());
}

TEST_CASE("Expected - ValueThrowsBadExpectedAccessWhenError")
{
    ccc::expected<int, int> exp(ccc::unexpect, 7);

    try {
        (void)exp.value();
        FAIL("value() should throw for an error state");
    }
    catch (const ccc::bad_expected_access<int>& ex) {
        CHECK(7 == ex.error());
        CHECK(std::string("bad access to ccc::expected without expected value(ErrorType = non-void)") ==
              std::string(ex.what()));
    }
}

TEST_CASE("Expected - ValueOr")
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    CHECK(42 == value.value_or(100));
    CHECK(100 == error.value_or(100));
    CHECK(42 == (ccc::expected<int, int>(42).value_or(100)));
    CHECK(100 == (ccc::expected<int, int>(ccc::unexpect, 7).value_or(100)));
}

TEST_CASE("Expected - ErrorOr")
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    CHECK(100 == value.error_or(100));
    CHECK(7 == error.error_or(100));
    CHECK(100 == (ccc::expected<int, int>(42).error_or(100)));
    CHECK(7 == (ccc::expected<int, int>(ccc::unexpect, 7).error_or(100)));
}

TEST_CASE("Expected - AssignExpected")
{
    ccc::expected<int, int> target(1);
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    target = value;
    REQUIRE(target.has_value());
    CHECK(42 == *target);

    target = error;
    REQUIRE_FALSE(target.has_value());
    CHECK(7 == target.error());

    target.assign(value);
    REQUIRE(target.has_value());
    CHECK(42 == *target);
}

TEST_CASE("Expected - MoveAssignExpected")
{
    ccc::expected<int, int> target(1);
    ccc::expected<int, int> value(42);
    ccc::expected<int, int> error(ccc::unexpect, 7);

    target = std::move(value);
    REQUIRE(target.has_value());
    CHECK(42 == *target);

    target = std::move(error);
    REQUIRE_FALSE(target.has_value());
    CHECK(7 == target.error());
}

TEST_CASE("Expected - AssignValue")
{
    // ReSharper disable once CppDFAUnusedValue
    ccc::expected<int, int> exp(ccc::unexpect, 7);

    exp = 42;
    REQUIRE(exp.has_value());
    CHECK(42 == *exp);

    exp.assign(100);
    REQUIRE(exp.has_value());
    CHECK(100 == *exp);
}

TEST_CASE("Expected - AssignUnexpected")
{
    ccc::expected<int, int> exp(42);

    exp.assign(ccc::unexpected<int>(7));
    REQUIRE_FALSE(exp.has_value());
    CHECK(7 == exp.error());

    exp = ccc::unexpected<int>(9);
    REQUIRE_FALSE(exp.has_value());
    CHECK(9 == exp.error());
}

TEST_CASE("Expected - EmplaceValue")
{
    ccc::expected<Point, int> exp(ccc::unexpect, 7);

    Point& value = exp.emplace(3, 4);
    REQUIRE(exp.has_value());
    CHECK((Point{3, 4}) == value);
    CHECK((Point{3, 4}) == *exp);
}

TEST_CASE("Expected - EmplaceValueWithInitializerList")
{
    ccc::expected<ListValue, int> exp(ccc::unexpect, 7);

    ListValue& value = exp.emplace({1, 2, 3}, 4);
    REQUIRE(exp.has_value());
    CHECK((ListValue{{1, 2, 3}, 4}) == value);
    CHECK((ListValue{{1, 2, 3}, 4}) == *exp);
}

TEST_CASE("Expected - SwapValueWithValue")
{
    ccc::expected<int, int> lhs(1);
    ccc::expected<int, int> rhs(2);

    lhs.swap(rhs);
    REQUIRE(lhs.has_value());
    REQUIRE(rhs.has_value());
    CHECK(2 == *lhs);
    CHECK(1 == *rhs);
}

TEST_CASE("Expected - SwapErrorWithError")
{
    ccc::expected<int, int> lhs(ccc::unexpect, 1);
    ccc::expected<int, int> rhs(ccc::unexpect, 2);

    swap(lhs, rhs);
    REQUIRE_FALSE(lhs.has_value());
    REQUIRE_FALSE(rhs.has_value());
    CHECK(2 == lhs.error());
    CHECK(1 == rhs.error());
}

TEST_CASE("Expected - SwapValueWithError")
{
    ccc::expected<int, int> value(42);
    ccc::expected<int, int> error(ccc::unexpect, 7);

    value.swap(error);
    REQUIRE_FALSE(value.has_value());
    REQUIRE(error.has_value());
    CHECK(7 == value.error());
    CHECK(42 == *error);
}

TEST_CASE("Expected - CompareWithExpected")
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> same_value(42);
    const ccc::expected<int, int> other_value(100);
    const ccc::expected<int, int> error(ccc::unexpect, 7);
    const ccc::expected<int, int> same_error(ccc::unexpect, 7);
    const ccc::expected<int, int> other_error(ccc::unexpect, 9);

    CHECK(value == same_value);
    CHECK_FALSE(value == other_value);
    CHECK_FALSE(value == error);
    CHECK(error == same_error);
    CHECK_FALSE(error == other_error);
}

TEST_CASE("Expected - CompareWithUnexpected")
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    CHECK_FALSE(value == ccc::unexpected<int>(7));
    CHECK(error == ccc::unexpected<int>(7));
    CHECK_FALSE(error == ccc::unexpected<int>(9));
}

TEST_CASE("Expected - CompareWithValue")
{
    const ccc::expected<int, int> value(42);
    const ccc::expected<int, int> error(ccc::unexpect, 7);

    CHECK(value == 42);
    CHECK_FALSE(value == 100);
    CHECK_FALSE(error == 42);
    CHECK(error == ccc::unexpected<int>(7));
}

TEST_CASE("Expected - BadExpectedAccessStoresError")
{
    ccc::bad_expected_access<int> access(7);
    const std::exception& base = access;

    CHECK(7 == access.error());
    access.error() = 9;
    CHECK(9 == access.error());
    CHECK(std::string("bad access to ccc::expected without expected value(ErrorType = non-void)") ==
          std::string(base.what()));
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
