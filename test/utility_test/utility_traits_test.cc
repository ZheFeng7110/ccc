#ifndef TEST_USE_MODULE
#include "ccc/utility.hh"
#endif

#include "test_pch.hh"

#include <tuple>

#ifdef TEST_USE_MODULE
import ccc.utility;
#endif

TEST(UtilityTraits, VoidT)
{
    static_assert(std::is_same<void, ccc::void_t<int>>::value, "");
    static_assert(std::is_same<void, ccc::void_t<int, double>>::value, "");
    static_assert(std::is_same<void, ccc::void_t<>>::value, "");
}

TEST(UtilityTraits, Conjunction)
{
    static_assert(ccc::conjunction<>::value, "");
    static_assert(ccc::conjunction<std::true_type>::value, "");
    static_assert(!ccc::conjunction<std::false_type>::value, "");
    static_assert(ccc::conjunction<std::true_type, std::true_type>::value, "");
    static_assert(!ccc::conjunction<std::true_type, std::false_type>::value, "");
    static_assert(!ccc::conjunction<std::false_type, std::false_type>::value, "");
}

TEST(UtilityTraits, Disjunction)
{
    static_assert(!ccc::disjunction<>::value, "");
    static_assert(ccc::disjunction<std::true_type>::value, "");
    static_assert(!ccc::disjunction<std::false_type>::value, "");
    static_assert(ccc::disjunction<std::true_type, std::true_type>::value, "");
    static_assert(ccc::disjunction<std::false_type, std::true_type>::value, "");
    static_assert(!ccc::disjunction<std::false_type, std::false_type>::value, "");
}

TEST(UtilityTraits, Negation)
{
    static_assert(ccc::negation<std::false_type>::value, "");
    static_assert(!ccc::negation<std::true_type>::value, "");
}

TEST(UtilityTraits, TypeIdentity)
{
    static_assert(std::is_same<int, typename ccc::type_identity<int>::type>::value, "");
    static_assert(std::is_same<int, ccc::type_identity_t<int>>::value, "");
}

TEST(UtilityTraits, BoundedArray)
{
    static_assert(ccc::is_bounded_array<int[5]>::value, "");
    static_assert(!ccc::is_bounded_array<int[]>::value, "");
    static_assert(!ccc::is_bounded_array<int>::value, "");
    static_assert(!ccc::is_bounded_array<int*>::value, "");
}

TEST(UtilityTraits, UnboundedArray)
{
    static_assert(ccc::is_unbounded_array<int[]>::value, "");
    static_assert(!ccc::is_unbounded_array<int[5]>::value, "");
    static_assert(!ccc::is_unbounded_array<int>::value, "");
    static_assert(!ccc::is_unbounded_array<int*>::value, "");
}

TEST(UtilityTraits, IsSwappable)
{
    static_assert(ccc::is_swappable<int>::value, "");
    static_assert(ccc::is_swappable_with<int&, int&>::value, "");
}

#if (__cplusplus >= 201703L)
TEST(UtilityTraits, IsNothrowSwappable)
{
    static_assert(ccc::is_nothrow_swappable<int>::value, "");
    static_assert(ccc::is_nothrow_swappable_with<int&, int&>::value, "");
}
#endif

TEST(UtilityTraits, IsNothrowConvertible)
{
#if (__cplusplus >= 201703L)
    static_assert(ccc::is_nothrow_convertible<int, double>::value, "");
    static_assert(!ccc::is_nothrow_convertible<int, void>::value, "");
    static_assert(ccc::is_nothrow_convertible<void, void>::value, "");
#endif
}

TEST(UtilityTraits, RemoveCv)
{
    static_assert(std::is_same<int, ccc::remove_cv_t<const int>>::value, "");
    static_assert(std::is_same<int, ccc::remove_cv_t<volatile int>>::value, "");
    static_assert(std::is_same<int, ccc::remove_cv_t<const volatile int>>::value, "");
    static_assert(std::is_same<int, ccc::remove_cv_t<int>>::value, "");
}

TEST(UtilityTraits, RemoveConst)
{
    static_assert(std::is_same<int, ccc::remove_const_t<const int>>::value, "");
    static_assert(std::is_same<int, ccc::remove_const_t<int>>::value, "");
    static_assert(std::is_same<volatile int, ccc::remove_const_t<const volatile int>>::value, "");
}

TEST(UtilityTraits, RemoveVolatile)
{
    static_assert(std::is_same<int, ccc::remove_volatile_t<volatile int>>::value, "");
    static_assert(std::is_same<int, ccc::remove_volatile_t<int>>::value, "");
    static_assert(std::is_same<const int, ccc::remove_volatile_t<const volatile int>>::value, "");
}

TEST(UtilityTraits, AddCv)
{
    static_assert(std::is_same<const volatile int, ccc::add_cv_t<int>>::value, "");
}

TEST(UtilityTraits, AddConst)
{
    static_assert(std::is_same<const int, ccc::add_const_t<int>>::value, "");
    static_assert(std::is_same<const int, ccc::add_const_t<const int>>::value, "");
}

TEST(UtilityTraits, AddVolatile)
{
    static_assert(std::is_same<volatile int, ccc::add_volatile_t<int>>::value, "");
    static_assert(std::is_same<volatile int, ccc::add_volatile_t<volatile int>>::value, "");
}

TEST(UtilityTraits, RemoveReference)
{
    static_assert(std::is_same<int, ccc::remove_reference_t<int>>::value, "");
    static_assert(std::is_same<int, ccc::remove_reference_t<int&>>::value, "");
    static_assert(std::is_same<int, ccc::remove_reference_t<int&&>>::value, "");
}

TEST(UtilityTraits, AddLvalueReference)
{
    static_assert(std::is_same<int&, ccc::add_lvalue_reference_t<int>>::value, "");
    static_assert(std::is_same<int&, ccc::add_lvalue_reference_t<int&>>::value, "");
    static_assert(std::is_same<int&, ccc::add_lvalue_reference_t<int&&>>::value, "");
    static_assert(std::is_same<void, ccc::add_lvalue_reference_t<void>>::value, "");
}

TEST(UtilityTraits, AddRvalueReference)
{
    static_assert(std::is_same<int&&, ccc::add_rvalue_reference_t<int>>::value, "");
    static_assert(std::is_same<int&, ccc::add_rvalue_reference_t<int&>>::value, "");
    static_assert(std::is_same<void, ccc::add_rvalue_reference_t<void>>::value, "");
}

TEST(UtilityTraits, MakeSigned)
{
    static_assert(std::is_same<int, ccc::make_signed_t<unsigned int>>::value, "");
    static_assert(std::is_same<int, ccc::make_signed_t<int>>::value, "");
}

TEST(UtilityTraits, MakeUnsigned)
{
    static_assert(std::is_same<unsigned int, ccc::make_unsigned_t<int>>::value, "");
    static_assert(std::is_same<unsigned int, ccc::make_unsigned_t<unsigned int>>::value, "");
}

TEST(UtilityTraits, RemoveExtent)
{
    static_assert(std::is_same<int, ccc::remove_extent_t<int[5]>>::value, "");
    static_assert(std::is_same<int[3], ccc::remove_extent_t<int[5][3]>>::value, "");
    static_assert(std::is_same<int, ccc::remove_extent_t<int>>::value, "");
}

TEST(UtilityTraits, RemoveAllExtents)
{
    static_assert(std::is_same<int, ccc::remove_all_extents_t<int[5][3][2]>>::value, "");
    static_assert(std::is_same<int, ccc::remove_all_extents_t<int>>::value, "");
}

TEST(UtilityTraits, RemovePointer)
{
    static_assert(std::is_same<int, ccc::remove_pointer_t<int*>>::value, "");
    static_assert(std::is_same<int, ccc::remove_pointer_t<int>>::value, "");
    static_assert(std::is_same<const int, ccc::remove_pointer_t<const int*>>::value, "");
}

TEST(UtilityTraits, AddPointer)
{
    static_assert(std::is_same<int*, ccc::add_pointer_t<int>>::value, "");
    static_assert(std::is_same<int*, ccc::add_pointer_t<int&>>::value, "");
    static_assert(std::is_same<const int*, ccc::add_pointer_t<const int&>>::value, "");
}

TEST(UtilityTraits, Decay)
{
    static_assert(std::is_same<int, ccc::decay_t<int>>::value, "");
    static_assert(std::is_same<int, ccc::decay_t<int&>>::value, "");
    static_assert(std::is_same<int, ccc::decay_t<const int&>>::value, "");
    static_assert(std::is_same<int*, ccc::decay_t<int[5]>>::value, "");
    static_assert(std::is_same<int (*)(int), ccc::decay_t<int(int)>>::value, "");
}

TEST(UtilityTraits, CommonType)
{
    static_assert(std::is_same<double, ccc::common_type_t<int, double>>::value, "");
    static_assert(std::is_same<int, ccc::common_type_t<int>>::value, "");
    static_assert(std::is_same<double, ccc::common_type_t<int, double, float>>::value, "");
}

TEST(UtilityTraits, UnderlyingType)
{
    enum class MyEnum : char {
        A,
        B
    };
    static_assert(std::is_same<char, ccc::underlying_type_t<MyEnum>>::value, "");
}

TEST(UtilityTraits, RemoveCvref)
{
    static_assert(std::is_same<int, ccc::remove_cvref_t<const int&>>::value, "");
    static_assert(std::is_same<int, ccc::remove_cvref_t<int&&>>::value, "");
    static_assert(std::is_same<int, ccc::remove_cvref_t<int>>::value, "");
    static_assert(std::is_same<int, ccc::remove_cvref_t<const int>>::value, "");
}

TEST(UtilityTraits, EnableIf)
{
    static_assert(std::is_same<void, ccc::enable_if_t<true>>::value, "");
    static_assert(std::is_same<int, ccc::enable_if_t<true, int>>::value, "");
}

TEST(UtilityTraits, Conditional)
{
    static_assert(std::is_same<int, ccc::conditional_t<true, int, double>>::value, "");
    static_assert(std::is_same<double, ccc::conditional_t<false, int, double>>::value, "");
}

TEST(UtilityTraits, TupleElement)
{
    static_assert(std::is_same<int, ccc::tuple_element_t<0, std::tuple<int, double>>>::value, "");
    static_assert(std::is_same<double, ccc::tuple_element_t<1, std::tuple<int, double>>>::value, "");
    static_assert(std::is_same<char, ccc::tuple_element_t<2, std::tuple<int, double, char>>>::value, "");
}
