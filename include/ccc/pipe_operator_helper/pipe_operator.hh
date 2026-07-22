/**
 * @file pipe_operator.hh
 * @brief
 */

// ReSharper disable CppRedundantInlineSpecifier

#pragma once
#ifndef CPP_PIPE_OPERATOR_HELPER_PIPE_OPERATOR_HH
#define CPP_PIPE_OPERATOR_HELPER_PIPE_OPERATOR_HH

#include "ccc/pipe_operator_helper/detail/config.hh"

#include <utility>
#include <type_traits>
#include <tuple>

namespace ccc {
namespace pipe_operator_helper {

struct pipe_operator_base {
};

template<typename T>
struct is_pipe_operator : std::is_base_of<pipe_operator_base, T> {
};

template<typename T>
inline constexpr bool is_pipe_operator_v = is_pipe_operator<T>::value;

namespace detail {

template<typename Callee, typename... BoundArgs>
class pipe_partial
{
public:
    using callee_type = Callee;

private:
    const callee_type& callee_;
    std::tuple<BoundArgs...> bound_;

    template<typename Lhs, std::size_t... Is>
    constexpr decltype(auto) invoke_(Lhs&& lhs, std::index_sequence<Is...>) const
        noexcept(noexcept(callee_(std::forward<Lhs>(lhs), std::get<Is>(bound_)...)))
    {
        return callee_(std::forward<Lhs>(lhs), std::get<Is>(bound_)...);
    }

public:
    template<typename... Init>
    constexpr explicit pipe_partial(const callee_type& callee,
                                    Init&&... init) noexcept((std::is_nothrow_constructible<BoundArgs, Init>::value &&
                                                              ...))
        : callee_(callee), bound_(std::forward<Init>(init)...)
    {
    }

    template<typename Lhs>
    constexpr decltype(auto) operator()(Lhs&& lhs) const
        noexcept(noexcept(invoke_(std::forward<Lhs>(lhs), std::index_sequence_for<BoundArgs...>{})))
    {
        return invoke_(std::forward<Lhs>(lhs), std::index_sequence_for<BoundArgs...>{});
    }
};

}  // namespace detail

template<typename Derived, std::size_t Arity>
class pipe_operator : public pipe_operator_base
{
public:
    template<typename... Bound
#ifndef __cpp_concepts
             ,
             typename = std::enable_if_t<(sizeof...(Bound) < Arity)>
#endif
             >
#ifdef __cpp_concepts
        requires(sizeof...(Bound) < Arity)
#endif
    constexpr auto operator()(Bound&&... bound) const
        noexcept(noexcept(detail::pipe_partial<Derived, Bound...>(static_cast<const Derived&>(*this),
                                                                  std::forward<Bound>(bound)...)))
            -> detail::pipe_partial<Derived, Bound...>
    {
        return detail::pipe_partial<Derived, Bound...>(static_cast<const Derived&>(*this),
                                                       std::forward<Bound>(bound)...);
    }
};

inline namespace operators {
inline namespace pipe_operators {

template<typename FirstArg, typename Callee, typename... BoundArgs>
inline constexpr decltype(auto) operator|(FirstArg&& first_arg,
                                          const detail::pipe_partial<Callee, BoundArgs...>&
                                              partial) noexcept(noexcept(partial(std::forward<FirstArg>(first_arg))))
{
    return partial(std::forward<FirstArg>(first_arg));
}

}  // namespace pipe_operators
}  // namespace operators

}  // namespace pipe_operator_helper
}  // namespace ccc

#endif  // !CPP_PIPE_OPERATOR_HELPER_PIPE_OPERATOR_HH
