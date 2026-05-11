#ifndef TEST_USE_MODULE
#include "ccc/expected.hh"
#endif

#include "test_pch.hh"

#include <array>
#include <string>
#include <tuple>
#include <utility>

#ifdef TEST_USE_MODULE
import ccc.expected;
#endif

#if (__cplusplus < 201703L)

TEST(ExpectedMonadic, SkippedBeforeCpp17)
{
    GTEST_SKIP() << "ccc::expected monadic tests are ignored: requires C++17 or higher.";
}

#else

TEST(ExpectedMonadic, AndThenPassesValueAndCriterionWhenHasValue)
{
    ccc::expected<int, std::string> src(42);
    bool called = false;

    auto result = src.and_then([&](const int value, const ccc::default_criterion& criterion) {
        called = true;
        EXPECT_EQ(42, value);
        EXPECT_TRUE(criterion.has_value());
        return ccc::expected<long, std::string>(value + 1);
    });

    EXPECT_TRUE(called);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(43, result.value());
}

TEST(ExpectedMonadic, AndThenSkipsCallbackWhenHasError)
{
    ccc::expected<int, std::string> src(ccc::unexpect, "err");
    bool called = false;

    auto result = src.and_then([&](const int, const ccc::default_criterion&) {
        called = true;
        return ccc::expected<long, std::string>(0L);
    });

    EXPECT_FALSE(called);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ("err", result.error());
}

TEST(ExpectedMonadic, OrElsePassesErrorAndCriterionWhenHasError)
{
    ccc::expected<int, std::string> src(ccc::unexpect, "old");
    bool called = false;

    auto result = src.or_else([&](const std::string& error, const ccc::default_criterion& criterion) {
        called = true;
        EXPECT_EQ("old", error);
        EXPECT_FALSE(criterion.has_value());
        return ccc::expected<int, std::string>(100);
    });

    EXPECT_TRUE(called);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(100, result.value());
}

TEST(ExpectedMonadic, OrElseSkipsCallbackWhenHasValue)
{
    ccc::expected<int, std::string> src(7);
    bool called = false;

    auto result = src.or_else([&](const std::string&, const ccc::default_criterion&) {
        called = true;
        return ccc::expected<int, std::string>(ccc::unexpect, "new");
    });

    EXPECT_FALSE(called);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(7, result.value());
}

TEST(ExpectedMonadic, TransformSingleValueKeepsDefaultCriterion)
{
    ccc::expected<int, std::string> src(12);

    auto result = src.transform([](const int value, const ccc::default_criterion& criterion) {
        EXPECT_TRUE(criterion.has_value());
        return value * 3;
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(36, result.value());
}

TEST(ExpectedMonadic, TransformPairAndTupleUseReturnedCriterion)
{
    ccc::expected<int, std::string> src(5);

    auto pair_result = src.transform([](const int value, const ccc::default_criterion& criterion) {
        EXPECT_TRUE(criterion.has_value());
        return std::make_pair(value + 1, ccc::default_criterion(true));
    });
    ASSERT_TRUE(pair_result.has_value());
    EXPECT_EQ(6, pair_result.value());

    auto tuple_result = src.transform([](const int value, const ccc::default_criterion& criterion) {
        EXPECT_TRUE(criterion.has_value());
        return std::make_tuple(value + 2, ccc::default_criterion(true));
    });
    ASSERT_TRUE(tuple_result.has_value());
    EXPECT_EQ(7, tuple_result.value());
}

TEST(ExpectedMonadic, TransformArrayUseReturnedCriterion)
{
    ccc::expected<ccc::default_criterion, std::string> src(ccc::default_criterion(true));

    auto result = src.transform([](const ccc::default_criterion& value, const ccc::default_criterion& criterion) {
        EXPECT_TRUE(value.has_value());
        EXPECT_TRUE(criterion.has_value());
        return std::array<ccc::default_criterion, 2>{ccc::default_criterion(true), ccc::default_criterion(true)};
    });

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().has_value());
}

TEST(ExpectedMonadic, TransformErrorSingleValueUsesDefaultErrorCriterion)
{
    ccc::expected<int, std::string> src(ccc::unexpect, "old");

    auto result = src.transform_error([](const std::string& error, const ccc::default_criterion& criterion) {
        EXPECT_EQ("old", error);
        EXPECT_FALSE(criterion.has_value());
        return error + "_new";
    });

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ("old_new", result.error());
}

TEST(ExpectedMonadic, TransformErrorPairAndArrayUseReturnedCriterion)
{
    ccc::expected<int, std::string> src(ccc::unexpect, "bad");

    auto pair_result = src.transform_error([](const std::string& error, const ccc::default_criterion& criterion) {
        EXPECT_FALSE(criterion.has_value());
        return std::make_pair(error + "!", ccc::default_criterion(false));
    });
    ASSERT_FALSE(pair_result.has_value());
    EXPECT_EQ("bad!", pair_result.error());

    ccc::expected<int, ccc::default_criterion> src_array(ccc::unexpect, ccc::default_criterion(false));
    auto array_result =
        src_array.transform_error([](const ccc::default_criterion& error, const ccc::default_criterion& criterion) {
            EXPECT_FALSE(error.has_value());
            EXPECT_FALSE(criterion.has_value());
            return std::array<ccc::default_criterion, 2>{ccc::default_criterion(false), ccc::default_criterion(false)};
        });

    ASSERT_FALSE(array_result.has_value());
    EXPECT_FALSE(array_result.error().has_value());
}

#endif
