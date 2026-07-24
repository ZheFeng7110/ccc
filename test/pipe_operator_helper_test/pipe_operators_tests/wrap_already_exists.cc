/**
 * @file wrap_already_exists.cc
 * @brief
 */

#include "ccc/detail/config.hh"

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

namespace {

namespace pipe_ = ccc::pipe_operator_helper;

CCC_CPP20_CONSTEXPR int add(int a, int b) noexcept
{
    return a + b;
}

CCC_CPP20_CONSTEXPR int divide(int a, int b)
{
    if (b == 0) {
        throw std::logic_error("divide by zero");
    }
    return a / b;
}

struct add_t : pipe_::pipe_operator<add_t, 2> {
    using pipe_::pipe_operator<add_t, 2>::operator();

    CCC_CPP20_CONSTEXPR int operator()(const int a, const int b) const noexcept
    {
        return add(a, b);
    }
};
namespace pp {
inline constexpr add_t add{};
}

struct divide_t : pipe_::pipe_operator<divide_t, 2> {
    using pipe_::pipe_operator<divide_t, 2>::operator();

    CCC_CPP20_CONSTEXPR int operator()(const int a, const int b) const
    {
        return divide(a, b);
    }
};
namespace pp {
inline constexpr divide_t divide{};
}

}  // namespace

TEST_CASE("PipeOperatorWrapAlreadyExistsTest - Add")
{
#ifdef TEST_IS_CPP20_OR_HIGHER
    static_assert(2 == pp::add(1, 1));
    static_assert(2 == (1 | pp::add(1)));

    static_assert(114515 == pp::add(pp::add(114514, 1), 0));
    static_assert(114515 == (114514 | pp::add(1) | pp::add(0)));
#endif

    CHECK(noexcept(pp::add(1, 1)));
    CHECK(noexcept(1 | pp::add(1)));

    CHECK(2 == pp::add(1, 1));
    CHECK(2 == (1 | pp::add(1)));

    CHECK(114515 == pp::add(pp::add(114514, 1), 0));
    CHECK(114515 == (114514 | pp::add(1) | pp::add(0)));
}

TEST_CASE("PipeOperatorWrapAlreadyExistsTest - Divide")
{
#ifdef TEST_IS_CPP20_OR_HIGHER
    constexpr std::equal_to<int> eq;

    static_assert(requires { pp::divide(1, 0); });
    static_assert(requires { 1 | pp::divide(0); });

    static_assert(eq(2, pp::divide(4, 2)));
    static_assert(eq(2, 4 | pp::divide(2)));

    static_assert(eq(-1, pp::divide(pp::divide(4, 2), -2)));
    static_assert(eq(-1, 4 | pp::divide(2) | pp::divide(-2)));
#endif

    CHECK_FALSE(noexcept(pp::divide(1, 1)));
    CHECK_FALSE(noexcept(1 | pp::divide(1)));

    CHECK(2 == pp::divide(4, 2));
    CHECK(2 == (4 | pp::divide(2)));

    CHECK_THROWS_AS(pp::divide(2, 0), std::logic_error);
    CHECK_THROWS_AS((2 | pp::divide(0)), std::logic_error);

    CHECK(-1 == pp::divide(pp::divide(4, 2), -2));
    CHECK(-1 == (4 | pp::divide(2) | pp::divide(-2)));

    CHECK_THROWS_AS(pp::divide(pp::divide(-4, 2), 0), std::logic_error);
    CHECK_THROWS_AS((-4 | pp::divide(2) | pp::divide(0)), std::logic_error);
}

namespace {
namespace tmpl {

template<typename T>
CCC_CPP20_CONSTEXPR T add(T a, T b) noexcept
{
    return a + b;
}

struct add_t : pipe_::pipe_operator<add_t, 2> {
    using pipe_::pipe_operator<add_t, 2>::operator();

    template<typename T>
    CCC_CPP20_CONSTEXPR T operator()(T a, T b) const noexcept
    {
        return add<T>(a, b);
    }
};
namespace pp {
inline constexpr add_t add{};
}

}  // namespace tmpl
}  // namespace

TEST_CASE("PipeOperatorWrapAlreadyExistsTest - TemplateAdd")
{
#ifdef TEST_IS_CPP20_OR_HIGHER
    constexpr std::equal_to<double> eq;

    static_assert(2 == tmpl::pp::add(1, 1));
    static_assert(2 == (1 | tmpl::pp::add(1)));

    static_assert(114515 == tmpl::pp::add(tmpl::pp::add(114514, 1), 0));
    static_assert(114515 == (114514 | tmpl::pp::add(1) | tmpl::pp::add(0)));

    static_assert(eq(2.0, tmpl::pp::add.operator()<double>(1.0, 1)));
    static_assert(eq(2.0, 1.0 | tmpl::pp::add.operator()<double>(1)));
#endif

    CHECK(noexcept(tmpl::pp::add(1, 1)));
    CHECK(noexcept(1 | tmpl::pp::add(1)));

    CHECK(2 == tmpl::pp::add(1, 1));
    CHECK(2 == (1 | tmpl::pp::add(1)));

    CHECK(114515 == tmpl::pp::add(tmpl::pp::add(114514, 1), 0));
    CHECK(114515 == (114514 | tmpl::pp::add(1) | tmpl::pp::add(0)));

    CHECK(2.0 == Approx(tmpl::pp::add.operator()<double>(1.0, 1)));
    CHECK(2.0 == Approx((1.0 | tmpl::pp::add.operator()<double>(1))));

    CHECK((std::is_same<decltype(tmpl::pp::add.operator()<double>(1.0, 1)), double>::value));
    CHECK((std::is_same<decltype(1.0 | tmpl::pp::add.operator()<double>(1)), double>::value));
}
