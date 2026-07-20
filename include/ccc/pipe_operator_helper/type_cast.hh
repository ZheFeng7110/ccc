/**
 * @file type_cast.hh
 * @brief The implementations for type-cast operators
 */

// ReSharper disable CppRedundantInlineSpecifier

#pragma once
#ifndef CPP_PIPE_OPERATOR_HELPER_TYPE_CAST_HH
#define CPP_PIPE_OPERATOR_HELPER_TYPE_CAST_HH

#include "ccc/pipe_operator_helper/detail/config.hh"
#include "ccc/pipe_operator_helper/detail/bit_cast_impl.hh"

#include <utility>
#include <type_traits>

namespace ccc {
namespace pipe_operator_helper {

namespace detail {

template<typename ToType>
struct static_cast_to_tag {
    using to_type = ToType;
};

template<typename ToType>
struct dynamic_cast_to_tag {
    using to_type = ToType;
};

template<typename ToType>
struct const_cast_to_tag {
    using to_type = ToType;
};

template<typename ToType>
struct reinterpret_cast_to_tag {
    using to_type = ToType;
};

template<typename ToType>
struct bit_cast_to_tag {
    using to_type = ToType;
};

struct move_to_tag {
};

template<typename ToType>
struct forward_to_tag {
    using to_type = ToType;
};

}  // namespace detail

template<typename ToType>
inline constexpr detail::static_cast_to_tag<ToType> static_cast_to() noexcept
{
    return detail::static_cast_to_tag<ToType>{};
}

template<typename ToType>
inline constexpr detail::static_cast_to_tag<ToType> cast_to() noexcept
{
    return detail::static_cast_to_tag<ToType>{};
}

template<typename ToType>
inline detail::dynamic_cast_to_tag<ToType> dynamic_cast_to() noexcept
{
    return detail::dynamic_cast_to_tag<ToType>{};
}

template<typename ToType>
inline detail::const_cast_to_tag<ToType> const_cast_to() noexcept
{
    return detail::const_cast_to_tag<ToType>{};
}

template<typename ToType>
inline detail::reinterpret_cast_to_tag<ToType> reinterpret_cast_to() noexcept
{
    return detail::reinterpret_cast_to_tag<ToType>{};
}

template<typename ToType>
inline CCC_CPP20_CONSTEXPR detail::bit_cast_to_tag<ToType> bit_cast_to() noexcept
{
    return detail::bit_cast_to_tag<ToType>{};
}

inline constexpr detail::move_to_tag move_to() noexcept
{
    return detail::move_to_tag{};
}

template<typename ToType>
inline constexpr detail::forward_to_tag<ToType> forward_to() noexcept
{
    return detail::forward_to_tag<ToType>{};
}

inline namespace operators {
inline namespace type_cast_operators {

template<typename FromType, typename ToType>
inline constexpr ToType operator>(const FromType& from, const detail::static_cast_to_tag<ToType>&) noexcept
{
    return static_cast<ToType>(from);
}

template<typename FromType, typename ToType>
inline ToType operator>(const FromType& from, const detail::dynamic_cast_to_tag<ToType>&) noexcept
{
    return dynamic_cast<ToType>(from);
}

template<typename FromType, typename ToType>
inline ToType operator>(const FromType& from, const detail::const_cast_to_tag<ToType>&) noexcept
{
    return const_cast<ToType>(from);
}

template<typename FromType, typename ToType>
inline ToType operator>(const FromType& from, const detail::reinterpret_cast_to_tag<ToType>&) noexcept
{
    return reinterpret_cast<ToType>(from);
}

template<typename FromType, typename ToType>
inline CCC_CPP20_CONSTEXPR ToType operator>(const FromType& from,
                                                                 const detail::bit_cast_to_tag<ToType>&) noexcept
{
    return detail::bit_cast_impl<ToType>(from);
}

template<typename T>
inline constexpr decltype(auto) operator>(T&& t, const detail::move_to_tag&) noexcept
{
    return std::move(t);  // NOLINT(*-move-forwarding-reference)
}

template<typename FromType, typename ToType>
inline constexpr decltype(auto) operator>(FromType&& from, const detail::forward_to_tag<ToType>&) noexcept
{
    return std::forward<ToType>(from);
}

}  // namespace type_cast_operators
}  // namespace operators

}  // namespace pipe_operator_helper
}  // namespace ccc

#endif  // !CPP_PIPE_OPERATOR_HELPER_TYPE_CAST_HH
