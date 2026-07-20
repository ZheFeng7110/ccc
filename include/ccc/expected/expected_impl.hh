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

    constexpr bool operator==(const default_criterion& other) const noexcept
    {
        return this->has_value_ == other.has_value_;
    }
    constexpr bool operator!=(const default_criterion& other) const noexcept
    {
        return this->has_value_ != other.has_value_;
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

namespace expected_impl {

#if (__cplusplus >= 201703L)

template<typename Func, typename T, typename Cr>
using result = remove_cvref_t<std::invoke_result_t<Func&&, T&&, Cr&&>>;

template<typename Func, typename T, typename Cr>
using result_xform = remove_cv_t<std::invoke_result_t<Func&&, T&&, Cr&&>>;

inline constexpr struct in_place_invoke_tag {
} in_place_invoke;

inline constexpr struct unexpect_invoke_tag {
} unexpect_invoke;

template<typename>
struct is_transform_func_return_2values : std::false_type {
};

template<typename T, typename U>
struct is_transform_func_return_2values<std::pair<T, U>> : std::true_type {
};

template<typename T, typename U>
struct is_transform_func_return_2values<std::tuple<T, U>> : std::true_type {
};

template<typename T>
struct is_transform_func_return_2values<std::array<T, 2>> : std::true_type {
};

template<typename T>
constexpr bool is_transform_func_return_2values_v = is_transform_func_return_2values<T>::value;

inline constexpr struct transform_func_return_2values_tag {
} transform_func_return_2_values;

#endif  // (__cplusplus >= 201703L)

template<typename T>
struct Guard {
    static_assert(std::is_nothrow_move_constructible<T>::value, "T must be nothrow move constructible");

    explicit CCC_CPP20_CONSTEXPR Guard(T& v) : guarded_addr_(std::addressof(v)), temp_(std::move(v))
    {
        destroy_at(guarded_addr_);
    }

    CCC_CPP20_CONSTEXPR ~Guard()
    {
        if (guarded_addr_ != nullptr) {
            ccc::construct_at(guarded_addr_, std::move(temp_));
        }
    }

    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;

    constexpr T&& release() noexcept
    {
        guarded_addr_ = nullptr;
        return std::move(temp_);
    }

private:
    T* guarded_addr_;
    T temp_;
};

struct reinit_no_throw_construct {
    template<typename NewType, typename OldType, typename Arg>
    static CCC_CPP20_CONSTEXPR void invoke(NewType* new_addr, OldType* old_addr, Arg&& arg)
    {
        ccc::destroy_at(old_addr);
        ccc::construct_at(new_addr, std::forward<Arg>(arg));
    }
};
struct reinit_no_throw_move_construct {
    template<typename NewType, typename OldType, typename Arg>
    static CCC_CPP20_CONSTEXPR void invoke(NewType* new_addr, OldType* old_addr, Arg&& arg)
    {
        NewType new_temp(std::forward<Arg>(arg));
        ccc::destroy_at(old_addr);
        ccc::construct_at(new_addr, std::move(new_temp));
    }
};
struct reinit_other_branch {
    template<typename NewType, typename OldType, typename Arg>
    static CCC_CPP20_CONSTEXPR void invoke(NewType* new_addr, OldType* old_addr, Arg&& arg)
    {
        Guard<OldType> guard(*old_addr);  // to restore old value without using try-catch statements
        ccc::construct_at(new_addr, std::forward<Arg>(arg));
        (void)guard.release();
    }
};

template<typename NewType, typename OldType, typename Arg>
CCC_CPP20_CONSTEXPR void reinit(NewType* new_addr,
                                OldType* old_addr,
                                Arg&& arg) noexcept(std::is_nothrow_constructible<NewType, Arg>::value)
{
    // clang-format off
    conditional_t<is_nothrow_constructible_v<NewType, Arg>, reinit_no_throw_construct,
        conditional_t<is_nothrow_move_constructible_v<NewType>, reinit_no_throw_move_construct,
            reinit_other_branch
        >
    >::invoke(new_addr, old_addr, std::forward<Arg>(arg));
    // clang-format on
}

}  // namespace expected_impl

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
    static_assert(!is_same_v<remove_cvref<criterion_type>, in_place_t> &&
                      !is_same_v<remove_cvref_t<criterion_type>, unexpect_t>,
                  "criterion_type must not be in_place_t or unexpect_t");
    static_assert(std::is_nothrow_copy_constructible<criterion_type>::value,
                  "Criterion must be nothrow copy constructible");
    static_assert(std::is_nothrow_move_constructible<criterion_type>::value,
                  "Criterion must be nothrow move constructible");
    static_assert(std::is_default_constructible<criterion_type>::value, "Criterion must be default constructible");

private:
    union {
        remove_cv_t<value_type> value_;
        error_type error_;
    };
    criterion_type criterion_;

public:
    CCC_NO_DISCARD constexpr bool has_value() const noexcept
    {
        return criterion_.has_value();
    }
    CCC_NO_DISCARD explicit constexpr operator bool() const noexcept
    {
        return has_value();
    }

    CCC_NO_DISCARD constexpr const criterion_type& get_criterion() const noexcept
    {
        return criterion_;
    }

public:  // Destructors
#ifdef __cpp_concepts
    constexpr ~expected() = default;

    constexpr ~expected()
        requires(!is_trivially_destructible_v<value_type> || !is_trivially_destructible_v<error_type> ||
                 !is_trivially_destructible_v<criterion_type>)
    {
        if (has_value()) {
            ccc::destroy_at(std::addressof(value_));
        }
        else {
            ccc::destroy_at(std::addressof(error_));
        }
        ccc::destroy_at(std::addressof(criterion_));
    }
#else
    CCC_CPP20_CONSTEXPR ~expected()
    {
        if CCC_CPP17_CONSTEXPR (!is_trivially_destructible_v<value_type> || !is_trivially_destructible_v<error_type> ||
                                !is_trivially_destructible_v<criterion_type>) {
            if (has_value()) {
                ccc::destroy_at(std::addressof(value_));
            }
            else {
                ccc::destroy_at(std::addressof(error_));
            }
            ccc::destroy_at(std::addressof(criterion_));
        }
    }
#endif

public:  // Constructors
    constexpr expected() : value_(), criterion_() {}

#ifdef __cpp_concepts
    constexpr expected(const expected&) = default;
#endif

    constexpr expected(const expected& other) noexcept(std::is_nothrow_copy_constructible<value_type>::value &&
                                                       std::is_nothrow_copy_constructible<error_type>::value)
#ifdef __cpp_concepts
        requires(is_copy_constructible_v<value_type> && is_copy_constructible_v<error_type> &&
                 !is_trivially_copy_constructible_v<value_type> && !is_trivially_copy_constructible_v<error_type>)
#endif
        : criterion_(other.criterion_)
    {
        if (has_value()) {
            value_ = other.value_;
        }
        else {
            error_ = other.error_;
        }
    }

#ifdef __cpp_concepts
    constexpr expected(expected&&) = default;
#endif

    constexpr expected(expected&& other) noexcept(std::is_nothrow_move_constructible<value_type>::value &&
                                                  std::is_nothrow_move_constructible<error_type>::value)
#ifdef __cpp_concepts
        requires(is_move_constructible_v<value_type> && is_move_constructible_v<error_type> &&
                 !is_trivially_move_constructible_v<value_type> && !is_trivially_move_constructible_v<error_type>)
#endif
        : criterion_(std::move(other.criterion_))
    {
        if (has_value()) {
            value_ = std::move(other.value_);
        }
        else {
            error_ = std::move(other.error_);
        }
    }

    template<
        typename U,
        typename G,
        typename Cr
#ifndef __cpp_concepts
        ,
        typename std::enable_if<
            std::is_constructible<value_type, const U&>::value && std::is_constructible<error_type, const G&>::value &&
            std::is_constructible<criterion_type, const Cr&>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
            !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
        >
#ifdef __cpp_concepts
        requires(std::is_constructible<value_type, const U&>::value &&
                 std::is_constructible<error_type, const G&>::value &&
                 std::is_constructible<criterion_type, const Cr&>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
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
             typename Cr
#ifndef __cpp_concepts
             ,
             typename std::enable_if<
                 std::is_constructible<value_type, U>::value && std::is_constructible<error_type, G>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<value_type, U>::value && std::is_constructible<error_type, G>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
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

    template<typename U = remove_cv_t<value_type>,
             typename Cr = remove_cv_t<criterion_type>
#ifndef __cpp_concepts
             ,
             typename std::enable_if<
                 !std::is_same<remove_cvref_t<U>, in_place_t>::value && !detail::is_expected_v<remove_cvref_t<U>> &&
                 !detail::is_unexpected_v<remove_cvref_t<U>> && std::is_constructible<value_type, U>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(!std::is_same<remove_cvref_t<U>, in_place_t>::value && !detail::is_expected_v<remove_cvref_t<U>> &&
                 !detail::is_unexpected_v<remove_cvref_t<U>> && std::is_constructible<value_type, U>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible<U, value_type>::value ||
                              !std::is_convertible<Cr, criterion_type>::value))
    constexpr expected(U&& v, Cr&& criterion = Cr(criterion_type()))
        : value_(std::forward<U>(v)), criterion_(std::forward<Cr>(criterion))
    {
    }

    template<typename G,
             typename Cr = remove_cv_t<criterion_type>
#ifndef __cpp_concepts
             ,
             typename std::enable_if<std::is_constructible<error_type, const G&>::value &&
                                     std::is_constructible<criterion_type, const Cr&>::value &&
                                     !is_same_v<remove_cvref<Cr>, in_place_t> &&
                                     !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<error_type, const G&>::value &&
                 std::is_constructible<criterion_type, const Cr&>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible_v<const G&, error_type> ||
                              !std::is_convertible<const Cr&, criterion_type>::value))
    constexpr expected(const unexpected<G>& e, const Cr& cr = Cr(criterion_type::default_error_value))
        : error_(std::forward<const G&>(e.error())), criterion_(std::forward<const Cr&>(cr))
    {
    }

    template<
        typename G,
        typename Cr = remove_cv_t<criterion_type>
#ifndef __cpp_concepts
        ,
        typename std::enable_if<
            std::is_constructible<error_type, G>::value && std::is_constructible<criterion_type, Cr>::value &&
            !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
        >
#ifdef __cpp_concepts
        requires(std::is_constructible<error_type, G>::value && std::is_constructible<criterion_type, Cr>::value &&
                 !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
    CCC_CONDITIONAL_EXPLICIT((!std::is_convertible_v<G, error_type> || !std::is_convertible<Cr, criterion_type>::value))
    constexpr expected(unexpected<G>&& e, Cr&& cr = Cr(criterion_type::default_error_value))
        : error_(std::forward<G>(e.error())), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename... Args
#ifndef __cpp_concepts
             ,
             typename std::enable_if<std::is_constructible<value_type, Args...>::value>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<value_type, Args...>::value)
#endif
    explicit constexpr expected(in_place_t, Args&&... args) : value_(std::forward<Args>(args)...), criterion_()
    {
    }

    template<
        typename Cr,
        typename... Args
#ifndef __cpp_concepts
        ,
        typename std::enable_if<
            std::is_constructible<value_type, Args...>::value && std::is_constructible<criterion_type, Cr>::value &&
            !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
        >
#ifdef __cpp_concepts
        requires(std::is_constructible<value_type, Args...>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
    constexpr expected(Cr&& cr, in_place_t, Args&&... args)
        : value_(std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename U,
             typename... Args
#ifndef __cpp_concepts
             ,
             typename std::enable_if<
                 std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value)
#endif
    explicit constexpr expected(in_place_t, std::initializer_list<U> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...), criterion_()
    {
    }

    template<typename Cr,
             typename U,
             typename... Args
#ifndef __cpp_concepts
             ,
             typename std::enable_if<std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value &&
                                     std::is_constructible<criterion_type, Cr>::value &&
                                     !is_same_v<remove_cvref<Cr>, in_place_t> &&
                                     !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<value_type, std::initializer_list<U>&, Args...>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
    constexpr expected(Cr&& cr, in_place_t, std::initializer_list<U> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename... Args
#ifndef __cpp_concepts
             ,
             typename std::enable_if<std::is_constructible<error_type, Args...>::value>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<error_type, Args...>::value)
#endif
    explicit constexpr expected(unexpect_t, Args&&... args)
        : error_(std::forward<Args>(args)...), criterion_(criterion_type::default_error_value)
    {
    }

    template<
        typename Cr,
        typename... Args
#ifndef __cpp_concepts
        ,
        typename std::enable_if<
            std::is_constructible<error_type, Args...>::value && std::is_constructible<criterion_type, Cr>::value &&
            !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
        >
#ifdef __cpp_concepts
        requires(std::is_constructible<error_type, Args...>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
    constexpr expected(Cr&& cr, unexpect_t, Args&&... args)
        : error_(std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

    template<typename U,
             typename... Args
#ifndef __cpp_concepts
             ,
             typename std::enable_if<
                 std::is_constructible<error_type, std::initializer_list<U>&, Args...>::value>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<error_type, std::initializer_list<U>&, Args...>::value)
#endif
    explicit constexpr expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
        : error_(il, std::forward<Args>(args)...), criterion_(criterion_type::default_error_value)
    {
    }

    template<typename Cr,
             typename U,
             typename... Args
#ifndef __cpp_concepts
             ,
             typename std::enable_if<std::is_constructible<error_type, std::initializer_list<U>&, Args...>::value &&
                                     std::is_constructible<criterion_type, Cr>::value &&
                                     !is_same_v<remove_cvref<Cr>, in_place_t> &&
                                     !is_same_v<remove_cvref_t<Cr>, unexpect_t>>::type* = nullptr
#endif
             >
#ifdef __cpp_concepts
        requires(std::is_constructible<error_type, std::initializer_list<U>&, Args...>::value &&
                 std::is_constructible<criterion_type, Cr>::value && !is_same_v<remove_cvref<Cr>, in_place_t> &&
                 !is_same_v<remove_cvref_t<Cr>, unexpect_t>)
#endif
    constexpr expected(Cr&& cr, unexpect_t, std::initializer_list<U> il, Args&&... args)
        : error_(il, std::forward<Args>(args)...), criterion_(std::forward<Cr>(cr))
    {
    }

private:  // Assign
    template<typename U, typename Cr = remove_cv_t<criterion_type>>
    CCC_CPP20_CONSTEXPR void assign_value_(U&& value, Cr&& cr = criterion_type())
    {
        if (has_value()) {
            value_ = std::forward<U>(value);
        }
        else {
            detail::expected_impl::reinit(std::addressof(value_), std::addressof(error_), std::forward<U>(value));
        }
        criterion_ = std::forward<Cr>(cr);
    }

    template<typename U, typename Cr = remove_cv_t<criterion_type>>
    CCC_CPP20_CONSTEXPR void assign_error_(U&& error, Cr&& cr = criterion_type::default_error_value)
    {
        if (!has_value()) {
            error_ = std::forward<U>(error);
        }
        else {
            detail::expected_impl::reinit(std::addressof(error_), std::addressof(value_), std::forward<U>(error));
        }
        criterion_ = std::forward<Cr>(cr);
    }

public:
    CCC_CPP20_CONSTEXPR auto operator=(const expected& other) noexcept(
        conjunction_v<std::is_nothrow_copy_constructible<value_type>,
                      std::is_nothrow_copy_constructible<error_type>,
                      std::is_nothrow_copy_assignable<value_type>,
                      std::is_nothrow_copy_assignable<error_type>>)
#ifdef __cpp_concepts
        -> expected&
        requires(std::is_copy_assignable_v<value_type> && std::is_copy_constructible_v<value_type> &&
                 std::is_copy_assignable_v<error_type> && std::is_copy_constructible_v<error_type> &&
                 (std::is_nothrow_move_constructible_v<value_type> || std::is_nothrow_move_constructible_v<error_type>))
#else
        -> enable_if_t<std::is_copy_assignable_v<value_type> && std::is_copy_constructible_v<value_type> &&
                           std::is_copy_assignable_v<error_type> && std::is_copy_constructible_v<error_type> &&
                           (std::is_nothrow_move_constructible_v<value_type> ||
                            std::is_nothrow_move_constructible_v<error_type>),
                       expected&>
#endif
    {
        if (this != std::addressof(other)) {
            if (other.has_value()) {
                assign_value_(other.value_, other.criterion_);
            }
            else {
                assign_error_(other.error_, other.criterion_);
            }
        }

        return *this;
    }
    CCC_CPP20_CONSTEXPR expected& assign(const expected& other) noexcept(
        conjunction_v<std::is_nothrow_copy_constructible<value_type>,
                      std::is_nothrow_copy_constructible<error_type>,
                      std::is_nothrow_copy_assignable<value_type>,
                      std::is_nothrow_copy_assignable<error_type>>)
    {
        return this->operator=(other);
    }

    CCC_CPP20_CONSTEXPR auto operator=(expected&& other) noexcept(
        conjunction_v<std::is_nothrow_move_constructible<value_type>,
                      std::is_nothrow_move_constructible<error_type>,
                      std::is_nothrow_move_assignable<value_type>,
                      std::is_nothrow_move_assignable<error_type>>)
#ifdef __cpp_concepts
        -> expected&
        requires(std::is_move_assignable_v<value_type> && std::is_move_constructible_v<value_type> &&
                 std::is_move_assignable_v<error_type> && std::is_move_constructible_v<error_type> &&
                 (std::is_nothrow_move_constructible_v<value_type> || std::is_nothrow_move_constructible_v<error_type>))
#else
        -> enable_if_t<std::is_move_assignable_v<value_type> && std::is_move_constructible_v<value_type> &&
                           std::is_move_assignable_v<error_type> && std::is_move_constructible_v<error_type> &&
                           (std::is_nothrow_move_constructible_v<value_type> ||
                            std::is_nothrow_move_constructible_v<error_type>),
                       expected&>
#endif
    {
        if (this != std::addressof(other)) {
            if (other.has_value()) {
                assign_value_(std::move(other.value_), std::move(other.criterion_));
            }
            else {
                assign_error_(std::move(other.error_), std::move(other.criterion_));
            }
        }

        return *this;
    }
    CCC_CPP20_CONSTEXPR expected& assign(expected&& other) noexcept(
        conjunction_v<std::is_nothrow_move_constructible<value_type>,
                      std::is_nothrow_move_constructible<error_type>,
                      std::is_nothrow_move_assignable<value_type>,
                      std::is_nothrow_move_assignable<error_type>>)
    {
        return this->operator=(std::move(other));
    }

    template<typename U = remove_cv_t<value_type>,
             typename Cr = remove_cv_t<criterion_type>
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<
                 !is_same_v<expected, remove_cvref_t<U>> && !detail::is_unexpected_v<remove_cvref_t<U>> &&
                 is_constructible_v<value_type, U> && is_assignable_v<value_type&, U> &&
                 !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t> &&
                 is_constructible_v<criterion_type, Cr> && is_assignable_v<criterion_type&, Cr> &&
                 (std::is_nothrow_constructible_v<value_type, U> || std::is_nothrow_move_constructible_v<value_type> ||
                  std::is_nothrow_move_constructible_v<error_type>)>
#endif
             >
#ifdef __cpp_concepts
        requires(!is_same_v<expected, remove_cvref_t<U>> && !detail::is_unexpected_v<remove_cvref_t<U>> &&
                 is_constructible_v<value_type, U> && is_assignable_v<value_type&, U> &&
                 !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t> &&
                 is_constructible_v<criterion_type, Cr> && is_assignable_v<criterion_type&, Cr> &&
                 (std::is_nothrow_constructible_v<value_type, U> || std::is_nothrow_move_constructible_v<value_type> ||
                  std::is_nothrow_move_constructible_v<error_type>))
#endif
    CCC_CPP20_CONSTEXPR expected& assign(U&& v, Cr&& cr = Cr(criterion_type()))
    {
        assign_value_(std::forward<U>(v), std::forward<Cr>(cr));
        return *this;
    }
    template<
        typename U = remove_cv_t<value_type>
#ifndef __cpp_concepts
        ,
        typename = enable_if_t<!is_same_v<expected, remove_cvref_t<U>> && !detail::is_unexpected_v<remove_cvref_t<U>>>
#endif
        >
#ifdef __cpp_concepts
        requires(!is_same_v<expected, remove_cvref_t<U>> && !detail::is_unexpected_v<remove_cvref_t<U>>)
#endif
    CCC_CPP20_CONSTEXPR expected& operator=(U&& v)
    {
        return this->assign(std::forward<U>(v), criterion_type());  // NOLINT(*-unconventional-assign-operator)
    }

    template<typename G,
             typename Cr = remove_cv_t<criterion_type>
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<
                 is_constructible_v<error_type, const G&> && is_assignable_v<error_type&, const G&> &&
                 (is_nothrow_constructible_v<error_type, const G&> || is_nothrow_move_constructible_v<value_type> ||
                  is_nothrow_move_constructible_v<error_type>) &&
                 !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t> &&
                 is_constructible_v<criterion_type, const Cr&> && is_assignable_v<criterion_type&, const Cr&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, const G&> && is_assignable_v<error_type&, const G&> &&
                 (is_nothrow_constructible_v<error_type, const G&> ||
                  std::is_nothrow_move_constructible_v<value_type> ||
                  std::is_nothrow_move_constructible_v<error_type>) &&
                 !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t> &&
                 is_constructible_v<criterion_type, const Cr&> && is_assignable_v<criterion_type&, const Cr&>)
#endif
    CCC_CPP20_CONSTEXPR expected& assign(const unexpected<G>& e, const Cr& cr = Cr(criterion_type::default_error_value))
    {
        assign_error_(e.error(), cr);
        return *this;
    }
    template<typename G
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<
                 is_constructible_v<error_type, const G&> && is_assignable_v<error_type&, const G&> &&
                 (is_nothrow_constructible_v<error_type, const G&> || is_nothrow_move_constructible_v<value_type> ||
                  is_nothrow_move_constructible_v<error_type>)>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, const G&> && is_assignable_v<error_type&, const G&> &&
                 (is_nothrow_constructible_v<error_type, const G&> || is_nothrow_move_constructible_v<value_type> ||
                  is_nothrow_move_constructible_v<error_type>))
#endif
    CCC_CPP20_CONSTEXPR expected& operator=(const unexpected<G>& e)
    {
        return this->assign(e);  // NOLINT(*-unconventional-assign-operator)
    }

    template<typename G,
             typename Cr = remove_cv_t<criterion_type>
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<
                 is_constructible_v<error_type, G> && is_assignable_v<error_type&, G> &&
                 (is_nothrow_constructible_v<error_type, G> || is_nothrow_move_constructible_v<value_type> ||
                  is_nothrow_move_constructible_v<error_type>) &&
                 !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t> &&
                 is_constructible_v<criterion_type, Cr> && is_assignable_v<criterion_type&, Cr>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, G> && is_assignable_v<error_type&, G> &&
                 (is_nothrow_constructible_v<error_type, G> || is_nothrow_move_constructible_v<value_type> ||
                  is_nothrow_move_constructible_v<error_type>) &&
                 !is_same_v<remove_cvref<Cr>, in_place_t> && !is_same_v<remove_cvref_t<Cr>, unexpect_t> &&
                 is_constructible_v<criterion_type, Cr> && is_assignable_v<criterion_type&, Cr>)
#endif
    CCC_CPP20_CONSTEXPR expected& assign(unexpected<G>&& e, Cr&& cr = Cr(criterion_type::default_error_value))
    {
        assign_error_(std::move(e.error()), std::forward<Cr>(cr));
        return *this;
    }
    template<typename G
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<error_type, G> && is_assignable_v<error_type&, G> &&
                                    (is_nothrow_constructible_v<error_type, G> ||
                                     is_nothrow_move_constructible_v<value_type> ||
                                     is_nothrow_move_constructible_v<error_type>)>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, G> && is_assignable_v<error_type&, G> &&
                 (is_nothrow_constructible_v<error_type, G> || is_nothrow_move_constructible_v<value_type> ||
                  is_nothrow_move_constructible_v<error_type>))
#endif
    CCC_CPP20_CONSTEXPR expected& operator=(unexpected<G>&& e)
    {
        return this->assign(std::move(e));  // NOLINT(*-unconventional-assign-operator)
    }

public:  // observer
    constexpr const value_type* operator->() const noexcept
    {
        return std::addressof(value_);
    }
    constexpr value_type* operator->() noexcept
    {
        return std::addressof(value_);
    }

    constexpr const value_type& operator*() const& noexcept
    {
        return value_;
    }
    constexpr value_type& operator*() & noexcept
    {
        return value_;
    }
    constexpr value_type&& operator*() && noexcept
    {
        return std::move(value_);
    }
    constexpr const value_type&& operator*() const&& noexcept
    {
        return std::move(value_);
    }

    constexpr const error_type& error() const& noexcept
    {
        return error_;
    }
    constexpr error_type& error() & noexcept
    {
        return error_;
    }
    constexpr error_type&& error() && noexcept
    {
        return std::move(error_);
    }
    constexpr const error_type&& error() const&& noexcept
    {
        return std::move(error_);
    }

    constexpr const value_type& value() const&
    {
        static_assert(is_copy_constructible_v<error_type>, "error_type must be copy constructible");
        if (!has_value()) {
            throw bad_expected_access<error_type>(ccc::as_const(error_));
        }
        return value_;
    }
    constexpr value_type& value() &
    {
        static_assert(is_copy_constructible_v<error_type>, "error_type must be copy constructible");
        if (!has_value()) {
            throw bad_expected_access<error_type>(ccc::as_const(error_));
        }
        return value_;
    }
    constexpr value_type&& value() &&
    {
        static_assert(is_copy_constructible_v<error_type>, "error_type must be copy constructible");
        static_assert(is_constructible_v<error_type, error_type&&>,
                      "error_type must be constructible from rvalue reference of itself");
        if (!has_value()) {
            throw bad_expected_access<error_type>(std::move(error_));
        }
        return std::move(value_);
    }
    constexpr const value_type&& value() const&&
    {
        static_assert(is_copy_constructible_v<error_type>, "error_type must be copy constructible");
        static_assert(is_constructible_v<error_type, const error_type&&>,
                      "error_type must be constructible from rvalue reference of itself");
        if (!has_value()) {
            throw bad_expected_access<error_type>(std::move(error_));
        }
        return std::move(value_);
    }

    template<typename U = remove_cv_t<value_type>>
    constexpr value_type value_or(U&& v) const& noexcept(is_nothrow_copy_constructible_v<value_type> &&
                                                         is_nothrow_convertible_v<U, value_type>)
    {
        static_assert(is_copy_constructible_v<value_type>, "value_type must be copy constructible");
        static_assert(is_convertible_v<U, value_type>, "U must be convertible to value_type");

        if (has_value()) {
            return value_;
        }
        return static_cast<value_type>(std::forward<U>(v));
    }

    template<typename U = remove_cv_t<value_type>>
    constexpr value_type value_or(U&& v) && noexcept(
        conjunction_v<std::is_nothrow_move_constructible<value_type>, is_nothrow_convertible<U, value_type>>)
    {
        static_assert(is_move_constructible_v<value_type>, "value_type must be move constructible");
        static_assert(is_convertible_v<U, value_type>, "U must be convertible to value_type");

        if (has_value()) {
            return std::move(value_);
        }
        return static_cast<value_type>(std::forward<U>(v));
    }

    template<typename G = error_type>
    constexpr error_type error_or(G&& e) const&
    {
        static_assert(is_copy_constructible_v<error_type>, "error_type must be copy constructible");
        static_assert(is_convertible_v<G, error_type>, "G must be convertible to error_type");

        if (has_value()) {
            return std::forward<G>(e);
        }
        return error_;
    }

    template<typename G = error_type>
    constexpr error_type error_or(G&& e) &&
    {
        static_assert(is_move_constructible_v<error_type>, "error_type must be move constructible");
        static_assert(is_convertible_v<G, error_type>, "G must be convertible to error_type");

        if (has_value()) {
            return std::forward<G>(e);
        }
        return std::move(error_);
    }

private:  // swap
    CCC_CPP20_CONSTEXPR void this_value_swap_with_other_error_(expected& other)
    {
        auto& self = *this;
        if CCC_CPP17_CONSTEXPR (is_nothrow_move_constructible_v<error_type>) {
            detail::expected_impl::Guard<error_type> guard(other.error_);

            construct_at(std::addressof(other.value_), std::move(self.value_));
            destroy_at(std::addressof(self.value_));
            construct_at(std::addressof(self.error_), guard.release());
        }
        else {
            detail::expected_impl::Guard<value_type> guard(self.value_);

            construct_at(std::addressof(self.error_), std::move(other.error_));
            destroy_at(std::addressof(other.error_));
            construct_at(std::addressof(other.value_), guard.release());
        }
    }

public:
    CCC_CPP20_CONSTEXPR auto swap(expected& other) noexcept(is_nothrow_move_constructible_v<value_type> &&
                                                            is_nothrow_swappable_v<value_type> &&
                                                            is_nothrow_move_constructible_v<error_type> &&
                                                            is_nothrow_swappable_v<error_type>)
#ifdef __cpp_concepts
        -> void
        requires(is_swappable_v<value_type> && is_swappable_v<error_type> && is_move_constructible_v<value_type> &&
                 is_move_constructible_v<error_type> &&
                 (is_nothrow_move_constructible_v<value_type> || is_nothrow_move_constructible_v<error_type>))
#else
        -> enable_if_t<is_swappable_v<value_type> && is_swappable_v<error_type> &&
                           is_move_constructible_v<value_type> && is_move_constructible_v<error_type> &&
                           (is_nothrow_move_constructible_v<value_type> || is_nothrow_move_constructible_v<error_type>),
                       void>
#endif
    {
        using std::swap;
        auto& self = *this;

        if (self.has_value()) {
            if (other.has_value()) {
                swap(self.value_, other.value_);
            }
            else {
                self.this_value_swap_with_other_error_(other);
            }
        }
        else {
            if (other.has_value()) {
                other.this_value_swap_with_other_error_(self);
            }
            else {
                swap(self.error_, other.error_);
            }
        }

        if (self.criterion_ != other.criterion_) {
            swap(self.criterion_, other.criterion_);
        }
    }

    friend CCC_CPP20_CONSTEXPR auto swap(expected& lhs, expected& rhs) noexcept(noexcept(lhs.swap(rhs)))
#ifdef __cpp_concepts
        -> void
        requires(requires { lhs.swap(rhs); })
#else
        -> void_t<decltype(lhs.swap(rhs))>
#endif
    {
        lhs.swap(rhs);
    }

private:  // emplace
    CCC_CPP20_CONSTEXPR void emplace_preprocess_()
    {
        if (has_value()) {
            destroy_at(std::addressof(value_));
        }
        else {
            destroy_at(std::addressof(error_));
        }
    }

public:
    template<typename... Args
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_nothrow_constructible_v<value_type, Args...> &&
                                    is_nothrow_constructible_v<criterion_type, criterion_type> &&
                                    is_nothrow_assignable_v<criterion_type&, criterion_type>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_nothrow_constructible_v<value_type, Args...> &&
                 is_nothrow_constructible_v<criterion_type, criterion_type> &&
                 is_nothrow_assignable_v<criterion_type&, criterion_type>)
#endif
    CCC_CPP20_CONSTEXPR value_type& emplace(Args&&... args) noexcept
    {
        emplace_preprocess_();
        criterion_ = criterion_type();
        return *construct_at(std::addressof(value_), std::forward<Args>(args)...);
    }
    template<typename U,
             typename... Args
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_nothrow_constructible_v<value_type, std::initializer_list<U>&, Args...> &&
                                    is_nothrow_constructible_v<criterion_type, criterion_type> &&
                                    is_nothrow_assignable_v<criterion_type&, criterion_type>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_nothrow_constructible_v<value_type, std::initializer_list<U>&, Args...> &&
                 is_nothrow_constructible_v<criterion_type, criterion_type> &&
                 is_nothrow_assignable_v<criterion_type&, criterion_type>)
#endif
    CCC_CPP20_CONSTEXPR value_type& emplace(std::initializer_list<U> il, Args&&... args) noexcept
    {
        emplace_preprocess_();
        criterion_ = criterion_type();
        return *ccc::construct_at(std::addressof(value_), il, std::forward<Args>(args)...);
    }

    template<typename Cr,
             typename... Args
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_nothrow_constructible_v<value_type, Args...> &&
                                    is_nothrow_constructible_v<criterion_type, Cr> &&
                                    is_nothrow_assignable_v<criterion_type&, Cr>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_nothrow_constructible_v<value_type, Args...> && is_nothrow_constructible_v<criterion_type, Cr> &&
                 is_nothrow_assignable_v<criterion_type&, Cr>)
#endif
    CCC_CPP20_CONSTEXPR value_type& emplace_with_criterion(Cr&& cr, Args&&... args) noexcept
    {
        emplace_preprocess_();
        criterion_ = std::forward<Cr>(cr);
        return *construct_at(std::addressof(value_), std::forward<Args>(args)...);
    }
    template<typename Cr,
             typename U,
             typename... Args
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_nothrow_constructible_v<value_type, std::initializer_list<U>&, Args...> &&
                                    is_nothrow_constructible_v<criterion_type, Cr> &&
                                    is_nothrow_assignable_v<criterion_type&, Cr>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_nothrow_constructible_v<value_type, std::initializer_list<U>&, Args...> &&
                 is_nothrow_constructible_v<criterion_type, Cr> && is_nothrow_assignable_v<criterion_type&, Cr>)
#endif
    CCC_CPP20_CONSTEXPR value_type& emplace_with_criterion(Cr&& cr,
                                                           std::initializer_list<U> il,
                                                           Args&&... args) noexcept
    {
        emplace_preprocess_();
        criterion_ = std::forward<Cr>(cr);
        return *construct_at(std::addressof(value_), il, std::forward<Args>(args)...);
    }

public:  // operator==
    template<typename T2, typename E2>
    constexpr auto operator==(const expected<T2, E2>& rhs) const
#ifdef __cpp_concepts
        -> bool
        requires(!is_void_v<T2> && requires(const expected& l, const expected<T2, E2>& r) {
            { *l == *r } -> std::convertible_to<bool>;
            { l.error() == r.error() } -> std::convertible_to<bool>;
        })
#else
        -> enable_if_t<!is_void_v<T2> &&
                           is_void_v<void_t<decltype(**this == *rhs), decltype(this->error() == rhs.error())>> &&
                           is_convertible_v<decltype(**this == *rhs), bool> &&
                           is_convertible_v<decltype(this->error() == rhs.error()), bool>,
                       bool>
#endif
    {
        const auto& lhs = *this;
        if (lhs.has_value() != rhs.has_value()) {
            return false;
        }
        return lhs.has_value() ? *lhs == *rhs : lhs.error() == rhs.error();
    }

    template<typename E2>
    constexpr auto operator==(const unexpected<E2>& unexp) const
#ifdef __cpp_concepts
        -> bool
        requires(requires(const expected& e, const unexpected<E2>& u) {
            { e.error() == u.error() } -> std::convertible_to<bool>;
        })
#else
        -> enable_if_t<is_void_v<void_t<decltype(this->error() == unexp.error())>> &&
                           is_convertible_v<decltype(this->error() == unexp.error()), bool>,
                       bool>
#endif
    {
        const auto& lhs = *this;
        return !lhs.has_value() && lhs.error() == unexp.error();
    }

    template<typename V>
    constexpr auto operator==(const V& val) const
#ifdef __cpp_concepts
        -> bool
        requires(!detail::is_expected_v<remove_cvref<V>> && requires(const expected& e, const V& val) {
            { *e == val } -> std::convertible_to<bool>;
        })
#else
        -> enable_if_t<!detail::is_expected_v<remove_cvref<V>> && is_void_v<void_t<decltype(**this == val)>> &&
                           is_convertible_v<decltype(**this == val), bool>,
                       bool>
#endif
    {
        const auto& lhs = *this;
        return lhs.has_value() && *lhs == val;
    }

#if (__cplusplus >= 201703L)
private:  // Monadic operations
    template<typename Func>
    explicit constexpr expected(detail::expected_impl::in_place_invoke_tag,
                                detail::expected_impl::transform_func_return_2values_tag,
                                Func&& func)
        : value_()
    {
        auto [value, criterion] = std::invoke(std::forward<Func>(func));
        value_ = std::move_if_noexcept(value);
        criterion_ = std::move_if_noexcept(criterion);
    }
    template<typename Func>
    explicit constexpr expected(detail::expected_impl::in_place_invoke_tag, Func&& func)
        : value_(std::invoke(std::forward<Func>(func))), criterion_()
    {
    }

    template<typename Func>
    explicit constexpr expected(detail::expected_impl::unexpect_invoke_tag,
                                detail::expected_impl::transform_func_return_2values_tag,
                                Func&& func)
        : error_()
    {
        auto [error, criterion] = std::invoke(std::forward<Func>(func));
        error_ = std::move_if_noexcept(error);
        criterion_ = std::move_if_noexcept(criterion);
    }
    template<typename Func>
    explicit constexpr expected(detail::expected_impl::unexpect_invoke_tag, Func&& func)
        : error_(std::invoke(std::forward<Func>(func))), criterion_(criterion_type::default_error_value)
    {
    }

public:
    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<error_type, error_type&> &&
                                    is_constructible_v<criterion_type, criterion_type&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, error_type&> && is_constructible_v<criterion_type, criterion_type&>)
#endif
    CCC_CPP20_CONSTEXPR auto and_then(Func&& func) &
    {
        using FuncResult = detail::expected_impl::result<Func, value_type&, criterion_type&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected");
        static_assert(is_same_v<error_type, typename FuncResult::error_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same error_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return std::invoke(std::forward<Func>(func), value_, criterion_);
        }
        return FuncResult(criterion_, unexpect, error_);
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<error_type, const error_type&> &&
                                    is_constructible_v<criterion_type, const criterion_type&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, const error_type&> &&
                 is_constructible_v<criterion_type, const criterion_type&>)
#endif
    CCC_CPP20_CONSTEXPR auto and_then(Func&& func) const&
    {
        using FuncResult = detail::expected_impl::result<Func, const value_type&, const criterion_type&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected");
        static_assert(is_same_v<error_type, typename FuncResult::error_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same error_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return std::invoke(std::forward<Func>(func), value_, criterion_);
        }
        return FuncResult(criterion_, unexpect, error_);
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<error_type, error_type&&> &&
                                    is_constructible_v<criterion_type, criterion_type&&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, error_type &&> && is_constructible_v<criterion_type, criterion_type &&>)
#endif
    CCC_CPP20_CONSTEXPR auto and_then(Func&& func) &&
    {
        using FuncResult = detail::expected_impl::result<Func, value_type&&, criterion_type&&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected");
        static_assert(is_same_v<error_type, typename FuncResult::error_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same error_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return std::invoke(std::forward<Func>(func), std::move(value_), std::move(criterion_));
        }
        return FuncResult(std::move(criterion_), unexpect, std::move(error_));
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<error_type, const error_type&&> &&
                                    is_constructible_v<criterion_type, const criterion_type&&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<error_type, const error_type &&> &&
                 is_constructible_v<criterion_type, const criterion_type &&>)
#endif
    CCC_CPP20_CONSTEXPR auto and_then(Func&& func) const&&
    {
        using FuncResult = detail::expected_impl::result<Func, const value_type&&, const criterion_type&&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected");
        static_assert(is_same_v<error_type, typename FuncResult::error_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same error_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::and_then must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return std::invoke(std::forward<Func>(func), std::move(value_), std::move(criterion_));
        }
        return FuncResult(std::move(criterion_), unexpect, std::move(error_));
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<value_type, value_type&> &&
                                    is_constructible_v<criterion_type, criterion_type&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<value_type, value_type&> && is_constructible_v<criterion_type, criterion_type&>)
#endif
    CCC_CPP20_CONSTEXPR auto or_else(Func&& func) &
    {
        using FuncResult = detail::expected_impl::result<Func, error_type&, criterion_type&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected");
        static_assert(is_same_v<value_type, typename FuncResult::value_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same value_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return FuncResult(criterion_, in_place, value_);
        }
        return std::invoke(std::forward<Func>(func), error_, criterion_);
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<value_type, const value_type&> &&
                                    is_constructible_v<criterion_type, const criterion_type&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<value_type, const value_type&> &&
                 is_constructible_v<criterion_type, const criterion_type&>)
#endif
    CCC_CPP20_CONSTEXPR auto or_else(Func&& func) const&
    {
        using FuncResult = detail::expected_impl::result<Func, const error_type&, const criterion_type&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected");
        static_assert(is_same_v<value_type, typename FuncResult::value_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same value_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return FuncResult(criterion_, in_place, value_);
        }
        return std::invoke(std::forward<Func>(func), error_, criterion_);
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<value_type, value_type&&> &&
                                    is_constructible_v<criterion_type, criterion_type&&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<value_type, value_type &&> && is_constructible_v<criterion_type, criterion_type &&>)
#endif
    CCC_CPP20_CONSTEXPR auto or_else(Func&& func) &&
    {
        using FuncResult = detail::expected_impl::result<Func, error_type&&, criterion_type&&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected");
        static_assert(is_same_v<value_type, typename FuncResult::value_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same value_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return FuncResult(std::move(criterion_), in_place, std::move(value_));
        }
        return std::invoke(std::forward<Func>(func), std::move(error_), std::move(criterion_));
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_constructible_v<value_type, const value_type&&> &&
                                    is_constructible_v<criterion_type, const criterion_type&&>>
#endif
             >
#ifdef __cpp_concepts
        requires(is_constructible_v<value_type, const value_type &&> &&
                 is_constructible_v<criterion_type, const criterion_type &&>)
#endif
    CCC_CPP20_CONSTEXPR auto or_else(Func&& func) const&&
    {
        using FuncResult = detail::expected_impl::result<Func, const error_type&&, const criterion_type&&>;
        static_assert(detail::is_expected_v<FuncResult>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected");
        static_assert(is_same_v<value_type, typename FuncResult::value_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same value_type");
        static_assert(is_same_v<criterion_type, typename FuncResult::criterion_type>,
                      "The function passed to ccc::expected<T, E, Criterion>::or_else must return a ccc::expected "
                      "with the same criterion_type");

        if (has_value()) {
            return FuncResult(std::move(criterion_), in_place, std::move(value_));
        }
        return std::invoke(std::forward<Func>(func), std::move(error_), std::move(criterion_));
    }

    template<
        typename Func
#ifndef __cpp_concepts
        ,
        typename = enable_if_t<is_default_constructible_v<value_type> && is_constructible_v<error_type, error_type&> &&
                               is_constructible_v<criterion_type, criterion_type&> && (sizeof(Func) > 0)>
#endif
        >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<value_type> && is_constructible_v<error_type, error_type&> &&
                 is_constructible_v<criterion_type, criterion_type&>)
#endif
    constexpr auto transform(Func&& func) &
    {
        using U = detail::expected_impl::result_xform<Func, value_type&, criterion_type&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Value = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<Value, error_type, CriterionType>;
            if (has_value()) {
                return Result(detail::expected_impl::in_place_invoke,
                              detail::expected_impl::transform_func_return_2_values,
                              [&] { return std::invoke(std::forward<Func>(func), value_, criterion_); });
            }
            return Result(criterion_, unexpect, error_);
        }
        else {
            using Result = expected<U, error_type, criterion_type>;
            if (has_value()) {
                return Result(detail::expected_impl::in_place_invoke,
                              [&] { return std::invoke(std::forward<Func>(func), value_, criterion_); });
            }
            return Result(criterion_, unexpect, error_);
        }
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_default_constructible_v<value_type> &&
                                    is_constructible_v<error_type, const error_type&> &&
                                    is_constructible_v<criterion_type, const criterion_type&> && (sizeof(Func) > 0)>
#endif
             >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<value_type> && is_constructible_v<error_type, const error_type&> &&
                 is_constructible_v<criterion_type, const criterion_type&>)
#endif
    constexpr auto transform(Func&& func) const&
    {
        using U = detail::expected_impl::result_xform<Func, const value_type&, const criterion_type&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Value = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<Value, error_type, CriterionType>;
            if (has_value()) {
                return Result(detail::expected_impl::in_place_invoke,
                              detail::expected_impl::transform_func_return_2_values,
                              [&] { return std::invoke(std::forward<Func>(func), value_, criterion_); });
            }
            return Result(criterion_, unexpect, error_);
        }
        else {
            using Result = expected<U, error_type, criterion_type>;
            if (has_value()) {
                return Result(detail::expected_impl::in_place_invoke,
                              [&] { return std::invoke(std::forward<Func>(func), value_, criterion_); });
            }
            return Result(criterion_, unexpect, error_);
        }
    }

    // Why there requires `(sizeof(Func) > 0)` in enable_if_t? It may be a bug in MinGW:
    // If without `(sizeof(Func) > 0)`, the MinGW compiler will report that substitution failure
    // in `transform`/`transform_error` function template even if this template have not be instantiated.
    //
    // So we use `(sizeof(Func) > 0)` in enable_if_t(But not use it in requires statement, because it
    // works on successfully with requires statement).
    template<
        typename Func
#ifndef __cpp_concepts
        ,
        typename = enable_if_t<is_default_constructible_v<value_type> && is_constructible_v<error_type, error_type&&> &&
                               is_constructible_v<criterion_type, criterion_type&&> && (sizeof(Func) > 0)>
#endif
        >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<value_type> && is_constructible_v<error_type, error_type &&> &&
                 is_constructible_v<criterion_type, criterion_type &&>)
#endif
    constexpr auto transform(Func&& func) &&
    {
        using U = detail::expected_impl::result_xform<Func, value_type&&, criterion_type&&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Value = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<Value, error_type, CriterionType>;
            if (has_value()) {
                return Result(
                    detail::expected_impl::in_place_invoke,
                    detail::expected_impl::transform_func_return_2_values,
                    [&] { return std::invoke(std::forward<Func>(func), std::move(value_), std::move(criterion_)); });
            }
            return Result(std::move(criterion_), unexpect, std::move(error_));
        }
        else {
            using Result = expected<U, error_type, criterion_type>;
            if (has_value()) {
                return Result(detail::expected_impl::in_place_invoke, [&] {
                    return std::invoke(std::forward<Func>(func), std::move(value_), std::move(criterion_));
                });
            }
            return Result(std::move(criterion_), unexpect, std::move(error_));
        }
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_default_constructible_v<value_type> &&
                                    is_constructible_v<error_type, const error_type&&> &&
                                    is_constructible_v<criterion_type, const criterion_type&&> && (sizeof(Func) > 0)>
#endif
             >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<value_type> && is_constructible_v<error_type, const error_type &&> &&
                 is_constructible_v<criterion_type, const criterion_type &&>)
#endif
    constexpr auto transform(Func&& func) const&&
    {
        using U = detail::expected_impl::result_xform<Func, const value_type&&, const criterion_type&&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Value = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<Value, error_type, CriterionType>;
            if (has_value()) {
                return Result(
                    detail::expected_impl::in_place_invoke,
                    detail::expected_impl::transform_func_return_2_values,
                    [&] { return std::invoke(std::forward<Func>(func), std::move(value_), std::move(criterion_)); });
            }
            return Result(std::move(criterion_), unexpect, std::move(error_));
        }
        else {
            using Result = expected<U, error_type, criterion_type>;
            if (has_value()) {
                return Result(detail::expected_impl::in_place_invoke, [&] {
                    return std::invoke(std::forward<Func>(func), std::move(value_), std::move(criterion_));
                });
            }
            return Result(std::move(criterion_), unexpect, std::move(error_));
        }
    }

    template<
        typename Func
#ifndef __cpp_concepts
        ,
        typename = enable_if_t<is_default_constructible_v<error_type> && is_constructible_v<error_type, error_type&> &&
                               is_constructible_v<criterion_type, criterion_type&> && (sizeof(Func) > 0)>
#endif
        >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<error_type> && is_constructible_v<error_type, error_type&> &&
                 is_constructible_v<criterion_type, criterion_type&>)
#endif
    constexpr auto transform_error(Func&& func) &
    {
        using U = detail::expected_impl::result_xform<Func, error_type&, criterion_type&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Err = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<value_type, Err, CriterionType>;
            if (has_value()) {
                return Result(criterion_, in_place, value_);
            }
            return Result(detail::expected_impl::unexpect_invoke,
                          detail::expected_impl::transform_func_return_2_values,
                          [&] { return std::invoke(std::forward<Func>(func), error_, criterion_); });
        }
        else {
            using Result = expected<value_type, U, criterion_type>;
            if (has_value()) {
                return Result(criterion_, in_place, value_);
            }
            return Result(detail::expected_impl::unexpect_invoke,
                          [&] { return std::invoke(std::forward<Func>(func), error_, criterion_); });
        }
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_default_constructible_v<error_type> &&
                                    is_constructible_v<error_type, const error_type&> &&
                                    is_constructible_v<criterion_type, const criterion_type&> && (sizeof(Func) > 0)>
#endif
             >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<error_type> && is_constructible_v<error_type, const error_type&> &&
                 is_constructible_v<criterion_type, const criterion_type&>)
#endif
    constexpr auto transform_error(Func&& func) const&
    {
        using U = detail::expected_impl::result_xform<Func, const error_type&, const criterion_type&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Err = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<value_type, Err, CriterionType>;
            if (has_value()) {
                return Result(criterion_, in_place, value_);
            }
            return Result(detail::expected_impl::unexpect_invoke,
                          detail::expected_impl::transform_func_return_2_values,
                          [&] { return std::invoke(std::forward<Func>(func), error_, criterion_); });
        }
        else {
            using Result = expected<value_type, U, criterion_type>;
            if (has_value()) {
                return Result(criterion_, in_place, value_);
            }
            return Result(detail::expected_impl::unexpect_invoke,
                          [&] { return std::invoke(std::forward<Func>(func), error_, criterion_); });
        }
    }

    template<
        typename Func
#ifndef __cpp_concepts
        ,
        typename = enable_if_t<is_default_constructible_v<error_type> && is_constructible_v<error_type, error_type&&> &&
                               is_constructible_v<criterion_type, criterion_type&&> && (sizeof(Func) > 0)>
#endif
        >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<error_type> && is_constructible_v<error_type, error_type &&> &&
                 is_constructible_v<criterion_type, criterion_type &&>)
#endif
    constexpr auto transform_error(Func&& func) &&
    {
        using U = detail::expected_impl::result_xform<Func, error_type&&, criterion_type&&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Err = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<value_type, Err, CriterionType>;
            if (has_value()) {
                return Result(std::move(criterion_), in_place, std::move(value_));
            }
            return Result(
                detail::expected_impl::unexpect_invoke,
                detail::expected_impl::transform_func_return_2_values,
                [&] { return std::invoke(std::forward<Func>(func), std::move(error_), std::move(criterion_)); });
        }
        else {
            using Result = expected<value_type, U, criterion_type>;
            if (has_value()) {
                return Result(std::move(criterion_), in_place, std::move(value_));
            }
            return Result(detail::expected_impl::unexpect_invoke, [&] {
                return std::invoke(std::forward<Func>(func), std::move(error_), std::move(criterion_));
            });
        }
    }

    template<typename Func
#ifndef __cpp_concepts
             ,
             typename = enable_if_t<is_default_constructible_v<error_type> &&
                                    is_constructible_v<error_type, const error_type&&> &&
                                    is_constructible_v<criterion_type, const criterion_type&&> && (sizeof(Func) > 0)>
#endif
             >
#ifdef __cpp_concepts
        requires(is_default_constructible_v<error_type> && is_constructible_v<error_type, const error_type &&> &&
                 is_constructible_v<criterion_type, const criterion_type &&>)
#endif
    constexpr auto transform_error(Func&& func) const&&
    {
        using U = detail::expected_impl::result_xform<Func, const error_type&&, const criterion_type&&>;

        if constexpr (detail::expected_impl::is_transform_func_return_2values_v<remove_cvref_t<U>>) {
            using Err = tuple_element_t<0, U>;
            using CriterionType = tuple_element_t<1, U>;

            using Result = expected<value_type, Err, CriterionType>;
            if (has_value()) {
                return Result(std::move(criterion_), in_place, std::move(value_));
            }
            return Result(
                detail::expected_impl::unexpect_invoke,
                detail::expected_impl::transform_func_return_2_values,
                [&] { return std::invoke(std::forward<Func>(func), std::move(error_), std::move(criterion_)); });
        }
        else {
            using Result = expected<value_type, U, criterion_type>;
            if (has_value()) {
                return Result(std::move(criterion_), in_place, std::move(value_));
            }
            return Result(detail::expected_impl::unexpect_invoke, [&] {
                return std::invoke(std::forward<Func>(func), std::move(error_), std::move(criterion_));
            });
        }
    }
#endif  // (__cplusplus >= 201703L)
};

}  // namespace ccc

#endif  // !CCC_EXPECTED_EXPECTED_IMPL_HH
