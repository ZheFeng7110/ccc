/**
 * @file 1arg.cc
 * @brief Tests for functions with 1 argument
 */

// NOLINTBEGIN(*-use-transparent-functors)

#ifndef TEST_USE_MODULE
#include "ccc/pipe_operator_helper.hh"
#endif

#include "test_pch.hh"

#include <stdexcept>
#include <functional>

#if (__cplusplus >= 202002L)
#define TEST_IS_CPP20_OR_HIGHER
#endif

#ifdef TEST_USE_MODULE
import ccc.pipe_operator_helper;
#endif

#ifndef CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR
#if (__cplusplus >= 202002L)
#define CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR constexpr
#else
#define CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR
#endif
#endif  // !CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR

namespace {

namespace pipe_ = ccc::pipe_operator_helper;

// Example function with 1 argument
struct logic_not_t : pipe_::pipe_operator<logic_not_t, 1> {
    using pipe_::pipe_operator<logic_not_t, 1>::operator();

    CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR bool operator()(const bool b) const noexcept
    {
        return !b;
    }
};
inline constexpr logic_not_t logic_not{};

}  // namespace

TEST_CASE("PipeOperator1ArgTest - TypicalLogicNot")
{
#ifdef TEST_IS_CPP20_OR_HIGHER
    constexpr std::equal_to<int> eq;

    static_assert(eq(true, logic_not(false)));
    static_assert(eq(true, false | logic_not()));

    static_assert(eq(false, logic_not(logic_not(false))));
    static_assert(eq(false, false | logic_not() | logic_not()));
#endif

    CHECK(noexcept(logic_not(false)));
    CHECK(noexcept(false | logic_not()));

    CHECK(true == logic_not(false));
    CHECK(true == (false | logic_not()));

    CHECK(false == logic_not(logic_not(false)));
    CHECK(false == (false | logic_not() | logic_not()));
}

namespace {

// Example function with reference
struct add1_t : pipe_::pipe_operator<add1_t, 1> {
    using pipe_::pipe_operator<add1_t, 1>::operator();

    CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR void operator()(int& v) const noexcept
    {
        ++v;
    }
};
inline constexpr add1_t add1{};

#ifdef TEST_IS_CPP20_OR_HIGHER
consteval bool test_() noexcept
{
    int a = 0;

    add1(a);
    if (a != 1) {
        return false;
    }

    a | add1();
    if (a != 2) {
        return false;
    }

    return true;
}
static_assert(test_(), "");
#endif

}  // namespace

TEST_CASE("PipeOperator1ArgTest - ReferenceTest")
{
    int a = 0;

    CHECK(noexcept(add1(a)));
    CHECK(noexcept(a | add1()));

    add1(a);
    CHECK(a == 1);

    a | add1();
    CHECK(a == 2);
}

// tests for exceptions
namespace {

struct could_not_past_zero_t : pipe_::pipe_operator<could_not_past_zero_t, 1> {
    using pipe_::pipe_operator<could_not_past_zero_t, 1>::operator();

    CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR bool operator()(const int v) const
    {
        if (v == 0) {
            throw std::logic_error("could not past zero");
        }
        return true;
    }
};
inline constexpr could_not_past_zero_t could_not_past_zero{};

}  // namespace

TEST_CASE("PipeOperator1ArgTest - ExceptionTest")
{
    CHECK_FALSE(noexcept(could_not_past_zero(1)));
    CHECK_FALSE(noexcept(1 | could_not_past_zero()));

    REQUIRE_NOTHROW(could_not_past_zero(1));
    REQUIRE_NOTHROW(1 | could_not_past_zero());
    CHECK(could_not_past_zero(1));
    CHECK((1 | could_not_past_zero()));

    CHECK_THROWS_AS(could_not_past_zero(0), std::logic_error);
    CHECK_THROWS_AS((0 | could_not_past_zero()), std::logic_error);
}

// callable object
namespace {

struct Increment : pipe_::pipe_operator<Increment, 1> {
    using pipe_::pipe_operator<Increment, 1>::operator();

    CCC_PIPE_OPERATOR_HELPER_CPP20_CONSTEXPR int operator()(const int v) const noexcept
    {
        return v + 1;
    }
};
inline constexpr Increment increment{};

}  // namespace

TEST_CASE("PipeOperator1ArgTest - CallableObjectTest")
{
    CHECK(noexcept(increment(1)));
    CHECK(noexcept(1 | increment()));

    CHECK(increment(1) == 2);
    CHECK((1 | increment()) == 2);
}

// NOLINTEND(*-use-transparent-functors)
