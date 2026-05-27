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

TEST(InplaceMap, DefaultConstruct)
{
    ccc::inplace_map<int, std::string, 5> m;
    EXPECT_TRUE(m.empty());
    EXPECT_FALSE(m.full());
    EXPECT_EQ(0u, m.size());
    EXPECT_EQ(5u, m.capacity());
    EXPECT_EQ(5u, m.max_size());
}

TEST(InplaceMap, InitializerListConstruct)
{
    ccc::inplace_map<int, std::string, 5> m = {{1, "one"}, {3, "three"}, {2, "two"}};
    EXPECT_EQ(3u, m.size());
    EXPECT_EQ(1, m.begin()->first);

    EXPECT_EQ("one", m.at(1));
    EXPECT_EQ("two", m.at(2));
    EXPECT_EQ("three", m.at(3));
}

TEST(InplaceMap, CopyConstruct)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    ccc::inplace_map<int, std::string, 5> m2(m);
    EXPECT_EQ(2u, m2.size());
    EXPECT_EQ("one", m2[1]);
    EXPECT_EQ("two", m2[2]);
}

TEST(InplaceMap, MoveConstruct)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "hello";
    m[2] = "world";
    auto m2(std::move(m));
    EXPECT_EQ(2u, m2.size());
    EXPECT_EQ("hello", m2[1]);
    EXPECT_EQ("world", m2[2]);
}

TEST(InplaceMap, CopyAssignment)
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "one";
    a[2] = "two";
    b[9] = "nine";
    b = a;
    EXPECT_EQ(2u, b.size());
    EXPECT_EQ("one", b[1]);
}

TEST(InplaceMap, MoveAssignment)
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "x";
    a[2] = "y";
    b[9] = "z";
    b = std::move(a);
    EXPECT_EQ(2u, b.size());
    EXPECT_EQ("x", b[1]);
}

TEST(InplaceMap, SelfAssignment)
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

    EXPECT_EQ(1u, m.size());
    EXPECT_EQ("one", m[1]);
}

TEST(InplaceMap, At)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    EXPECT_EQ("one", m.at(1));
    EXPECT_EQ("two", m.at(2));
    EXPECT_THROW(m.at(3), std::out_of_range);
}

TEST(InplaceMap, AtConst)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    const auto& cm = m;
    EXPECT_EQ("one", cm.at(1));
    EXPECT_THROW(cm.at(99), std::out_of_range);
}

TEST(InplaceMap, SubscriptOperatorExisting)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    EXPECT_EQ("one", m[1]);
    EXPECT_EQ("two", m[2]);
}

TEST(InplaceMap, SubscriptOperatorInsertDefault)
{
    ccc::inplace_map<int, std::string, 5> m;
    EXPECT_EQ("", m[42]);
    EXPECT_EQ(1u, m.size());
    m[42] = "answer";
    EXPECT_EQ("answer", m[42]);
}

TEST(InplaceMap, SubscriptOperatorInsertLots)
{
    ccc::inplace_map<int, std::string, 10> m;
    for (int i = 0; i < 10; ++i) m[i] = std::to_string(i);
    EXPECT_TRUE(m.full());
    EXPECT_EQ(10u, m.size());
    for (int i = 0; i < 10; ++i) EXPECT_EQ(std::to_string(i), m[i]);
}

TEST(InplaceMap, IteratorForward)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[1] = "one";
    m[3] = "three";
    int e = 1;
    for (auto it = m.begin(); it != m.end(); ++it) {
        EXPECT_EQ(e, it->first);
        ++e;
    }
    EXPECT_EQ(4, e);
}

TEST(InplaceMap, IteratorProxyAccess)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    auto it = m.begin();
    EXPECT_EQ(1, it->first);
    EXPECT_EQ("one", it->second);
    EXPECT_EQ(1, (*it).first);
    EXPECT_EQ("one", (*it).second);
}

TEST(InplaceMap, ReverseIterator)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    int e = 3;
    for (auto it = m.rbegin(); it != m.rend(); ++it) {
        EXPECT_EQ(e, it->first);
        --e;
    }
}

TEST(InplaceMap, ConstIterator)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    const auto& cm = m;
    auto it = cm.begin();
    EXPECT_EQ(1, it->first);
    EXPECT_EQ("one", it->second);
}

TEST(InplaceMap, IteratorConversion)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    ccc::inplace_map<int, std::string, 5>::const_iterator it = m.begin();
    EXPECT_TRUE(it == m.begin());
}

TEST(InplaceMap, StaticCapacity)
{
    constexpr auto cap = ccc::inplace_map<int, char, 8>::capacity();
    constexpr auto ms = ccc::inplace_map<int, char, 8>::max_size();
    EXPECT_EQ(8u, cap);
    EXPECT_EQ(8u, ms);
}

TEST(InplaceMap, Full)
{
    ccc::inplace_map<int, int, 3> m;
    EXPECT_FALSE(m.full());
    m[1] = 10;
    m[2] = 20;
    m[3] = 30;
    EXPECT_TRUE(m.full());
    EXPECT_EQ(3u, m.size());
}

TEST(InplaceMap, InsertNew)
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.insert({1, "one"});
    EXPECT_TRUE(r.second);
    EXPECT_EQ(1, r.first->first);
    EXPECT_EQ(1u, m.size());
}

TEST(InplaceMap, InsertDuplicate)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "original";
    auto r = m.insert({1, "overwrite"});
    EXPECT_FALSE(r.second);
    EXPECT_EQ("original", m[1]);
}

TEST(InplaceMap, InsertMultiple)
{
    ccc::inplace_map<int, std::string, 10> m;
    m.insert({{3, "three"}, {1, "one"}, {4, "four"}, {2, "two"}});
    EXPECT_EQ(4u, m.size());
    EXPECT_EQ(1, m.begin()->first);
}

TEST(InplaceMap, InsertWithHint)
{
    ccc::inplace_map<int, std::string, 10> m;
    m[1] = "one";
    m[3] = "three";
    auto it = m.insert(m.end(), {2, "two"});
    EXPECT_EQ(2, it->first);
    EXPECT_EQ(3u, m.size());
}

TEST(InplaceMap, InsertOrAssignNew)
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.insert_or_assign(1, "hello");
    EXPECT_TRUE(r.second);
    EXPECT_EQ("hello", m[1]);
}

TEST(InplaceMap, InsertOrAssignExisting)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "old";
    auto r = m.insert_or_assign(1, "new");
    EXPECT_FALSE(r.second);
    EXPECT_EQ("new", m[1]);
}

TEST(InplaceMap, Emplace)
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.emplace(1, "hello");
    EXPECT_TRUE(r.second);
    EXPECT_EQ("hello", m[1]);
}

TEST(InplaceMap, EmplaceDuplicate)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "original";
    auto r = m.emplace(1, "new");
    EXPECT_FALSE(r.second);
    EXPECT_EQ("original", m[1]);
}

TEST(InplaceMap, TryEmplaceNew)
{
    ccc::inplace_map<int, std::string, 5> m;
    auto r = m.try_emplace(1, "hello");
    EXPECT_TRUE(r.second);
    EXPECT_EQ("hello", m[1]);
}

TEST(InplaceMap, TryEmplaceDuplicate)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "original";
    auto r = m.try_emplace(1, "new");
    EXPECT_FALSE(r.second);
    EXPECT_EQ("original", m[1]);
}

TEST(InplaceMap, EraseByKey)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    EXPECT_EQ(1u, m.erase(2));
    EXPECT_EQ(2u, m.size());
    EXPECT_FALSE(m.contains(2));
}

TEST(InplaceMap, EraseByKeyNotFound)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    EXPECT_EQ(0u, m.erase(99));
    EXPECT_EQ(1u, m.size());
}

TEST(InplaceMap, EraseByIterator)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    auto it = m.erase(m.cbegin() + 1);
    EXPECT_EQ(3, it->first);
    EXPECT_EQ(2u, m.size());
}

TEST(InplaceMap, EraseRange)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m[3] = "three";
    m[4] = "four";
    auto it = m.erase(m.cbegin() + 1, m.cbegin() + 3);
    EXPECT_EQ(4, it->first);
    EXPECT_EQ(2u, m.size());
}

TEST(InplaceMap, Clear)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m.clear();
    EXPECT_TRUE(m.empty());
}

TEST(InplaceMap, ClearThenReuse)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    m.clear();
    m[3] = "three";
    EXPECT_EQ(1u, m.size());
    EXPECT_EQ("three", m[3]);
}

TEST(InplaceMap, InsertThrowsWhenFull)
{
    ccc::inplace_map<int, std::string, 2> m;
    m[1] = "one";
    m[2] = "two";
    EXPECT_THROW(m.insert({3, "three"}), std::bad_alloc);
}

TEST(InplaceMap, SubscriptThrowsWhenFull)
{
    ccc::inplace_map<int, std::string, 2> m;
    m[1] = "one";
    m[2] = "two";
    EXPECT_THROW(m[3] = "three", std::bad_alloc);
}

TEST(InplaceMap, Find)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    m[2] = "two";
    EXPECT_NE(m.end(), m.find(1));
    EXPECT_EQ("one", m.find(1)->second);
    EXPECT_EQ(m.end(), m.find(99));
}

TEST(InplaceMap, Contains)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    EXPECT_TRUE(m.contains(1));
    EXPECT_FALSE(m.contains(99));
}

TEST(InplaceMap, Count)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[1] = "one";
    EXPECT_EQ(1u, m.count(1));
    EXPECT_EQ(0u, m.count(99));
}

TEST(InplaceMap, LowerBound)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[4] = "four";
    m[6] = "six";
    EXPECT_EQ(2, m.lower_bound(1)->first);
    EXPECT_EQ(m.end(), m.lower_bound(7));
}

TEST(InplaceMap, UpperBound)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[4] = "four";
    m[6] = "six";
    EXPECT_EQ(2, m.upper_bound(1)->first);
    EXPECT_EQ(4, m.upper_bound(2)->first);
    EXPECT_EQ(m.end(), m.upper_bound(6));
}

TEST(InplaceMap, EqualRange)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[2] = "two";
    m[4] = "four";
    m[6] = "six";
    auto r = m.equal_range(4);
    EXPECT_EQ(4, r.first->first);
    EXPECT_EQ(6, r.second->first);
}

TEST(InplaceMap, Equality)
{
    ccc::inplace_map<int, std::string, 5> a, b, c;
    a[1] = "one";
    a[2] = "two";
    b[1] = "one";
    b[2] = "two";
    c[1] = "one";
    c[3] = "three";
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(InplaceMap, LessThan)
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "alpha";
    a[2] = "beta";
    b[1] = "alpha";
    b[3] = "gamma";
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(b > a);
}

TEST(InplaceMap, MemberSwap)
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "one";
    a[2] = "two";
    b[3] = "three";
    a.swap(b);
    EXPECT_EQ(1u, a.size());
    EXPECT_EQ("three", a[3]);
    EXPECT_EQ(2u, b.size());
}

TEST(InplaceMap, NonMemberSwap)
{
    ccc::inplace_map<int, std::string, 5> a, b;
    a[1] = "one";
    b[2] = "two";
    using std::swap;
    swap(a, b);
    EXPECT_EQ("two", a[2]);
    EXPECT_EQ("one", b[1]);
}

TEST(InplaceMap, DestructorCallsElementDestructors)
{
    Counter::reset();
    {
        ccc::inplace_map<int, Counter, 5> m;
        m.emplace(1, Counter(10));
        m.emplace(2, Counter(20));
    }
    EXPECT_EQ(Counter::constructed, Counter::destructed);
}

TEST(InplaceMap, KeyComp)
{
    ccc::inplace_map<int, std::string, 5> m;
    auto c = m.key_comp();
    EXPECT_TRUE(c(1, 2));
    EXPECT_FALSE(c(2, 1));
    EXPECT_FALSE(c(1, 1));
}

TEST(InplaceMap, ValueComp)
{
    ccc::inplace_map<int, std::string, 5> m;
    auto c = m.value_comp();
    EXPECT_TRUE(c({1, "a"}, {2, "b"}));
    EXPECT_FALSE(c({2, "b"}, {1, "a"}));
}

TEST(InplaceMap, KeysAndValuesAccess)
{
    ccc::inplace_map<int, std::string, 5> m;
    m[3] = "three";
    m[1] = "one";
    m[2] = "two";
    const auto& k = m.keys();
    const auto& v = m.values();
    EXPECT_EQ(1, k[0]);
    EXPECT_EQ(2, k[1]);
    EXPECT_EQ(3, k[2]);
    EXPECT_EQ("one", v[0]);
}

TEST(InplaceMap, RangeConstruct)
{
    std::vector<std::pair<int, std::string>> src = {{3, "three"}, {1, "one"}, {2, "two"}};
    ccc::inplace_map<int, std::string, 5> m(src.begin(), src.end());
    EXPECT_EQ(3u, m.size());
    EXPECT_EQ(1, m.begin()->first);
}
