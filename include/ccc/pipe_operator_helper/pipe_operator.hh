/**
 * @file pipe_operator.hh
 * @brief
 */

// ReSharper disable CppRedundantInlineSpecifier

#pragma once
#ifndef CPP_PIPE_OPERATOR_HELPER_PIPE_OPERATOR_HH
#define CPP_PIPE_OPERATOR_HELPER_PIPE_OPERATOR_HH

#include "ccc/pipe_operator_helper/detail/config.hh"

#ifndef CCC_PIPE_OPERATOR_HELPER_MODULE_INTERFACE_UNIT
#include <utility>
#include <type_traits>
#include <functional>
#endif

namespace ccc {
namespace pipe_operator_helper {

CCC_PIPE_OPERATOR_HELPER_MODULE_EXPORT_BEGIN

template<typename Callable>
class pipe_tag
{
public:
    using callable_type = Callable;

private:
    callable_type callable_;

public:
    explicit constexpr pipe_tag(Callable&& callable) noexcept : callable_(std::forward<Callable>(callable)) {}

    constexpr const callable_type& get_callable() const& noexcept
    {
        return callable_;
    }
    constexpr callable_type& get_callable() & noexcept
    {
        return callable_;
    }
    constexpr callable_type&& get_callable() && noexcept
    {
        return std::move(callable_);
    }
};

inline namespace operators {
inline namespace pipe_operators {

template<typename FirstArg, typename RemainCallable>
inline constexpr decltype(auto)
operator|(FirstArg&& first_arg, const pipe_tag<RemainCallable>& remain_callable) noexcept(
    std::is_nothrow_invocable_v<decltype(remain_callable.get_callable()), decltype(std::forward<FirstArg>(first_arg))>)
{
    return std::invoke(remain_callable.get_callable(), std::forward<FirstArg>(first_arg));
}
template<typename FirstArg, typename RemainCallable>
inline constexpr decltype(auto) operator|(FirstArg&& first_arg, pipe_tag<RemainCallable>&& remain_callable) noexcept(
    std::is_nothrow_invocable_v<decltype(std::move(remain_callable).get_callable()),
                                decltype(std::forward<FirstArg>(first_arg))>)
{
    return std::invoke(std::move(remain_callable).get_callable(), std::forward<FirstArg>(first_arg));
}

}  // namespace pipe_operators
}  // namespace operators

CCC_PIPE_OPERATOR_HELPER_MODULE_EXPORT_END

}  // namespace pipe_operator_helper
}  // namespace ccc

#endif  // !CPP_PIPE_OPERATOR_HELPER_PIPE_OPERATOR_HH
