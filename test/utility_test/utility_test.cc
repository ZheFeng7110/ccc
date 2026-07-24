#ifndef TEST_USE_MODULE
#include "ccc/utility.hh"
#endif

#include "test_pch.hh"

#include <vector>
#include <algorithm>

#ifdef TEST_USE_MODULE
import ccc.utility;
#endif

namespace {

struct Constructible {
    int a;
    double b;

    Constructible(int a_, double b_) : a(a_), b(b_) {}
};

struct DefaultConstructible {
    int value;
    DefaultConstructible() : value(99) {}
};

}  // namespace

TEST_CASE("UtilityTest - InPlaceTag")
{
    ccc::in_place_t tag{};
    const auto& ref = ccc::in_place;
    (void)tag;
    (void)ref;
}

TEST_CASE("UtilityTest - AsConst")
{
    int value = 42;
    const int& cref = ccc::as_const(value);
    CHECK(&value == &cref);
    static_assert(std::is_same<const int&, decltype(ccc::as_const(value))>::value, "");
}

TEST_CASE("UtilityTest - ConstructAt")
{
    alignas(Constructible) unsigned char buf[sizeof(Constructible)];
    Constructible* ptr = ccc::construct_at(reinterpret_cast<Constructible*>(buf), 42, 3.14);
    REQUIRE(ptr != nullptr);
    CHECK(42 == ptr->a);
    CHECK(ptr->b == Approx(3.14));
    ccc::destroy_at(ptr);
}

TEST_CASE("UtilityTest - ConstructAtTrivial")
{
    alignas(int) unsigned char buf[sizeof(int)];
    int* ptr = ccc::construct_at(reinterpret_cast<int*>(buf), 42);
    REQUIRE(ptr != nullptr);
    CHECK(42 == *ptr);
    ccc::destroy_at(ptr);
}

TEST_CASE("UtilityTest - DestroyRange")
{
    alignas(int) unsigned char buf[3 * sizeof(int)];
    int* arr = reinterpret_cast<int*>(buf);
    ccc::construct_at(arr, 1);
    ccc::construct_at(arr + 1, 2);
    ccc::construct_at(arr + 2, 3);
    ccc::destroy(arr, arr + 3);
}

TEST_CASE("UtilityTest - UninitializedCopy")
{
    std::vector<int> src = {1, 2, 3, 4, 5};
    alignas(int) unsigned char buf[5 * sizeof(int)];
    int* dest = reinterpret_cast<int*>(buf);
    int* end = ccc::uninitialized_copy(src.begin(), src.end(), dest);
    REQUIRE(dest + 5 == end);
    for (int i = 0; i < 5; ++i) {
        CHECK(src[i] == dest[i]);
    }
    ccc::destroy(dest, end);
}

TEST_CASE("UtilityTest - UninitializedFillN")
{
    alignas(int) unsigned char buf[5 * sizeof(int)];
    int* dest = reinterpret_cast<int*>(buf);
    ccc::uninitialized_fill_n(dest, 5, 42);
    for (int i = 0; i < 5; ++i) {
        CHECK(42 == dest[i]);
    }
    ccc::destroy(dest, dest + 5);
}

TEST_CASE("UtilityTest - UninitializedDefaultConstructN")
{
    alignas(DefaultConstructible) unsigned char buf[5 * sizeof(DefaultConstructible)];
    DefaultConstructible* dest = reinterpret_cast<DefaultConstructible*>(buf);
    DefaultConstructible* end = ccc::uninitialized_default_construct_n(dest, 5);
    REQUIRE(dest + 5 == end);
    for (int i = 0; i < 5; ++i) {
        CHECK(99 == dest[i].value);
    }
    ccc::destroy(dest, end);
}
