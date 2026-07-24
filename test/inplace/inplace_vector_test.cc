#ifndef TEST_USE_MODULE
#include "ccc/inplace/inplace_vector.hh"
#endif

#include "test_pch.hh"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef TEST_USE_MODULE
import ccc.inplace.inplace_vector;
#endif

namespace {

struct Counter {
    int value;
    static int constructed;
    static int destructed;
    static int copied;
    static int moved;

    Counter() : value(0)
    {
        ++constructed;
    }
    explicit Counter(int v) : value(v)
    {
        ++constructed;
    }
    Counter(const Counter& o) : value(o.value)
    {
        ++constructed;
        ++copied;
    }
    Counter(Counter&& o) noexcept : value(o.value)
    {
        o.value = -1;
        ++constructed;
        ++moved;
    }
    Counter& operator=(const Counter& o)
    {
        value = o.value;
        ++copied;
        return *this;
    }
    Counter& operator=(Counter&& o) noexcept
    {
        value = o.value;
        o.value = -1;
        ++moved;
        return *this;
    }
    ~Counter()
    {
        ++destructed;
    }

    bool operator==(const Counter& o) const
    {
        return value == o.value;
    }
    bool operator!=(const Counter& o) const
    {
        return value != o.value;
    }
    bool operator<(const Counter& o) const
    {
        return value < o.value;
    }

    static void reset()
    {
        constructed = 0;
        destructed = 0;
        copied = 0;
        moved = 0;
    }
};

int Counter::constructed = 0;
int Counter::destructed = 0;
int Counter::copied = 0;
int Counter::moved = 0;

}  // namespace

TEST_CASE("InplaceVector - DefaultConstruct")
{
    ccc::inplace_vector<int, 5> v;

    CHECK(v.empty());
    CHECK_FALSE(v.full());
    CHECK(0u == v.size());
    CHECK(5u == v.capacity());
    CHECK(5u == v.max_size());
}

TEST_CASE("InplaceVector - PushBack")
{
    ccc::inplace_vector<int, 5> v;

    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    CHECK(3u == v.size());
    CHECK(1 == v[0]);
    CHECK(2 == v[1]);
    CHECK(3 == v[2]);
    CHECK(1 == v.front());
    CHECK(3 == v.back());
}

TEST_CASE("InplaceVector - PushBackRvalue")
{
    ccc::inplace_vector<std::string, 3> v;
    std::string s = "hello";

    v.push_back(std::move(s));

    CHECK("hello" == v[0]);
}

TEST_CASE("InplaceVector - TryPushBack")
{
    ccc::inplace_vector<int, 3> v;

    CHECK(nullptr != v.try_push_back(1));
    CHECK(nullptr != v.try_push_back(2));
    CHECK(nullptr != v.try_push_back(3));
    CHECK(nullptr == v.try_push_back(4));

    CHECK(3u == v.size());
}

TEST_CASE("InplaceVector - UncheckedPushBack")
{
    ccc::inplace_vector<int, 3> v;

    auto p = v.unchecked_push_back(10);
    CHECK(10 == *p);

    v.unchecked_push_back(20);
    v.unchecked_push_back(30);

    CHECK(3u == v.size());
    CHECK(10 == v[0]);
}

TEST_CASE("InplaceVector - PushBackThrowsWhenFull")
{
    ccc::inplace_vector<int, 2> v;
    v.push_back(1);
    v.push_back(2);

    CHECK_THROWS_AS(v.push_back(3), std::bad_alloc);
}

TEST_CASE("InplaceVector - PopBack")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    v.pop_back();
    CHECK(2u == v.size());
    CHECK(2 == v.back());

    v.pop_back();
    CHECK(1u == v.size());
    CHECK(1 == v.back());

    v.pop_back();
    CHECK(v.empty());
}

TEST_CASE("InplaceVector - EmplaceBack")
{
    ccc::inplace_vector<std::string, 5> v;

    v.emplace_back(3, 'a');

    CHECK("aaa" == v.back());
    CHECK(1u == v.size());
}

TEST_CASE("InplaceVector - TryEmplaceBack")
{
    ccc::inplace_vector<int, 2> v;

    CHECK(nullptr != v.try_emplace_back(1));
    CHECK(nullptr != v.try_emplace_back(2));
    CHECK(nullptr == v.try_emplace_back(3));

    CHECK(2u == v.size());
}

TEST_CASE("InplaceVector - UncheckedEmplaceBack")
{
    ccc::inplace_vector<std::string, 5> v;

    auto p = v.unchecked_emplace_back(2, 'x');
    CHECK("xx" == *p);
    CHECK(1u == v.size());
}

TEST_CASE("InplaceVector - CopyConstruct")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    ccc::inplace_vector<int, 5> v2(v);

    CHECK(3u == v2.size());
    CHECK(10 == v2[0]);
    CHECK(20 == v2[1]);
    CHECK(30 == v2[2]);
    CHECK(3u == v.size());
}

TEST_CASE("InplaceVector - MoveConstruct")
{
    ccc::inplace_vector<std::string, 5> v;
    v.push_back("hello");
    v.push_back("world");

    ccc::inplace_vector<std::string, 5> v2(std::move(v));

    CHECK(2u == v2.size());
    CHECK("hello" == v2[0]);
    CHECK("world" == v2[1]);
}

TEST_CASE("InplaceVector - InitializerListConstruct")
{
    ccc::inplace_vector<int, 5> v = {10, 20, 30};

    CHECK(3u == v.size());
    CHECK(10 == v[0]);
    CHECK(20 == v[1]);
    CHECK(30 == v[2]);
}

TEST_CASE("InplaceVector - InitializerListConstructThrowsWhenTooLarge")
{
    CHECK_THROWS_AS((ccc::inplace_vector<int, 2>{1, 2, 3}), std::bad_alloc);
}

TEST_CASE("InplaceVector - DestructorCallsElementDestructors")
{
    Counter::reset();
    {
        ccc::inplace_vector<Counter, 5> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);
    }
    CHECK(Counter::constructed == Counter::destructed);
}

TEST_CASE("InplaceVector - CopyAssignment")
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    a.push_back(2);

    ccc::inplace_vector<int, 5> b;
    b.push_back(9);

    b = a;

    CHECK(2u == b.size());
    CHECK(1 == b[0]);
    CHECK(2 == b[1]);
    CHECK(2u == a.size());
}

TEST_CASE("InplaceVector - MoveAssignment")
{
    ccc::inplace_vector<std::string, 5> a;
    a.push_back("x");
    a.push_back("y");

    ccc::inplace_vector<std::string, 5> b;
    b.push_back("z");

    b = std::move(a);

    CHECK(2u == b.size());
    CHECK("x" == b[0]);
    CHECK("y" == b[1]);
}

TEST_CASE("InplaceVector - InitializerListAssignment")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);

    v = {10, 20, 30};

    CHECK(3u == v.size());
    CHECK(10 == v[0]);
}

TEST_CASE("InplaceVector - SelfAssignment")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    v = v;
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    CHECK(2u == v.size());
    CHECK(1 == v[0]);
    CHECK(2 == v[1]);
}

TEST_CASE("InplaceVector - AssignCount")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);

    v.assign(5, 42);

    CHECK(5u == v.size());
    CHECK(42 == v[0]);
    CHECK(42 == v[4]);
}

TEST_CASE("InplaceVector - AssignRange")
{
    std::vector<int> src = {3, 5, 7, 9};
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.assign(src.begin(), src.end());

    CHECK(4u == v.size());
    CHECK(3 == v[0]);
    CHECK(9 == v[3]);
}

TEST_CASE("InplaceVector - AssignInitializerList")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.assign({4, 5, 6});

    CHECK(3u == v.size());
    CHECK(4 == v[0]);
}

TEST_CASE("InplaceVector - AssignRangeFreeFunc")
{
    int arr[] = {7, 8, 9};
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.assign_range(arr);

    CHECK(3u == v.size());
    CHECK(7 == v[0]);
}

TEST_CASE("InplaceVector - AssignThrowsWhenTooLarge")
{
    ccc::inplace_vector<int, 3> v;
    CHECK_THROWS_AS(v.assign(5, 0), std::bad_alloc);
}

TEST_CASE("InplaceVector - InsertSingleCopy")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(5);

    auto it = v.insert(v.begin() + 1, 3);

    CHECK(3 == *it);
    CHECK(3u == v.size());
    CHECK(1 == v[0]);
    CHECK(3 == v[1]);
    CHECK(5 == v[2]);
}

TEST_CASE("InplaceVector - InsertSingleMove")
{
    ccc::inplace_vector<std::string, 10> v;
    v.push_back("a");
    v.push_back("c");

    std::string s = "b";
    v.insert(v.begin() + 1, std::move(s));

    CHECK(3u == v.size());
    CHECK("a" == v[0]);
    CHECK("b" == v[1]);
    CHECK("c" == v[2]);
}

TEST_CASE("InplaceVector - InsertAtEnd")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    auto it = v.insert(v.end(), 2);

    CHECK(2 == *it);
    CHECK(2u == v.size());
    CHECK(1 == v[0]);
    CHECK(2 == v[1]);
}

TEST_CASE("InplaceVector - InsertAtBegin")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(2);
    v.push_back(3);

    auto it = v.insert(v.begin(), 1);

    CHECK(1 == *it);
    CHECK(3u == v.size());
    CHECK(1 == v[0]);
    CHECK(2 == v[1]);
    CHECK(3 == v[2]);
}

TEST_CASE("InplaceVector - InsertCount")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    auto it = v.insert(v.begin() + 1, 3, 5);

    CHECK(5 == *it);
    CHECK(5u == v.size());
    CHECK(1 == v[0]);
    CHECK(5 == v[1]);
    CHECK(5 == v[2]);
    CHECK(5 == v[3]);
    CHECK(9 == v[4]);
}

TEST_CASE("InplaceVector - InsertCountZero")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    auto it = v.insert(v.begin(), 0, 99);

    CHECK(1 == *it);
    CHECK(1u == v.size());
}

TEST_CASE("InplaceVector - InsertRange")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    int arr[] = {3, 5, 7};
    auto it = v.insert(v.begin() + 1, arr, arr + 3);

    CHECK(3 == *it);
    CHECK(5u == v.size());
    CHECK(1 == v[0]);
    CHECK(3 == v[1]);
    CHECK(5 == v[2]);
    CHECK(7 == v[3]);
    CHECK(9 == v[4]);
}

TEST_CASE("InplaceVector - InsertInitializerList")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    v.insert(v.begin() + 1, {3, 5, 7});

    CHECK(5u == v.size());
    CHECK(3 == v[1]);
    CHECK(7 == v[3]);
}

TEST_CASE("InplaceVector - InsertThrowsWhenFull")
{
    ccc::inplace_vector<int, 2> v;
    v.push_back(1);
    v.push_back(2);

    CHECK_THROWS_AS(v.insert(v.begin(), 0), std::bad_alloc);
}

TEST_CASE("InplaceVector - Emplace")
{
    ccc::inplace_vector<std::string, 10> v;
    v.push_back("a");
    v.push_back("c");

    auto it = v.emplace(v.begin() + 1, 1, 'b');

    CHECK("b" == *it);
    CHECK(3u == v.size());
    CHECK("a" == v[0]);
    CHECK("b" == v[1]);
    CHECK("c" == v[2]);
}

TEST_CASE("InplaceVector - InsertRangeFreeFunc")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    int arr[] = {3, 5};
    v.insert_range(v.begin() + 1, arr);

    CHECK(4u == v.size());
    CHECK(3 == v[1]);
    CHECK(5 == v[2]);
}

TEST_CASE("InplaceVector - AppendRange")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    int arr[] = {2, 3, 4};
    v.append_range(arr);

    CHECK(4u == v.size());
    CHECK(1 == v[0]);
    CHECK(2 == v[1]);
    CHECK(4 == v[3]);
}

TEST_CASE("InplaceVector - TryAppendRange")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);

    int arr[] = {3, 4, 5};
    CHECK(v.try_append_range(arr));
    CHECK(5u == v.size());

    int arr2[] = {6};
    CHECK_FALSE(v.try_append_range(arr2));
    CHECK(5u == v.size());
}

TEST_CASE("InplaceVector - EraseSingle")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);

    auto it = v.erase(v.begin() + 1);

    CHECK(3 == *it);
    CHECK(3u == v.size());
    CHECK(1 == v[0]);
    CHECK(3 == v[1]);
    CHECK(4 == v[2]);
}

TEST_CASE("InplaceVector - EraseRange")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);

    auto it = v.erase(v.begin() + 1, v.begin() + 3);

    CHECK(4 == *it);
    CHECK(2u == v.size());
    CHECK(1 == v[0]);
    CHECK(4 == v[1]);
}

TEST_CASE("InplaceVector - EraseToEnd")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    auto it = v.erase(v.begin() + 1, v.end());

    CHECK(v.end() == it);
    CHECK(1u == v.size());
    CHECK(1 == v[0]);
}

TEST_CASE("InplaceVector - Clear")
{
    Counter::reset();
    {
        ccc::inplace_vector<Counter, 5> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.clear();

        CHECK(v.empty());
        CHECK(0u == v.size());
    }
    CHECK(Counter::constructed == Counter::destructed);
}

TEST_CASE("InplaceVector - ClearThenReuse")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.clear();

    CHECK(v.empty());

    v.push_back(3);

    CHECK(1u == v.size());
    CHECK(3 == v[0]);
}

TEST_CASE("InplaceVector - ResizeDefaultLarger")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);

    v.resize(5);

    CHECK(5u == v.size());
    CHECK(1 == v[0]);
    CHECK(2 == v[1]);
}

TEST_CASE("InplaceVector - ResizeDefaultSmaller")
{
    Counter::reset();
    {
        ccc::inplace_vector<Counter, 10> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);

        v.resize(1);

        CHECK(1u == v.size());
        CHECK(1 == v[0].value);
    }
    CHECK(Counter::constructed == Counter::destructed);
}

TEST_CASE("InplaceVector - ResizeWithValueLarger")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.resize(4, 7);

    CHECK(4u == v.size());
    CHECK(1 == v[0]);
    CHECK(7 == v[1]);
    CHECK(7 == v[3]);
}

TEST_CASE("InplaceVector - ResizeThrowsWhenTooLarge")
{
    ccc::inplace_vector<int, 3> v;
    CHECK_THROWS_AS(v.resize(10), std::out_of_range);
}

TEST_CASE("InplaceVector - Reserve")
{
    ccc::inplace_vector<int, 5> v;
    CHECK_NOTHROW(v.reserve(3));
    CHECK_NOTHROW(v.reserve(5));
    CHECK_THROWS_AS(v.reserve(6), std::bad_alloc);
}

TEST_CASE("InplaceVector - ShrinkToFit")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    CHECK_NOTHROW(v.shrink_to_fit());
    CHECK(1u == v.size());
}

TEST_CASE("InplaceVector - At")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(10);
    v.push_back(20);

    CHECK(10 == v.at(0));
    CHECK(20 == v.at(1));
    CHECK_THROWS_AS(v.at(2), std::out_of_range);
}

TEST_CASE("InplaceVector - Data")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);

    int* p = v.data();
    CHECK(1 == p[0]);
    CHECK(2 == p[1]);

    const auto& cv = v;
    const int* cp = cv.data();
    CHECK(1 == cp[0]);
}

TEST_CASE("InplaceVector - Iterators")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    int sum = 0;
    for (auto it = v.begin(); it != v.end(); ++it) {
        sum += *it;
    }
    CHECK(6 == sum);

    sum = 0;
    for (auto it = v.cbegin(); it != v.cend(); ++it) {
        sum += *it;
    }
    CHECK(6 == sum);
}

TEST_CASE("InplaceVector - ReverseIterators")
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    int sum = 0;
    for (auto it = v.rbegin(); it != v.rend(); ++it) {
        sum += *it;
    }
    CHECK(6 == sum);

    sum = 0;
    for (auto it = v.crbegin(); it != v.crend(); ++it) {
        sum += *it;
    }
    CHECK(6 == sum);
}

TEST_CASE("InplaceVector - Swap")
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);

    ccc::inplace_vector<int, 5> b;
    b.push_back(4);
    b.push_back(5);

    a.swap(b);

    CHECK(2u == a.size());
    CHECK(4 == a[0]);
    CHECK(5 == a[1]);

    CHECK(3u == b.size());
    CHECK(1 == b[0]);
    CHECK(2 == b[1]);
    CHECK(3 == b[2]);
}

TEST_CASE("InplaceVector - NonMemberSwap")
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    ccc::inplace_vector<int, 5> b;
    b.push_back(2);

    using std::swap;
    swap(a, b);

    CHECK(2 == a[0]);
    CHECK(1 == b[0]);
}

TEST_CASE("InplaceVector - Equality")
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    a.push_back(2);

    ccc::inplace_vector<int, 5> b;
    b.push_back(1);
    b.push_back(2);

    ccc::inplace_vector<int, 5> c;
    c.push_back(1);
    c.push_back(3);

    CHECK(a == b);
    CHECK_FALSE(a != b);
    CHECK_FALSE(a == c);
    CHECK(a != c);
}

TEST_CASE("InplaceVector - LessThan")
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    a.push_back(2);

    ccc::inplace_vector<int, 5> b;
    b.push_back(1);
    b.push_back(3);

    ccc::inplace_vector<int, 5> c;
    c.push_back(1);
    c.push_back(1);

    CHECK(a < b);
    CHECK_FALSE(b < a);
    CHECK(c < a);
    CHECK_FALSE(a < a);

    CHECK(a <= b);
    CHECK(a <= a);
    CHECK_FALSE(b <= a);

    CHECK(b > a);
    CHECK_FALSE(a > b);

    CHECK(b >= a);
    CHECK(a >= a);
}

TEST_CASE("InplaceVector - NonMemberErase")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(2);
    v.push_back(4);

    auto r = ccc::erase(v, 2);

    CHECK(2u == r);
    CHECK(3u == v.size());
    CHECK(1 == v[0]);
    CHECK(3 == v[1]);
    CHECK(4 == v[2]);
}

TEST_CASE("InplaceVector - NonMemberEraseIf")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    v.push_back(5);

    auto r = ccc::erase_if(v, [](int x) { return x % 2 == 0; });

    CHECK(2u == r);
    CHECK(3u == v.size());
    CHECK(1 == v[0]);
    CHECK(3 == v[1]);
    CHECK(5 == v[2]);
}

TEST_CASE("InplaceVector - StdAlgorithmCompatibility")
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(3);
    v.push_back(1);
    v.push_back(2);

    std::sort(v.begin(), v.end());

    CHECK(1 == v[0]);
    CHECK(2 == v[1]);
    CHECK(3 == v[2]);
}

TEST_CASE("InplaceVector - ConstexprStaticMembers")
{
    constexpr std::size_t cap = ccc::inplace_vector<int, 5>::capacity();
    constexpr std::size_t ms = ccc::inplace_vector<int, 5>::max_size();
    CHECK(5u == cap);
    CHECK(5u == ms);
}

// ====================================================================
// Compile-time tests (consteval + static_assert)
// ====================================================================

#if (__cpp_constexpr >= 202406L)

// Static member functions are always constexpr
static_assert(ccc::inplace_vector<int, 5>::capacity() == 5);
static_assert(ccc::inplace_vector<int, 5>::max_size() == 5);

// Default construction and basic state queries
consteval bool test_constexpr_default_construct()
{
    ccc::inplace_vector<int, 5> v;
    return v.empty() && !v.full() && v.size() == 0 && v.begin() == v.end();
}
static_assert(test_constexpr_default_construct());

// Reserve/shrink_to_fit with valid sizes
consteval bool test_constexpr_reserve_valid()
{
    ccc::inplace_vector<int, 5> v;
    v.reserve(3);
    v.reserve(5);
    v.shrink_to_fit();
    return true;
}
static_assert(test_constexpr_reserve_valid());

// Move construction of default-constructed vector
consteval bool test_constexpr_move_default()
{
    ccc::inplace_vector<int, 5> v;
    ccc::inplace_vector<int, 5> v2(std::move(v));
    return v2.empty() && v2.size() == 0;
}
static_assert(test_constexpr_move_default());

// Copy construction of default-constructed vector
consteval bool test_constexpr_copy_default()
{
    ccc::inplace_vector<int, 5> v;
    ccc::inplace_vector<int, 5> v2(v);
    return v2.empty() && v2.size() == 0;
}
static_assert(test_constexpr_copy_default());

// Clear on default-constructed vector (no-op)
consteval bool test_constexpr_clear_default()
{
    ccc::inplace_vector<int, 5> v;
    v.clear();
    return v.empty();
}
static_assert(test_constexpr_clear_default());

// Equality of default-constructed vectors
consteval bool test_constexpr_equality_default()
{
    ccc::inplace_vector<int, 5> a;
    ccc::inplace_vector<int, 5> b;
    return a == b;
}
static_assert(test_constexpr_equality_default());

// Swap of default-constructed vectors
consteval bool test_constexpr_swap_default()
{
    ccc::inplace_vector<int, 5> a;
    ccc::inplace_vector<int, 5> b;
    a.swap(b);
    return a.empty() && b.empty();
}
static_assert(test_constexpr_swap_default());

// consteval bool test_constexpr_swap()
//{
//     ccc::inplace_vector<int, 5> a{1, 2, 3};
//     ccc::inplace_vector<int, 5> b{9, 8};
//     a.swap(b);
//     return (b == ccc::inplace_vector<int, 5>{1, 2, 3}) && (a == ccc::inplace_vector<int, 5>{9, 8});
// }
// static_assert(test_constexpr_swap());

#endif  // (__cpp_constexpr >= 202406L)
