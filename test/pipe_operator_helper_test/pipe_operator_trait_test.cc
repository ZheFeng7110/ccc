/**
 * @file pipe_operator_trait_test.cc
 * @brief Tests for is_pipe_operator and is_pipe_operator_v type traits
 */

#ifndef TEST_USE_MODULE
#include "ccc/pipe_operator_helper.hh"
#endif

#include "test_pch.hh"

#include <string>
#include <type_traits>

#ifdef TEST_USE_MODULE
import ccc.pipe_operator_helper;
#endif

namespace {

namespace pipe_ = ccc::pipe_operator_helper;

struct logic_not_t : pipe_::pipe_operator<logic_not_t, 1> {
    using pipe_::pipe_operator<logic_not_t, 1>::operator();

    constexpr bool operator()(const bool b) const noexcept
    {
        return !b;
    }
};

struct add_t : pipe_::pipe_operator<add_t, 2> {
    using pipe_::pipe_operator<add_t, 2>::operator();

    constexpr int operator()(const int a, const int b) const noexcept
    {
        return a + b;
    }
};

struct not_a_pipe_operator {
    int operator()(const int v) const noexcept
    {
        return v + 1;
    }
};

}  // namespace

TEST(PipeOperatorTraitTest, PipeOperatorTypesAreIdentified)
{
    static_assert(pipe_::is_pipe_operator_v<logic_not_t>, "logic_not_t should be a pipe operator type");
    static_assert(pipe_::is_pipe_operator_v<add_t>, "add_t should be a pipe operator type");
    static_assert(pipe_::is_pipe_operator_v<pipe_::pipe_operator_base>, "pipe_operator_base itself should be a pipe operator type");

    EXPECT_TRUE((pipe_::is_pipe_operator_v<logic_not_t>));
    EXPECT_TRUE((pipe_::is_pipe_operator_v<add_t>));
    EXPECT_TRUE((pipe_::is_pipe_operator_v<pipe_::pipe_operator_base>));
}

TEST(PipeOperatorTraitTest, NonPipeOperatorTypesAreRejected)
{
    static_assert(!pipe_::is_pipe_operator_v<not_a_pipe_operator>, "not_a_pipe_operator should not be a pipe operator type");
    static_assert(!pipe_::is_pipe_operator_v<int>);
    static_assert(!pipe_::is_pipe_operator_v<double>);
    static_assert(!pipe_::is_pipe_operator_v<std::string>);
    static_assert(!pipe_::is_pipe_operator_v<int*>);
    static_assert(!pipe_::is_pipe_operator_v<int&>);

    EXPECT_FALSE((pipe_::is_pipe_operator_v<not_a_pipe_operator>));
    EXPECT_FALSE((pipe_::is_pipe_operator_v<int>));
    EXPECT_FALSE((pipe_::is_pipe_operator_v<double>));
    EXPECT_FALSE((pipe_::is_pipe_operator_v<std::string>));
}

TEST(PipeOperatorTraitTest, ConstAndVolatileQualifiedTypes)
{
    static_assert(pipe_::is_pipe_operator_v<const logic_not_t>);
    static_assert(pipe_::is_pipe_operator_v<volatile logic_not_t>);
    static_assert(pipe_::is_pipe_operator_v<const volatile logic_not_t>);

    EXPECT_TRUE((pipe_::is_pipe_operator_v<const logic_not_t>));
    EXPECT_TRUE((pipe_::is_pipe_operator_v<volatile logic_not_t>));
    EXPECT_TRUE((pipe_::is_pipe_operator_v<const volatile logic_not_t>));
}

TEST(PipeOperatorTraitTest, IsPipeOperatorAliasMatchesVariableTemplate)
{
    EXPECT_EQ(pipe_::is_pipe_operator<logic_not_t>::value, pipe_::is_pipe_operator_v<logic_not_t>);
    EXPECT_EQ(pipe_::is_pipe_operator<int>::value, pipe_::is_pipe_operator_v<int>);
    EXPECT_EQ(pipe_::is_pipe_operator<not_a_pipe_operator>::value, pipe_::is_pipe_operator_v<not_a_pipe_operator>);
}
