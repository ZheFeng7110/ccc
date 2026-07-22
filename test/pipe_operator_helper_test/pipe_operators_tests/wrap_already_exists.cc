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

TEST(PipeWrapAlreadyExistsTest, Add)
{
#ifdef TEST_IS_CPP20_OR_HIGHER
    static_assert(2 == pp::add(1, 1));
    static_assert(2 == (1 | pp::add(1)));

    static_assert(114515 == pp::add(pp::add(114514, 1), 0));
    static_assert(114515 == (114514 | pp::add(1) | pp::add(0)));
#endif

    EXPECT_TRUE(noexcept(pp::add(1, 1)));
    EXPECT_TRUE(noexcept(1 | pp::add(1)));

    EXPECT_EQ(2, pp::add(1, 1));
    EXPECT_EQ(2, 1 | pp::add(1));

    EXPECT_EQ(114515, pp::add(pp::add(114514, 1), 0));
    EXPECT_EQ(114515, 114514 | pp::add(1) | pp::add(0));
}

TEST(PipeWrapAlreadyExistsTest, Divide)
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

    EXPECT_FALSE(noexcept(pp::divide(1, 1)));
    EXPECT_FALSE(noexcept(1 | pp::divide(1)));

    EXPECT_EQ(2, pp::divide(4, 2));
    EXPECT_EQ(2, 4 | pp::divide(2));

    EXPECT_THROW(pp::divide(2, 0), std::logic_error);
    EXPECT_THROW(2 | pp::divide(0), std::logic_error);

    EXPECT_EQ(-1, pp::divide(pp::divide(4, 2), -2));
    EXPECT_EQ(-1, 4 | pp::divide(2) | pp::divide(-2));

    EXPECT_THROW(pp::divide(pp::divide(-4, 2), 0), std::logic_error);
    EXPECT_THROW(-4 | pp::divide(2) | pp::divide(0), std::logic_error);
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

TEST(PipeWrapAlreadyExistsTest, TemplateAdd)
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

    EXPECT_TRUE(noexcept(tmpl::pp::add(1, 1)));
    EXPECT_TRUE(noexcept(1 | tmpl::pp::add(1)));

    EXPECT_EQ(2, tmpl::pp::add(1, 1));
    EXPECT_EQ(2, 1 | tmpl::pp::add(1));

    EXPECT_EQ(114515, tmpl::pp::add(tmpl::pp::add(114514, 1), 0));
    EXPECT_EQ(114515, 114514 | tmpl::pp::add(1) | tmpl::pp::add(0));

    EXPECT_DOUBLE_EQ(2.0, tmpl::pp::add.operator()<double>(1.0, 1));
    EXPECT_DOUBLE_EQ(2.0, 1.0 | tmpl::pp::add.operator()<double>(1));

    EXPECT_TRUE((std::is_same<decltype(tmpl::pp::add.operator()<double>(1.0, 1)), double>::value));
    EXPECT_TRUE((std::is_same<decltype(1.0 | tmpl::pp::add.operator()<double>(1)), double>::value));
}
