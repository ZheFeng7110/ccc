/**
 * @file type_cast_test.cc
 * @brief
 */

// NOLINTBEGIN(*-unary-static-assert)

#ifndef TEST_USE_MODULE
#include "ccc/pipe_operator_helper.hh"
#endif

#include "test_pch.hh"

#include <cstdint>
#include <cstring>

#include <type_traits>

#ifdef TEST_USE_MODULE
import ccc.pipe_operator_helper;
#endif

namespace {

namespace pipe_ = ccc::pipe_operator_helper;
using namespace pipe_::type_cast_operators;

#define STATIC_CAST_TEST(value, to_type)                                        \
    static_assert((value) == ((value) > pipe_::static_cast_to<to_type>()), ""); \
    CHECK(static_cast<to_type>(value) == ((value) > pipe_::static_cast_to<to_type>()))

}  // namespace

TEST_CASE("PipeOperatorStaticCast - BasicType")
{
    STATIC_CAST_TEST(123.0f, int);
    STATIC_CAST_TEST(114.514f, double);
    STATIC_CAST_TEST(114514, double);
}

TEST_CASE("PipeOperatorStaticCast - Pointer")
{
    int value = 1919;
    int* ptr = &value;
    CHECK(ptr == (ptr > pipe_::cast_to<int*>()));
    CHECK(value == *(ptr > pipe_::cast_to<int*>()));

    auto vptr = &value > pipe_::cast_to<void*>();
    CHECK(value == *(vptr > pipe_::cast_to<int*>()));

    CHECK(value == *(ptr > pipe_::cast_to<void*>() > pipe_::cast_to<int*>()));
}

TEST_CASE("PipeOperatorStaticCast - Class")
{
    struct Base {
        int a;
        float b;

        bool operator==(const Base& other) const
        {
            return a == other.a && b == other.b;
        }
    };

    struct Derived : Base {
        char c;
        short d;
    };

    Base base{114, 5.14f};
    Derived derived{{114, 5.14f}, 'a', 1919};
    CHECK(base == (derived > pipe_::cast_to<Base>()));

    Base& base_ref = base;
    Derived& derived_ref = derived;
    CHECK(base_ref == (derived_ref > pipe_::cast_to<const Base&>()));

    const Base& base_cref = base;
    const Derived& derived_cref = derived;
    CHECK(base_cref == (derived_cref > pipe_::cast_to<const Base&>()));

    Base* base_ptr = &base;
    Derived* derived_ptr = &derived;
    CHECK(*base_ptr == *(derived_ptr > pipe_::cast_to<Base*>()));
}

#undef STATIC_CAST_TEST

TEST_CASE("PipeOperatorDynamicCast - Class")
{
    struct Base {
        virtual ~Base() = default;

        int a;
        short b;
        char c;

        Base(const int a_, const short b_, const char c_) : a(a_), b(b_), c(c_) {}

        [[nodiscard]] virtual int getA() const
        {
            return a;
        }

        virtual short func()
        {
            return 0;
        }

        bool operator==(const Base& other) const
        {
            return a == other.a && b == other.b && c == other.c;
        }
    };

    struct Derived : Base {
        int a_;
        char c;
        short d;

        Derived(const Base& base, const int a, const char c_, const short d_) : Base(base), a_(a), c(c_), d(d_) {}

        [[nodiscard]] int getA() const override
        {
            return a_;
        }

        short func() override
        {
            return 214;
        }
    };

    Base base{1, 2, 'c'};
    Derived derived{{1, 2, 'c'}, 114, 'a', 1919};

    auto derived_casted = &derived > pipe_::dynamic_cast_to<Base*>();
    REQUIRE(derived_casted != nullptr);
    CHECK(base == *derived_casted);

    REQUIRE(nullptr == (&base > pipe_::dynamic_cast_to<Derived*>()));
}

TEST_CASE("PipeOperatorConstCast - BasicType")
{
    int a = 114;
    const int* a_cptr = &a;
    CHECK(a_cptr == (a_cptr > pipe_::const_cast_to<int*>()));
    CHECK(*a_cptr == *(a_cptr > pipe_::const_cast_to<int*>()));
    CHECK(a_cptr == (a_cptr > pipe_::const_cast_to<volatile int*>()));
    CHECK(*a_cptr == *(a_cptr > pipe_::const_cast_to<volatile int*>()));

    const int& a_cref = a;
    CHECK(a == a_cref);
    CHECK(a_cref == (a_cref > pipe_::const_cast_to<int&>()));
    CHECK(a_cref == (a_cref > pipe_::const_cast_to<volatile int&>()));

    short b = -1454;
    volatile short* b_vptr = &b;
    CHECK(b_vptr == (b_vptr > pipe_::const_cast_to<short*>()));
    CHECK(*b_vptr == *(b_vptr > pipe_::const_cast_to<short*>()));
    CHECK(b_vptr == (b_vptr > pipe_::const_cast_to<const short*>()));
    CHECK(*b_vptr == *(b_vptr > pipe_::const_cast_to<const short*>()));

    volatile short& b_vref = b;
    CHECK(b == b_vref);
    CHECK(b_vref == (b_vref > pipe_::const_cast_to<short&>()));
    CHECK(b_vref == (b_vref > pipe_::const_cast_to<const short&>()));
}

namespace {

template<typename T>
struct none_ptr_store_t {
    static_assert(std::is_void<T>::value, "Could not implements `ptr_store_t`");
    using type = T;
};

using ptr_store_t =
    std::conditional<sizeof(unsigned int) == sizeof(int*),
                     unsigned int,
                     std::conditional<sizeof(unsigned long) == sizeof(int*),
                                      unsigned long,
                                      std::conditional<sizeof(unsigned long long) == sizeof(int*),
                                                       unsigned long long,
                                                       none_ptr_store_t<void>::type>::type>::type>::type;

}  // namespace

TEST_CASE("PipeOperatorReinterpretCast - BasicType")
{
    int a = 3;
    int* pa = &a;

    auto pa_value = pa > pipe_::reinterpret_cast_to<ptr_store_t>();
    CHECK(pa_value == reinterpret_cast<ptr_store_t>(pa));

    auto v_to_pa = pa_value > pipe_::reinterpret_cast_to<int*>();
    CHECK(v_to_pa == reinterpret_cast<int*>(pa_value));
    CHECK(pa == v_to_pa);
    CHECK(a == *v_to_pa);
}

TEST_CASE("PipeOperatorBitCast - BasicType")
{
    int a = 3;
    int* pa = &a;

    auto pa_value = pa > pipe_::bit_cast_to<ptr_store_t>();
    ptr_store_t expect_pa_value;
    std::memcpy(&expect_pa_value, &pa, sizeof(int*));
    CHECK(pa_value == expect_pa_value);

    auto v_to_pa = pa_value > pipe_::bit_cast_to<int*>();
    int* expect_v_to_pa;
    std::memcpy(&expect_v_to_pa, &pa_value, sizeof pa_value);
    CHECK(v_to_pa == expect_v_to_pa);
    CHECK(pa == v_to_pa);
    CHECK(a == *v_to_pa);
}

namespace {

class TestMove
{
private:
    bool is_move_constructed_ = false;

public:
    constexpr TestMove() noexcept = default;
    constexpr TestMove(const TestMove&) noexcept {}
    constexpr TestMove(TestMove&&) noexcept : is_move_constructed_(true) {}

    [[nodiscard]] constexpr bool isMoveConstructed() const noexcept
    {
        return is_move_constructed_;
    }
};

}  // namespace

TEST_CASE("PipeOperatorMoveTo - Class")
{
    TestMove t0, t00;
    CHECK_FALSE(t0.isMoveConstructed());
    CHECK_FALSE(t00.isMoveConstructed());

    TestMove t1{t0};
    TestMove t2{std::move(t0)};
    CHECK_FALSE(t1.isMoveConstructed());
    CHECK(t2.isMoveConstructed());

    TestMove t3{t00 > pipe_::move_to()};
    CHECK(t3.isMoveConstructed());
}

namespace {
namespace test_forward {

constexpr bool is_rvalue(int&&) noexcept
{
    return true;
}

constexpr bool is_rvalue(const int&) noexcept
{
    return false;
}

template<typename T>
constexpr bool std_forward(T&& t) noexcept
{
    return is_rvalue(std::forward<T>(t));
}

template<typename T>
constexpr bool forward_to(T&& t) noexcept
{
    return is_rvalue(t > pipe_::forward_to<T>());
}

}  // namespace test_forward
}  // namespace

TEST_CASE("PipeOperatorForwardTo - BasicType")
{
    using namespace test_forward;

    int a = 1;
    CHECK(is_rvalue(1));
    CHECK_FALSE(is_rvalue(a));

    CHECK(std_forward(1));
    CHECK_FALSE(std_forward(a));

    CHECK(forward_to(1));
    CHECK_FALSE(forward_to(a));

    int& a_ref = a;
    CHECK_FALSE(is_rvalue(a_ref));
    CHECK_FALSE(std_forward(a_ref));
    CHECK_FALSE(forward_to(a_ref));

    CHECK(is_rvalue(std::move(a_ref)));    // NOLINT(*-move-const-arg)
    CHECK(std_forward(std::move(a_ref)));  // NOLINT(*-move-const-arg)
    CHECK(forward_to(std::move(a_ref)));   // NOLINT(*-move-const-arg)
}

// NOLINTEND(*-unary-static-assert)
