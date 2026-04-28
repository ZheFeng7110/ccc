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

class default_criterion
{
public:
    using value_type = bool;

private:
    value_type has_value_;

public:
    static constexpr value_type default_error_value = false;

    constexpr default_criterion() noexcept : has_value_(true) {}

    explicit constexpr default_criterion(const bool has_value) noexcept : has_value_(has_value) {}

    CCC_NO_DISCARD constexpr bool has_value() const noexcept
    {
        return has_value_;
    }
};
static_assert(is_criterion_v<default_criterion>, "default_criterion must satisfy the criterion requirements");

template<typename T, typename E, typename Criterion>
class expected;

namespace detail {

template<typename>
struct is_expected : std::false_type {
};

template<typename T, typename E, typename Criterion>
struct is_expected<expected<T, E, Criterion>> : std::true_type {
};

template<typename T>
constexpr bool is_expected_v = is_expected<T>::value;

template<typename T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

}  // namespace detail

template<typename T, typename E, typename Criterion = default_criterion>
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

    static_assert(is_criterion_v<criterion_type>, "Criterion must satisfy the criterion requirements");
    static_assert(std::is_nothrow_copy_constructible<criterion_type>::value,
                  "Criterion must be nothrow copy constructible");
    static_assert(std::is_nothrow_move_constructible<criterion_type>::value,
                  "Criterion must be nothrow move constructible");
    static_assert(std::is_default_constructible<criterion_type>::value, "Criterion must be default constructible");

private:
    union {
        std::remove_cv<value_type>::type value_;
        error_type error_;
    };
    criterion_type criterion_;

public:
    CCC_NO_DISCARD constexpr bool has_value() const noexcept
    {
        return criterion_.has_value();
    }

public:  // Constructors
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

    template<typename U,
             typename G,
             typename Cr,
             typename std::enable_if<std::is_constructible<value_type, const U&>::value &&
                                     std::is_constructible<error_type, const G&>::value &&
                                     std::is_constructible<criterion_type, const Cr&>::value>::type* = nullptr>
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible<const U&, value_type>::value ||
                              !std::is_convertible<const G&, error_type>::value ||
                              !std::is_convertible<const Cr&, criterion_type>::value))
    constexpr expected(const expected<U, G, Cr>& other)
        : criterion_(other.criterion_)
    {
        if (has_value()) {
            value_ = value_type(other.value_);
        }
        else {
            error_ = error_type(other.error_);
        }
    }

    template<typename U,
             typename G,
             typename Cr,
             typename std::enable_if<std::is_constructible<value_type, U>::value &&
                                     std::is_constructible<error_type, G>::value &&
                                     std::is_constructible<criterion_type, Cr>::value>::type* = nullptr>
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible<U, value_type>::value ||
                              !std::is_convertible<G, error_type>::value ||
                              !std::is_convertible<Cr, criterion_type>::value))
    constexpr expected(expected<U, G, Criterion>&& other)
        : criterion_(std::move(other.criterion_))
    {
        if (has_value()) {
            value_ = value_type(std::move(other.value_));
        }
        else {
            error_ = error_type(std::move(other.error_));
        }
    }

    template<typename U = std::remove_cv<value_type>::type,
             typename Cr = std::remove_cv<criterion_type>::type,
             typename std::enable_if<!std::is_same<detail::remove_cvref_t<U>, in_place_t>::value &&
                                     !detail::is_expected_v<detail::remove_cvref_t<U>> &&
                                     !detail::is_unexpected_v<detail::remove_cvref_t<U>> &&
                                     std::is_constructible<value_type, U>::value &&
                                     std::is_constructible<criterion_type, Cr>::value>::type* = nullptr>
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible<U, value_type>::value ||
                              !std::is_convertible<Cr, criterion_type>::value))
    constexpr expected(U&& v, Cr&& criterion = {})
        : value_(std::forward<U>(v)), criterion_(std::forward<Cr>(criterion))
    {
    }

    template<typename G,
             typename Cr = std::remove_cv<criterion_type>::type,
             typename std::enable_if<std::is_constructible<error_type, const G&>::value &&
                                     std::is_constructible<criterion_type, const Cr&>::value>::type* = nullptr>
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible_v<const G&, error_type> ||
                              !std::is_convertible<const Cr&, criterion_type>::value))
    constexpr expected(const unexpected<G>& e, const Cr& cr = Cr::default_error_value)
        : error_(std::forward<const G&>(e.error())), criterion_(std::forward<const Cr&>(cr))
    {
    }

    template<typename G,
             typename Cr = std::remove_cv<criterion_type>::type,
             typename std::enable_if<std::is_constructible<error_type, G>::value &&
                                     std::is_constructible<criterion_type, Cr>::value>::type* = nullptr>
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible_v<G, error_type> || !std::is_convertible<Cr, criterion_type>::value))
    constexpr expected(unexpected<G>&& e, Cr&& cr = Cr::default_error_value)
        : error_(std::forward<G>(e.error())), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename... Args,
             typename std::enable_if<std::is_constructible<value_type, Args...>::value>::type* = nullptr>
    explicit constexpr expected(in_place_t, Args&&... args) : value_(std::forward<Args>(args)...), criterion_()
    {
    }

    template<typename Cr,
             typename... Args,
             typename std::enable_if<std::is_constructible<value_type, Args...>::value &&
                                     std::is_constructible<criterion_type, Cr>::value &&
                                     !std::is_same<detail::remove_cvref_t<Cr>, in_place_t>::value>::type* = nullptr>
    constexpr expected(Cr&& cr, in_place_t, Args&&... args)
        : value_(std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename U,
             typename... Args,
             typename std::enable_if<
                 std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value>::type* = nullptr>
    explicit constexpr expected(in_place_t, std::initializer_list<U> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...), criterion_()
    {
    }

    template<typename Cr,
             typename U,
             typename... Args,
             typename std::enable_if<std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value &&
                                     std::is_constructible<criterion_type, Cr>::value &&
                                     !std::is_same<detail::remove_cvref_t<Cr>, in_place_t>::value>::type* = nullptr>
    constexpr expected(Cr&& cr, in_place_t, std::initializer_list<U> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename... Args,
             typename std::enable_if<std::is_constructible<value_type, Args...>::value>::type* = nullptr>
    explicit constexpr expected(unexpect_t, Args&&... args)
        : value_(std::forward<Args>(args)...), criterion_(criterion_type::default_error_value)
    {
    }

    template<typename Cr,
             typename... Args,
             typename std::enable_if<std::is_constructible<value_type, Args...>::value &&
                                     std::is_constructible<criterion_type, Cr>::value &&
                                     !std::is_same<detail::remove_cvref_t<Cr>, unexpect_t>::value>::type* = nullptr>
    constexpr expected(Cr&& cr, unexpect_t, Args&&... args)
        : value_(std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename U,
             typename... Args,
             typename std::enable_if<
                 std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value>::type* = nullptr>
    explicit constexpr expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...), criterion_(criterion_type::default_error_value)
    {
    }

    template<typename Cr,
             typename U,
             typename... Args,
             typename std::enable_if<std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value &&
                                     std::is_constructible<criterion_type, Cr>::value &&
                                     !std::is_same<detail::remove_cvref_t<Cr>, unexpect_t>::value>::type* = nullptr>
    constexpr expected(Cr&& cr, unexpect_t, std::initializer_list<U> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

private:  // Assign

};

}  // namespace ccc

#endif  // !CCC_EXPECTED_EXPECTED_IMPL_HH
