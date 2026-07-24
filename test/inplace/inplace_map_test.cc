#ifndef TEST_USE_MODULE
#include "ccc/inplace/inplace_map.hh"
#endif

#include "test_pch.hh"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef TEST_USE_MODULE
import ccc.inplace.inplace_map;
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

TEST_CASE("InplaceMap - DefaultConstruct")
{
    ccc::inplace_map<int, std::string, 5> m;
    CHECK(m.empty());
    CHECK_FALSE(m.full());
    CHECK(0u == m.size());
    CHECK(5u == m.capacity());
    CHECK(5u == m.max_size());
}

TEST_CASE("InplaceMap - InitializerListConstruct")
{
    ccc::inplace_map<int, std::string, 5> m = {{1, "one"}, {3, "three"}, {2, "two"}};
    CHECK(3u == m.size());
    CHECK(1 == m.begin()->first);

    CHECK("one" == m.at(1));
    CHECK("two" == m.at(2));
    CHECK("three" == m.at(3));
}

TEST_CASE("InplaceMap - CopyConstruct")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    ccc::inplace_map<int, std::string, 5> m2(m);
    CHECK(2u == m2.size());
    CHECK("one" == m2[1]);
    CHECK("two" == m2[2]);
}

TEST_CASE("InplaceMap - MoveConstruct")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "hello";
    m[2] = "world";
    auto m2(std::move(m));
    CHECK(2u == m2.size());
    CHECK("hello" == m2[1]);
    CHECK("world" == m2[2]);
}

TEST_CASE("InplaceMap - CopyAssignment")
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "one";
    a[2] = "two";
    b[9] = "nine";
    b = a;
    CHECK(2u == b.size());
    CHECK("one" == b[1]);
}

TEST_CASE("InplaceMap - MoveAssignment")
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "x";
    a[2] = "y";
    b[9] = "z";
    b = std::move(a);
    CHECK(2u == b.size());
    CHECK("x" == b[1]);
}

TEST_CASE("InplaceMap - SelfAssignment")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    m = m;
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    CHECK(1u == m.size());
    CHECK("one" == m[1]);
}

TEST_CASE("InplaceMap - At")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    CHECK("one" == m.at(1));
    CHECK("two" == m.at(2));
    CHECK_THROWS_AS(m.at(3), std::out_of_range);
}

TEST_CASE("InplaceMap - AtConst")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    const auto& cm = m;
    CHECK("one" == cm.at(1));
    CHECK_THROWS_AS(cm.at(99), std::out_of_range);
}

TEST_CASE("InplaceMap - SubscriptOperatorExisting")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    CHECK("one" == m[1]);
    CHECK("two" == m[2]);
}

TEST_CASE("InplaceMap - SubscriptOperatorInsertDefault")
{
    ccc::inplace_map<int, std::string, 5> m;
    CHECK("" == m[42]);
    CHECK(1u == m.size());
    m[42] = "answer";
    CHECK("answer" == m[42]);
}

TEST_CASE("InplaceMap - SubscriptOperatorInsertLots")
{
    ccc::inplace_map<int, std::string, 10> m;
    for (int i = 0; i < 10; ++i) m[i] = std::to_string(i);
    CHECK(m.full());
    CHECK(10u == m.size());
    for (int i = 0; i < 10; ++i) CHECK(std::to_string(i) == m[i]);
}

TEST_CASE("InplaceMap - IteratorForward")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[1] = "one";
    m[3] = "three";
    int e = 1;
    for (auto it = m.begin(); it != m.end(); ++it) {
        CHECK(e == it->first);
        ++e;
    }
    CHECK(4 == e);
}

TEST_CASE("InplaceMap - IteratorProxyAccess")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    auto it = m.begin();
    CHECK(1 == it->first);
    CHECK("one" == it->second);
    CHECK(1 == (*it).first);
    CHECK("one" == (*it).second);
}

TEST_CASE("InplaceMap - ReverseIterator")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    int e = 3;
    for (auto it = m.rbegin(); it != m.rend(); ++it) {
        CHECK(e == it->first);
        --e;
    }
}

TEST_CASE("InplaceMap - ConstIterator")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    const auto& cm = m;
    auto it = cm.begin();
    CHECK(1 == it->first);
    CHECK("one" == it->second);
}

TEST_CASE("InplaceMap - IteratorConversion")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    ccc::inplace_map<int, std::string, 5>::const_iterator it = m.begin();
    CHECK(it == m.begin());
}

TEST_CASE("InplaceMap - StaticCapacity")
{
    constexpr auto cap = ccc::inplace_map<int, char, 8>::capacity();
    constexpr auto ms = ccc::inplace_map<int, char, 8>::max_size();
    CHECK(8u == cap);
    CHECK(8u == ms);
}

TEST_CASE("InplaceMap - Full")
{
    ccc::inplace_map<int, int, 3> m;
    CHECK_FALSE(m.full());
    m[1] = 10;
    m[2] = 20;
    m[3] = 30;
    CHECK(m.full());
    CHECK(3u == m.size());
}

TEST_CASE("InplaceMap - InsertNew")
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.insert({1, "one"});
    CHECK(r.second);
    CHECK(1 == r.first->first);
    CHECK(1u == m.size());
}

TEST_CASE("InplaceMap - InsertDuplicate")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "original";
    auto r = m.insert({1, "overwrite"});
    CHECK_FALSE(r.second);
    CHECK("original" == m[1]);
}

TEST_CASE("InplaceMap - InsertMultiple")
{
    ccc::inplace_map<int, std::string, 10> m;
    m.insert({{3, "three"}, {1, "one"}, {4, "four"}, {2, "two"}});
    CHECK(4u == m.size());
    CHECK(1 == m.begin()->first);
}

TEST_CASE("InplaceMap - InsertWithHint")
{
    ccc::inplace_map<int, std::string, 10> m;
    m[1] = "one";
    m[3] = "three";
    auto it = m.insert(m.end(), {2, "two"});
    CHECK(2 == it->first);
    CHECK(3u == m.size());
}

TEST_CASE("InplaceMap - InsertOrAssignNew")
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.insert_or_assign(1, "hello");
    CHECK(r.second);
    CHECK("hello" == m[1]);
}

TEST_CASE("InplaceMap - InsertOrAssignExisting")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "old";
    auto r = m.insert_or_assign(1, "new");
    CHECK_FALSE(r.second);
    CHECK("new" == m[1]);
}

TEST_CASE("InplaceMap - Emplace")
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.emplace(1, "hello");
    CHECK(r.second);
    CHECK("hello" == m[1]);
}

TEST_CASE("InplaceMap - EmplaceDuplicate")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "original";
    auto r = m.emplace(1, "new");
    CHECK_FALSE(r.second);
    CHECK("original" == m[1]);
}

TEST_CASE("InplaceMap - TryEmplaceNew")
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.try_emplace(1, "hello");
    CHECK(r.second);
    CHECK("hello" == m[1]);
}

TEST_CASE("InplaceMap - TryEmplaceDuplicate")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "original";
    auto r = m.try_emplace(1, "new");
    CHECK_FALSE(r.second);
    CHECK("original" == m[1]);
}

TEST_CASE("InplaceMap - EraseByKey")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    CHECK(1u == m.erase(2));
    CHECK(2u == m.size());
    CHECK_FALSE(m.contains(2));
}

TEST_CASE("InplaceMap - EraseByKeyNotFound")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    CHECK(0u == m.erase(99));
    CHECK(1u == m.size());
}

TEST_CASE("InplaceMap - EraseByIterator")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    auto it = m.erase(m.cbegin() + 1);
    CHECK(3 == it->first);
    CHECK(2u == m.size());
}

TEST_CASE("InplaceMap - EraseRange")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    m[4] = "four";
    auto it = m.erase(m.cbegin() + 1, m.cbegin() + 3);
    CHECK(4 == it->first);
    CHECK(2u == m.size());
}

TEST_CASE("InplaceMap - Clear")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m.clear();
    CHECK(m.empty());
}

TEST_CASE("InplaceMap - ClearThenReuse")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m.clear();
    m[3] = "three";
    CHECK(1u == m.size());
    CHECK("three" == m[3]);
}

TEST_CASE("InplaceMap - InsertThrowsWhenFull")
{
    ccc::inplace_map<int, std::string, 2> m;
    m[1] = "one";
    m[2] = "two";
    CHECK_THROWS_AS(m.insert({3, "three"}), std::bad_alloc);
}

TEST_CASE("InplaceMap - SubscriptThrowsWhenFull")
{
    ccc::inplace_map<int, std::string, 2> m;
    m[1] = "one";
    m[2] = "two";
    CHECK_THROWS_AS(m[3] = "three", std::bad_alloc);
}

TEST_CASE("InplaceMap - Find")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    CHECK(m.end() != m.find(1));
    CHECK("one" == m.find(1)->second);
    CHECK(m.end() == m.find(99));
}

TEST_CASE("InplaceMap - Contains")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    CHECK(m.contains(1));
    CHECK_FALSE(m.contains(99));
}

TEST_CASE("InplaceMap - Count")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    CHECK(1u == m.count(1));
    CHECK(0u == m.count(99));
}

TEST_CASE("InplaceMap - LowerBound")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[4] = "four";
    m[6] = "six";
    CHECK(2 == m.lower_bound(1)->first);
    CHECK(m.end() == m.lower_bound(7));
}

TEST_CASE("InplaceMap - UpperBound")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[4] = "four";
    m[6] = "six";
    CHECK(2 == m.upper_bound(1)->first);
    CHECK(4 == m.upper_bound(2)->first);
    CHECK(m.end() == m.upper_bound(6));
}

TEST_CASE("InplaceMap - EqualRange")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[4] = "four";
    m[6] = "six";
    auto r = m.equal_range(4);
    CHECK(4 == r.first->first);
    CHECK(6 == r.second->first);
}

TEST_CASE("InplaceMap - Equality")
{
    ccc::inplace_map<int, std::string, 5> a, b, c;
    a[1] = "one";
    a[2] = "two";
    b[1] = "one";
    b[2] = "two";
    c[1] = "one";
    c[3] = "three";
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

TEST_CASE("InplaceMap - LessThan")
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "alpha";
    a[2] = "beta";
    b[1] = "alpha";
    b[3] = "gamma";
    CHECK(a < b);
    CHECK_FALSE(b < a);
    CHECK(a <= a);
    CHECK(b > a);
}

TEST_CASE("InplaceMap - MemberSwap")
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "one";
    a[2] = "two";
    b[3] = "three";
    a.swap(b);
    CHECK(1u == a.size());
    CHECK("three" == a[3]);
    CHECK(2u == b.size());
}

TEST_CASE("InplaceMap - NonMemberSwap")
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "one";
    b[2] = "two";
    using std::swap;
    swap(a, b);
    CHECK("two" == a[2]);
    CHECK("one" == b[1]);
}

TEST_CASE("InplaceMap - DestructorCallsElementDestructors")
{
    Counter::reset();
    {
        ccc::inplace_map<int, Counter, 5> m;
        m.emplace(1, Counter(10));
        m.emplace(2, Counter(20));
    }
    CHECK(Counter::constructed == Counter::destructed);
}

TEST_CASE("InplaceMap - KeyComp")
{
    ccc::inplace_map<int, std::string, 5> m;
    auto c = m.key_comp();
    CHECK(c(1, 2));
    CHECK_FALSE(c(2, 1));
    CHECK_FALSE(c(1, 1));
}

TEST_CASE("InplaceMap - ValueComp")
{
    ccc::inplace_map<int, std::string, 5> m;
    auto c = m.value_comp();
    CHECK(c({1, "a"}, {2, "b"}));
    CHECK_FALSE(c({2, "b"}, {1, "a"}));
}

TEST_CASE("InplaceMap - KeysAndValuesAccess")
{
    ccc::inplace_map<int, std::string, 5> m;
    m[3] = "three";
    m[1] = "one";
    m[2] = "two";
    const auto& k = m.keys();
    const auto& v = m.values();
    CHECK(1 == k[0]);
    CHECK(2 == k[1]);
    CHECK(3 == k[2]);
    CHECK("one" == v[0]);
}

TEST_CASE("InplaceMap - RangeConstruct")
{
    std::vector<std::pair<int, std::string>> src = {{3, "three"}, {1, "one"}, {2, "two"}};
    ccc::inplace_map<int, std::string, 5> m(src.begin(), src.end());
    CHECK(3u == m.size());
    CHECK(1 == m.begin()->first);
}
