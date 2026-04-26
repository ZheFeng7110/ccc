/**
 * @file unexpected.hh
 * @brief
 */

// ReSharper disable CppRedundantInlineSpecifier
// ReSharper disable CppUseTypeTraitAlias

#pragma once
#ifndef CCC_EXPECTED_UNEXPECTED_HH
#define CCC_EXPECTED_UNEXPECTED_HH

#include "ccc/expected/base.hh"

namespace ccc {

CCC_MODULE_EXPORT_BEGIN

template<typename E>
class unexpected
{
public:
    static_assert(!std::is_void<E>::value, "E must not be void");

    unexpected() = delete;
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;

    template<typename Err = E,
             typename std::enable_if<!std::is_same<Err, unexpected>::value && !std::is_same<Err, in_place_t>::value &&
                                     std::is_constructible<E, Err>::value>::type* = nullptr>
    explicit constexpr unexpected(Err&& e) : val_(std::forward<Err>(e))  // NOLINT(*-forwarding-reference-overload)
    {
    }

    template<typename... Args, typename std::enable_if<std::is_constructible<E, Args&&...>::value>::type* = nullptr>
    explicit constexpr unexpected(in_place_t, Args&&... args) : val_(std::forward<Args>(args)...)
    {
    }
    template<
        typename U,
        typename... Args,
        typename std::enable_if<std::is_constructible<E, std::initializer_list<U>&, Args&&...>::value>::type* = nullptr>
    explicit constexpr unexpected(in_place_t, std::initializer_list<U> l, Args&&... args)
        : val_(l, std::forward<Args>(args)...)
    {
    }

    constexpr const E& error() const& noexcept
    {
        return val_;
    }
    constexpr E& error() & noexcept
    {
        return val_;
    }
    constexpr E&& error() && noexcept
    {
        return std::move(val_);
    }
    constexpr const E&& error() const&& noexcept
    {
        return std::move(val_);
    }

    constexpr void swap(unexpected& other) noexcept(std::is_nothrow_swappable<E>::value)
    {
        using std::swap;
        swap(val_, other.val_);
    }
    friend constexpr void swap(unexpected& lhs, unexpected& rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

private:
    E val_;
};

#ifdef __cpp_deduction_guides
template<typename E>
unexpected(E) -> unexpected<E>;
#endif

template<typename E>
inline constexpr bool operator==(const unexpected<E>& lhs, const unexpected<E>& rhs)
{
    return lhs.error() == rhs.error();
}
#if (__cplusplus >= 202002L)
template<typename E>
inline constexpr auto operator<=>(const unexpected<E>& lhs, const unexpected<E>& rhs)
{
    return lhs.error() <=> rhs.error();
}
#else
template<typename E>
inline constexpr bool operator!=(const unexpected<E>& lhs, const unexpected<E>& rhs)
{
    return lhs.error() != rhs.error();
}
template<typename E>
inline constexpr bool operator<(const unexpected<E>& lhs, const unexpected<E>& rhs)
{
    return lhs.error() < rhs.error();
}
template<typename E>
inline constexpr bool operator<=(const unexpected<E>& lhs, const unexpected<E>& rhs)
{
    return lhs.error() <= rhs.error();
}
template<typename E>
inline constexpr bool operator>(const unexpected<E>& lhs, const unexpected<E>& rhs)
{
    return lhs.error() > rhs.error();
}
template<typename E>
inline constexpr bool operator>=(const unexpected<E>& lhs, const unexpected<E>& rhs)
{
    return lhs.error() >= rhs.error();
}
#endif

struct unexpect_t {
    unexpect_t() = default;
};
constexpr unexpect_t unexpect{};

CCC_MODULE_EXPORT_END

namespace detail {

template<typename>
struct is_unexpected : std::false_type {
};

template<typename E>
struct is_unexpected<unexpected<E>> : std::true_type {
};

template<typename T>
constexpr bool is_unexpected_v = is_unexpected<T>::value;

template<typename E>
constexpr bool can_be_unexpected_v = std::is_object<E>::value && (!std::is_array<E>::value) && (!is_unexpected_v<E>) &&
                                     (!std::is_const<E>::value) && (!std::is_volatile<E>::value);

}  // namespace detail

}  // namespace ccc

#endif  // !CCC_EXPECTED_UNEXPECTED_HH
