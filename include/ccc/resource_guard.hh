/**
 * @file resource_guard.hh
 * @brief
 */

#pragma once
#ifndef CCC_RESOURCE_GUARD_HH
#define CCC_RESOURCE_GUARD_HH

#include "ccc/detail/config.hh"

#ifndef CCC_MODULE_INTERFACE_UNIT
#include <type_traits>
#include <utility>
#endif

CCC_MODULE_EXPORT
namespace ccc {

template<typename Func>
class defer final
{
public:
    explicit constexpr defer(const Func& func) noexcept : func_(func) {}
    explicit constexpr defer(Func&& func) noexcept : func_(std::move(func)) {}

    CCC_CPP20_CONSTEXPR ~defer()
    {
        func_();
    }

private:
    Func func_;
};

template<typename TryBlock, typename FinallyBlock>
inline constexpr void try_finally(TryBlock&& try_block, FinallyBlock&& finally_block)
{
    CCC_MAYBE_UNUSED defer _{std::forward<FinallyBlock>(finally_block)};
    std::forward<TryBlock>(try_block)();
}

template<typename T, typename Func>
inline constexpr decltype(auto) with(T&& value, Func&& func)
{
    std::forward<T>(value).with_start();
    CCC_MAYBE_UNUSED defer _{[&] { std::forward<T>(value).with_end(); }};
    return std::forward<Func>(func)();
}

}  // namespace ccc

#endif  // !CCC_RESOURCE_GUARD_HH
