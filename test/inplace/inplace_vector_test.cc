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

TEST(InplaceVector, DefaultConstruct)
{
    ccc::inplace_vector<int, 5> v;

    EXPECT_TRUE(v.empty());
    EXPECT_FALSE(v.full());
    EXPECT_EQ(0u, v.size());
    EXPECT_EQ(5u, v.capacity());
    EXPECT_EQ(5u, v.max_size());
}

TEST(InplaceVector, PushBack)
{
    ccc::inplace_vector<int, 5> v;

    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);
    EXPECT_EQ(1, v.front());
    EXPECT_EQ(3, v.back());
}

TEST(InplaceVector, PushBackRvalue)
{
    ccc::inplace_vector<std::string, 3> v;
    std::string s = "hello";

    v.push_back(std::move(s));

    EXPECT_EQ("hello", v[0]);
}

TEST(InplaceVector, TryPushBack)
{
    ccc::inplace_vector<int, 3> v;

    EXPECT_NE(nullptr, v.try_push_back(1));
    EXPECT_NE(nullptr, v.try_push_back(2));
    EXPECT_NE(nullptr, v.try_push_back(3));
    EXPECT_EQ(nullptr, v.try_push_back(4));

    EXPECT_EQ(3u, v.size());
}

TEST(InplaceVector, UncheckedPushBack)
{
    ccc::inplace_vector<int, 3> v;

    auto p = v.unchecked_push_back(10);
    EXPECT_EQ(10, *p);

    v.unchecked_push_back(20);
    v.unchecked_push_back(30);

    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(10, v[0]);
}

TEST(InplaceVector, PushBackThrowsWhenFull)
{
    ccc::inplace_vector<int, 2> v;
    v.push_back(1);
    v.push_back(2);

    EXPECT_THROW(v.push_back(3), std::bad_alloc);
}

TEST(InplaceVector, PopBack)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    v.pop_back();
    EXPECT_EQ(2u, v.size());
    EXPECT_EQ(2, v.back());

    v.pop_back();
    EXPECT_EQ(1u, v.size());
    EXPECT_EQ(1, v.back());

    v.pop_back();
    EXPECT_TRUE(v.empty());
}

TEST(InplaceVector, EmplaceBack)
{
    ccc::inplace_vector<std::string, 5> v;

    v.emplace_back(3, 'a');

    EXPECT_EQ("aaa", v.back());
    EXPECT_EQ(1u, v.size());
}

TEST(InplaceVector, TryEmplaceBack)
{
    ccc::inplace_vector<int, 2> v;

    EXPECT_NE(nullptr, v.try_emplace_back(1));
    EXPECT_NE(nullptr, v.try_emplace_back(2));
    EXPECT_EQ(nullptr, v.try_emplace_back(3));

    EXPECT_EQ(2u, v.size());
}

TEST(InplaceVector, UncheckedEmplaceBack)
{
    ccc::inplace_vector<std::string, 5> v;

    auto p = v.unchecked_emplace_back(2, 'x');
    EXPECT_EQ("xx", *p);
    EXPECT_EQ(1u, v.size());
}

TEST(InplaceVector, CopyConstruct)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    ccc::inplace_vector<int, 5> v2(v);

    EXPECT_EQ(3u, v2.size());
    EXPECT_EQ(10, v2[0]);
    EXPECT_EQ(20, v2[1]);
    EXPECT_EQ(30, v2[2]);
    EXPECT_EQ(3u, v.size());
}

TEST(InplaceVector, MoveConstruct)
{
    ccc::inplace_vector<std::string, 5> v;
    v.push_back("hello");
    v.push_back("world");

    ccc::inplace_vector<std::string, 5> v2(std::move(v));

    EXPECT_EQ(2u, v2.size());
    EXPECT_EQ("hello", v2[0]);
    EXPECT_EQ("world", v2[1]);
}

TEST(InplaceVector, InitializerListConstruct)
{
    ccc::inplace_vector<int, 5> v = {10, 20, 30};

    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(10, v[0]);
    EXPECT_EQ(20, v[1]);
    EXPECT_EQ(30, v[2]);
}

TEST(InplaceVector, InitializerListConstructThrowsWhenTooLarge)
{
    EXPECT_THROW((ccc::inplace_vector<int, 2>{1, 2, 3}), std::bad_alloc);
}

TEST(InplaceVector, DestructorCallsElementDestructors)
{
    Counter::reset();
    {
        ccc::inplace_vector<Counter, 5> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);
    }
    EXPECT_EQ(Counter::constructed, Counter::destructed);
}

TEST(InplaceVector, CopyAssignment)
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    a.push_back(2);

    ccc::inplace_vector<int, 5> b;
    b.push_back(9);

    b = a;

    EXPECT_EQ(2u, b.size());
    EXPECT_EQ(1, b[0]);
    EXPECT_EQ(2, b[1]);
    EXPECT_EQ(2u, a.size());
}

TEST(InplaceVector, MoveAssignment)
{
    ccc::inplace_vector<std::string, 5> a;
    a.push_back("x");
    a.push_back("y");

    ccc::inplace_vector<std::string, 5> b;
    b.push_back("z");

    b = std::move(a);

    EXPECT_EQ(2u, b.size());
    EXPECT_EQ("x", b[0]);
    EXPECT_EQ("y", b[1]);
}

TEST(InplaceVector, InitializerListAssignment)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);

    v = {10, 20, 30};

    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(10, v[0]);
}

TEST(InplaceVector, SelfAssignment)
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

    EXPECT_EQ(2u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
}

TEST(InplaceVector, AssignCount)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);

    v.assign(5, 42);

    EXPECT_EQ(5u, v.size());
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(42, v[4]);
}

TEST(InplaceVector, AssignRange)
{
    std::vector<int> src = {3, 5, 7, 9};
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.assign(src.begin(), src.end());

    EXPECT_EQ(4u, v.size());
    EXPECT_EQ(3, v[0]);
    EXPECT_EQ(9, v[3]);
}

TEST(InplaceVector, AssignInitializerList)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.assign({4, 5, 6});

    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(4, v[0]);
}

TEST(InplaceVector, AssignRangeFreeFunc)
{
    int arr[] = {7, 8, 9};
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.assign_range(arr);

    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(7, v[0]);
}

TEST(InplaceVector, AssignThrowsWhenTooLarge)
{
    ccc::inplace_vector<int, 3> v;
    EXPECT_THROW(v.assign(5, 0), std::bad_alloc);
}

TEST(InplaceVector, InsertSingleCopy)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(5);

    auto it = v.insert(v.begin() + 1, 3);

    EXPECT_EQ(3, *it);
    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(3, v[1]);
    EXPECT_EQ(5, v[2]);
}

TEST(InplaceVector, InsertSingleMove)
{
    ccc::inplace_vector<std::string, 10> v;
    v.push_back("a");
    v.push_back("c");

    std::string s = "b";
    v.insert(v.begin() + 1, std::move(s));

    EXPECT_EQ(3u, v.size());
    EXPECT_EQ("a", v[0]);
    EXPECT_EQ("b", v[1]);
    EXPECT_EQ("c", v[2]);
}

TEST(InplaceVector, InsertAtEnd)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    auto it = v.insert(v.end(), 2);

    EXPECT_EQ(2, *it);
    EXPECT_EQ(2u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
}

TEST(InplaceVector, InsertAtBegin)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(2);
    v.push_back(3);

    auto it = v.insert(v.begin(), 1);

    EXPECT_EQ(1, *it);
    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);
}

TEST(InplaceVector, InsertCount)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    auto it = v.insert(v.begin() + 1, 3, 5);

    EXPECT_EQ(5, *it);
    EXPECT_EQ(5u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(5, v[1]);
    EXPECT_EQ(5, v[2]);
    EXPECT_EQ(5, v[3]);
    EXPECT_EQ(9, v[4]);
}

TEST(InplaceVector, InsertCountZero)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    auto it = v.insert(v.begin(), 0, 99);

    EXPECT_EQ(1, *it);
    EXPECT_EQ(1u, v.size());
}

TEST(InplaceVector, InsertRange)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    int arr[] = {3, 5, 7};
    auto it = v.insert(v.begin() + 1, arr, arr + 3);

    EXPECT_EQ(3, *it);
    EXPECT_EQ(5u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(3, v[1]);
    EXPECT_EQ(5, v[2]);
    EXPECT_EQ(7, v[3]);
    EXPECT_EQ(9, v[4]);
}

TEST(InplaceVector, InsertInitializerList)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    v.insert(v.begin() + 1, {3, 5, 7});

    EXPECT_EQ(5u, v.size());
    EXPECT_EQ(3, v[1]);
    EXPECT_EQ(7, v[3]);
}

TEST(InplaceVector, InsertThrowsWhenFull)
{
    ccc::inplace_vector<int, 2> v;
    v.push_back(1);
    v.push_back(2);

    EXPECT_THROW(v.insert(v.begin(), 0), std::bad_alloc);
}

TEST(InplaceVector, Emplace)
{
    ccc::inplace_vector<std::string, 10> v;
    v.push_back("a");
    v.push_back("c");

    auto it = v.emplace(v.begin() + 1, 1, 'b');

    EXPECT_EQ("b", *it);
    EXPECT_EQ(3u, v.size());
    EXPECT_EQ("a", v[0]);
    EXPECT_EQ("b", v[1]);
    EXPECT_EQ("c", v[2]);
}

TEST(InplaceVector, InsertRangeFreeFunc)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(9);

    int arr[] = {3, 5};
    v.insert_range(v.begin() + 1, arr);

    EXPECT_EQ(4u, v.size());
    EXPECT_EQ(3, v[1]);
    EXPECT_EQ(5, v[2]);
}

TEST(InplaceVector, AppendRange)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    int arr[] = {2, 3, 4};
    v.append_range(arr);

    EXPECT_EQ(4u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(4, v[3]);
}

TEST(InplaceVector, TryAppendRange)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);

    int arr[] = {3, 4, 5};
    EXPECT_TRUE(v.try_append_range(arr));
    EXPECT_EQ(5u, v.size());

    int arr2[] = {6};
    EXPECT_FALSE(v.try_append_range(arr2));
    EXPECT_EQ(5u, v.size());
}

TEST(InplaceVector, EraseSingle)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);

    auto it = v.erase(v.begin() + 1);

    EXPECT_EQ(3, *it);
    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(3, v[1]);
    EXPECT_EQ(4, v[2]);
}

TEST(InplaceVector, EraseRange)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);

    auto it = v.erase(v.begin() + 1, v.begin() + 3);

    EXPECT_EQ(4, *it);
    EXPECT_EQ(2u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(4, v[1]);
}

TEST(InplaceVector, EraseToEnd)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    auto it = v.erase(v.begin() + 1, v.end());

    EXPECT_EQ(v.end(), it);
    EXPECT_EQ(1u, v.size());
    EXPECT_EQ(1, v[0]);
}

TEST(InplaceVector, Clear)
{
    Counter::reset();
    {
        ccc::inplace_vector<Counter, 5> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.clear();

        EXPECT_TRUE(v.empty());
        EXPECT_EQ(0u, v.size());
    }
    EXPECT_EQ(Counter::constructed, Counter::destructed);
}

TEST(InplaceVector, ClearThenReuse)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.clear();

    EXPECT_TRUE(v.empty());

    v.push_back(3);

    EXPECT_EQ(1u, v.size());
    EXPECT_EQ(3, v[0]);
}

TEST(InplaceVector, ResizeDefaultLarger)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);

    v.resize(5);

    EXPECT_EQ(5u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
}

TEST(InplaceVector, ResizeDefaultSmaller)
{
    Counter::reset();
    {
        ccc::inplace_vector<Counter, 10> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);

        v.resize(1);

        EXPECT_EQ(1u, v.size());
        EXPECT_EQ(1, v[0].value);
    }
    EXPECT_EQ(Counter::constructed, Counter::destructed);
}

TEST(InplaceVector, ResizeWithValueLarger)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);

    v.resize(4, 7);

    EXPECT_EQ(4u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(7, v[1]);
    EXPECT_EQ(7, v[3]);
}

TEST(InplaceVector, ResizeThrowsWhenTooLarge)
{
    ccc::inplace_vector<int, 3> v;
    EXPECT_THROW(v.resize(10), std::out_of_range);
}

TEST(InplaceVector, Reserve)
{
    ccc::inplace_vector<int, 5> v;
    EXPECT_NO_THROW(v.reserve(3));
    EXPECT_NO_THROW(v.reserve(5));
    EXPECT_THROW(v.reserve(6), std::bad_alloc);
}

TEST(InplaceVector, ShrinkToFit)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    EXPECT_NO_THROW(v.shrink_to_fit());
    EXPECT_EQ(1u, v.size());
}

TEST(InplaceVector, At)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(10);
    v.push_back(20);

    EXPECT_EQ(10, v.at(0));
    EXPECT_EQ(20, v.at(1));
    EXPECT_THROW(v.at(2), std::out_of_range);
}

TEST(InplaceVector, Data)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);

    int* p = v.data();
    EXPECT_EQ(1, p[0]);
    EXPECT_EQ(2, p[1]);

    const auto& cv = v;
    const int* cp = cv.data();
    EXPECT_EQ(1, cp[0]);
}

TEST(InplaceVector, Iterators)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    int sum = 0;
    for (auto it = v.begin(); it != v.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(6, sum);

    sum = 0;
    for (auto it = v.cbegin(); it != v.cend(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(6, sum);
}

TEST(InplaceVector, ReverseIterators)
{
    ccc::inplace_vector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    int sum = 0;
    for (auto it = v.rbegin(); it != v.rend(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(6, sum);

    sum = 0;
    for (auto it = v.crbegin(); it != v.crend(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(6, sum);
}

TEST(InplaceVector, Swap)
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);

    ccc::inplace_vector<int, 5> b;
    b.push_back(4);
    b.push_back(5);

    a.swap(b);

    EXPECT_EQ(2u, a.size());
    EXPECT_EQ(4, a[0]);
    EXPECT_EQ(5, a[1]);

    EXPECT_EQ(3u, b.size());
    EXPECT_EQ(1, b[0]);
    EXPECT_EQ(2, b[1]);
    EXPECT_EQ(3, b[2]);
}

TEST(InplaceVector, NonMemberSwap)
{
    ccc::inplace_vector<int, 5> a;
    a.push_back(1);
    ccc::inplace_vector<int, 5> b;
    b.push_back(2);

    using std::swap;
    swap(a, b);

    EXPECT_EQ(2, a[0]);
    EXPECT_EQ(1, b[0]);
}

TEST(InplaceVector, Equality)
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

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(InplaceVector, LessThan)
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

    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
    EXPECT_TRUE(c < a);
    EXPECT_FALSE(a < a);

    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a <= a);
    EXPECT_FALSE(b <= a);

    EXPECT_TRUE(b > a);
    EXPECT_FALSE(a > b);

    EXPECT_TRUE(b >= a);
    EXPECT_TRUE(a >= a);
}

TEST(InplaceVector, NonMemberErase)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(2);
    v.push_back(4);

    auto r = ccc::erase(v, 2);

    EXPECT_EQ(2u, r);
    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(3, v[1]);
    EXPECT_EQ(4, v[2]);
}

TEST(InplaceVector, NonMemberEraseIf)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    v.push_back(5);

    auto r = ccc::erase_if(v, [](int x) { return x % 2 == 0; });

    EXPECT_EQ(2u, r);
    EXPECT_EQ(3u, v.size());
    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(3, v[1]);
    EXPECT_EQ(5, v[2]);
}

TEST(InplaceVector, StdAlgorithmCompatibility)
{
    ccc::inplace_vector<int, 10> v;
    v.push_back(3);
    v.push_back(1);
    v.push_back(2);

    std::sort(v.begin(), v.end());

    EXPECT_EQ(1, v[0]);
    EXPECT_EQ(2, v[1]);
    EXPECT_EQ(3, v[2]);
}

TEST(InplaceVector, ConstexprStaticMembers)
{
    constexpr std::size_t cap = ccc::inplace_vector<int, 5>::capacity();
    constexpr std::size_t ms = ccc::inplace_vector<int, 5>::max_size();
    EXPECT_EQ(5u, cap);
    EXPECT_EQ(5u, ms);
}