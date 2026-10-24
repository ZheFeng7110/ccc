/**
 * @file impl.hh
 * @brief Resource guard utilities.
 */

#pragma once
#ifndef CCC_RESOURCE_GUARD_IMPL_HH
#define CCC_RESOURCE_GUARD_IMPL_HH

#include "ccc/detail/config.hh"

#include <type_traits>
#include <utility>

namespace ccc {

template<typename Func>
class defer final
{
public:
    explicit inline constexpr defer(const Func& func) noexcept : func_(func) {}
    explicit inline constexpr defer(Func&& func) noexcept : func_(std::move(func)) {}

    defer(const defer&) = delete;
    defer(defer&& other) = delete;
    defer& operator=(const defer&) = delete;
    defer& operator=(defer&&) = delete;

    inline CCC_CPP20_CONSTEXPR ~defer()
    {
        func_();
    }

private:
    Func func_;
};

namespace detail {

template<typename Func>
inline CCC_CPP20_CONSTEXPR defer<typename std::decay<Func>::type> make_defer(Func&& func)
{
    return defer<typename std::decay<Func>::type>(std::forward<Func>(func));
}

}  // namespace detail

template<typename TryBlock, typename FinallyBlock>
inline CCC_CPP20_CONSTEXPR void try_finally(TryBlock&& try_block, FinallyBlock&& finally_block)
{
    using defer_t = defer<typename std::decay<FinallyBlock>::type>;
    CCC_MAYBE_UNUSED defer_t _{std::forward<FinallyBlock>(finally_block)};
    std::forward<TryBlock>(try_block)();
}

template<typename T, typename Func>
inline CCC_CPP20_CONSTEXPR auto with(T&& value, Func&& func) -> decltype(std::forward<Func>(func)())
{
    std::forward<T>(value).with_start();
    auto end_action = [&] { std::forward<T>(value).with_end(); };
    CCC_MAYBE_UNUSED defer<decltype(end_action)> _{end_action};
    return std::forward<Func>(func)();
}

}  // namespace ccc

#endif  // !CCC_RESOURCE_GUARD_IMPL_HH
