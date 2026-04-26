/**
 * @file expected_impl.hh
 * @brief
 */

// ReSharper disable CppRedundantInlineSpecifier
// ReSharper disable CppUseTypeTraitAlias

#pragma once
#ifndef CCC_EXPECTED_EXPECTED_IMPL_HH
#define CCC_EXPECTED_EXPECTED_IMPL_HH

#include "unexpected.hh"

namespace ccc {

template<typename T, typename E, typename Criterion>
class expected
{
public:
    static_assert(!std::is_reference<T>::value, "T must not be a reference");
    static_assert(!std::is_function<T>::value, "T must not be a function");
    static_assert(!std::is_same<std::remove_cv_t<T>, in_place_t>::value, "T must not be in_place_t");
    static_assert(!std::is_same<std::remove_cv_t<T>, unexpect_t>::value, "T must not be unexpect_t");
    static_assert(detail::can_be_unexpected_v<E>, "E must be a valid unexpected type");

    using value_type = T;
    using error_type = E;

    using unexpected_type = unexpected<E>;

    using criterion_type = Criterion;

    static_assert(std::is_nothrow_copy_constructible<criterion_type>::value,
                  "Criterion must be nothrow copy constructible");
    static_assert(std::is_nothrow_move_constructible<criterion_type>::value,
                  "Criterion must be nothrow move constructible");
    static_assert(std::is_default_constructible<criterion_type>::value, "Criterion must be default constructible");

private:
    union {
        value_type value_;
        error_type error_;
    };
    criterion_type criterion_;

public:
    CCC_NO_DISCARD constexpr bool has_value() const noexcept
    {
        return criterion_.has_value();
    }

public:
    constexpr expected() : value_(), criterion_() {}

    constexpr expected(const expected&) noexcept(std::is_nothrow_copy_constructible<value_type>::value &&
                                                 std::is_nothrow_copy_constructible<error_type>::value) = default;

    template<typename std::enable_if<std::is_copy_constructible<value_type>::value &&
                                     std::is_copy_constructible<error_type>::value &&
                                     !std::is_trivially_copy_constructible<value_type>::value &&
                                     !std::is_trivially_copy_constructible<error_type>::value>::type* = nullptr>
    constexpr expected(const expected& other) noexcept(std::is_nothrow_copy_constructible<value_type>::value &&
                                                       std::is_nothrow_copy_constructible<error_type>::value)
        : criterion_(other.criterion_)
    {
        if (has_value()) {
            value_ = other.value_;
        }
        else {
            error_ = other.error_;
        }
    }

    constexpr expected(expected&&) noexcept(std::is_nothrow_move_constructible<value_type>::value &&
                                            std::is_nothrow_move_constructible<error_type>::value) = default;

    template<typename std::enable_if<std::is_move_constructible<value_type>::value &&
                                     std::is_move_constructible<error_type>::value &&
                                     !std::is_trivially_move_constructible<value_type>::value &&
                                     !std::is_trivially_move_constructible<error_type>::value>::type* = nullptr>
    constexpr expected(expected&& other) noexcept(std::is_nothrow_move_constructible<value_type>::value &&
                                                  std::is_nothrow_move_constructible<error_type>::value)
        : criterion_(std::move(other.criterion_))
    {
        if (has_value()) {
            value_ = std::move(other.value_);
        }
        else {
            error_ = std::move(other.error_);
        }
    }
};

}  // namespace ccc

#endif  // !CCC_EXPECTED_EXPECTED_IMPL_HH
