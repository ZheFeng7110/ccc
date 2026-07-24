#ifndef TEST_USE_MODULE
#include "ccc/expected.hh"
#endif

#include "test_pch.hh"

#include <array>
#include <string>
#include <tuple>
#include <utility>

#ifdef TEST_USE_MODULE
import ccc.utility;
import ccc.expected;
#endif

#if (__cplusplus >= 201703L)

TEST_CASE("ExpectedMonadic - AndThenPassesValueAndCriterionWhenHasValue")
{
    ccc::expected<int, std::string> src(42);
    bool called = false;

    auto result = src.and_then([&](const int value, const ccc::default_criterion& criterion) {
        called = true;
        CHECK(42 == value);
        CHECK(criterion.has_value());
        return ccc::expected<long, std::string>(value + 1);
    });

    CHECK(called);
    REQUIRE(result.has_value());
    CHECK(43 == result.value());
}

TEST_CASE("ExpectedMonadic - AndThenSkipsCallbackWhenHasError")
{
    ccc::expected<int, std::string> src(ccc::unexpect, "err");
    bool called = false;

    auto result = src.and_then([&](const int, const ccc::default_criterion&) {
        called = true;
        return ccc::expected<long, std::string>(0L);
    });

    CHECK_FALSE(called);
    REQUIRE_FALSE(result.has_value());
    CHECK("err" == result.error());
}

TEST_CASE("ExpectedMonadic - OrElsePassesErrorAndCriterionWhenHasError")
{
    ccc::expected<int, std::string> src(ccc::unexpect, "old");
    bool called = false;

    auto result = src.or_else([&](const std::string& error, const ccc::default_criterion& criterion) {
        called = true;
        CHECK("old" == error);
        CHECK_FALSE(criterion.has_value());
        return ccc::expected<int, std::string>(100);
    });

    CHECK(called);
    REQUIRE(result.has_value());
    CHECK(100 == result.value());
}

TEST_CASE("ExpectedMonadic - OrElseSkipsCallbackWhenHasValue")
{
    ccc::expected<int, std::string> src(7);
    bool called = false;

    auto result = src.or_else([&](const std::string&, const ccc::default_criterion&) {
        called = true;
        return ccc::expected<int, std::string>(ccc::unexpect, "new");
    });

    CHECK_FALSE(called);
    REQUIRE(result.has_value());
    CHECK(7 == result.value());
}

TEST_CASE("ExpectedMonadic - TransformSingleValueKeepsDefaultCriterion")
{
    ccc::expected<int, std::string> src(12);

    auto result = src.transform([](const int value, const ccc::default_criterion& criterion) {
        CHECK(criterion.has_value());
        return value * 3;
    });

    REQUIRE(result.has_value());
    CHECK(36 == result.value());
}

TEST_CASE("ExpectedMonadic - TransformPairAndTupleUseReturnedCriterion")
{
    ccc::expected<int, std::string> src(5);

    auto pair_result = src.transform([](const int value, const ccc::default_criterion& criterion) {
        CHECK(criterion.has_value());
        return std::make_pair(value + 1, ccc::default_criterion(true));
    });
    REQUIRE(pair_result.has_value());
    CHECK(6 == pair_result.value());

    auto tuple_result = src.transform([](const int value, const ccc::default_criterion& criterion) {
        CHECK(criterion.has_value());
        return std::make_tuple(value + 2, ccc::default_criterion(true));
    });
    REQUIRE(tuple_result.has_value());
    CHECK(7 == tuple_result.value());
}

TEST_CASE("ExpectedMonadic - TransformArrayUseReturnedCriterion")
{
    ccc::expected<ccc::default_criterion, std::string> src(ccc::default_criterion(true));

    auto result = src.transform([](const ccc::default_criterion& value, const ccc::default_criterion& criterion) {
        CHECK(value.has_value());
        CHECK(criterion.has_value());
        return std::array<ccc::default_criterion, 2>{ccc::default_criterion(true), ccc::default_criterion(true)};
    });

    REQUIRE(result.has_value());
    CHECK(result.value().has_value());
}

TEST_CASE("ExpectedMonadic - TransformErrorSingleValueUsesDefaultErrorCriterion")
{
    ccc::expected<int, std::string> src(ccc::unexpect, "old");

    auto result = src.transform_error([](const std::string& error, const ccc::default_criterion& criterion) {
        CHECK("old" == error);
        CHECK_FALSE(criterion.has_value());
        return error + "_new";
    });

    REQUIRE_FALSE(result.has_value());
    CHECK("old_new" == result.error());
}

TEST_CASE("ExpectedMonadic - TransformErrorPairAndArrayUseReturnedCriterion")
{
    ccc::expected<int, std::string> src(ccc::unexpect, "bad");

    auto pair_result = src.transform_error([](const std::string& error, const ccc::default_criterion& criterion) {
        CHECK_FALSE(criterion.has_value());
        return std::make_pair(error + "!", ccc::default_criterion(false));
    });
    REQUIRE_FALSE(pair_result.has_value());
    CHECK("bad!" == pair_result.error());

    ccc::expected<int, ccc::default_criterion> src_array(ccc::unexpect, ccc::default_criterion(false));
    auto array_result =
        src_array.transform_error([](const ccc::default_criterion& error, const ccc::default_criterion& criterion) {
            CHECK_FALSE(error.has_value());
            CHECK_FALSE(criterion.has_value());
            return std::array<ccc::default_criterion, 2>{ccc::default_criterion(false), ccc::default_criterion(false)};
        });

    REQUIRE_FALSE(array_result.has_value());
    CHECK_FALSE(array_result.error().has_value());
}

#endif
