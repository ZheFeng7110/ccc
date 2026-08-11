/**
 * @file when.hh
 * @brief Generalized switch-like `when` expression
 */

#pragma once
#ifndef CCC_WHEN_HH
#define CCC_WHEN_HH

#if (__cplusplus >= 201703L)

#include "ccc/detail/config.hh"

#include "ccc/utility.hh"

#include <type_traits>
#include <utility>

namespace ccc {

/// Tag type used to introduce the default branch in a `when` expression.
struct default_tag_t {
};

/// Tag object used to introduce the default branch in a `when` expression.
inline constexpr default_tag_t default_tag{};

namespace detail {

// is_default_tag
template<typename T>
struct is_default_tag : std::false_type {
};

template<>
struct is_default_tag<default_tag_t> : std::true_type {
};

template<typename T>
inline constexpr bool is_default_tag_v = is_default_tag<T>::value;

// is_nothrow_equality_comparable
template<typename T, typename CaseVal, typename = void>
struct is_nothrow_equality_comparable : std::false_type {
};

template<typename T, typename CaseVal>
struct is_nothrow_equality_comparable<
    T,
    CaseVal,
    void_t<decltype(std::declval<const T>() == std::declval<const CaseVal>()),
           enable_if_t<
               std::is_convertible<decltype(std::declval<const T>() == std::declval<const CaseVal>()), bool>::value>,
           enable_if_t<noexcept(std::declval<const T>() == std::declval<const CaseVal>())>>> : std::true_type {
};

template<typename T, typename CaseVal>
inline constexpr bool is_nothrow_equality_comparable_v = is_nothrow_equality_comparable<T, CaseVal>::value;

// is_valid_when_pair_v
template<typename R, typename T, typename CaseVal, typename Func>
inline constexpr bool is_valid_when_pair_v =
    std::is_invocable<Func>::value && std::is_same_v<R, std::invoke_result_t<Func>> &&
    (is_default_tag_v<remove_cvref_t<CaseVal>> || is_nothrow_equality_comparable_v<T, CaseVal>);

// is_when_args
template<typename R, typename T, typename... Args>
struct is_when_args;

template<typename R, typename T>
struct is_when_args<R, T> : std::true_type {
};

template<typename R, typename T, typename CaseVal, typename Func, typename... Rest>
struct is_when_args<R, T, CaseVal, Func, Rest...>
    : std::bool_constant<is_valid_when_pair_v<R, T, CaseVal, Func> && !is_default_tag_v<remove_cvref_t<CaseVal>> &&
                         is_when_args<R, T, Rest...>::value> {
};

template<typename R, typename T, typename CaseVal, typename Func>
struct is_when_args<R, T, CaseVal, Func> : std::bool_constant<is_valid_when_pair_v<R, T, CaseVal, Func>> {
};

template<typename R, typename T, typename... Args>
inline constexpr bool is_when_args_v = is_when_args<R, T, Args...>::value;

// WhenResult
template<typename... Args>
struct WhenResult {
    using type = void;
};

template<typename CaseVal1, typename Func1, typename... Rest>
struct WhenResult<CaseVal1, Func1, Rest...> : std::invoke_result<Func1> {
};

template<typename... Args>
using WhenResult_t = typename WhenResult<Args...>::type;

// is_nothrow_when_args
template<typename... Args>
struct is_nothrow_when_args;

template<>
struct is_nothrow_when_args<> : std::true_type {
};

template<typename CaseVal, typename Func, typename... Rest>
struct is_nothrow_when_args<CaseVal, Func, Rest...>
    : std::bool_constant<std::is_nothrow_invocable<Func>::value && is_nothrow_when_args<Rest...>::value> {
};

template<typename... Args>
inline constexpr bool is_nothrow_when_args_v = is_nothrow_when_args<Args...>::value;

// when_impl
template<typename R, typename T>
inline constexpr R when_impl(const T&)
{
    return R();
}

template<typename R, typename T, typename CaseVal, typename Func, typename... Rest>
inline constexpr R when_impl(const T& v, CaseVal&& case_val, Func&& func, Rest&&... rest)
{
    if constexpr (is_default_tag_v<remove_cvref_t<CaseVal>>) {
        return std::forward<Func>(func)();
    }
    else {
        if (v == std::forward<CaseVal>(case_val)) {
            return std::forward<Func>(func)();
        }
        else {
            return when_impl<R>(v, std::forward<Rest>(rest)...);
        }
    }
}

}  // namespace detail

/**
 * @brief A generalized switch-like expression.
 *
 * Pairs of arguments are treated as `(case_value, handler)`. The first handler
 * whose case value compares equal to @p v with `operator==` is executed and its
 * result is returned. If the special tag `ccc::default_tag` is used as a case
 * value, the corresponding handler is executed when no previous case matched.
 * If no case matches and there is no default branch, a default-constructed
 * result is returned.
 *
 * Example:
 * @code{cpp}
 * int r = ccc::when(value,
 *     1, [] { return -1; },
 *     2, [] { return -2; },
 *     ccc::default_tag, [] { return 0; }
 * );
 * @endcode
 *
 * @attention Arguments must appear in pairs. If `ccc::default_tag` is used, it
 * must form the last pair.
 */
template<typename T, typename... Args>
CCC_NO_DISCARD inline constexpr decltype(auto) when(const T& v,
                                             Args&&... args) noexcept(detail::is_nothrow_when_args_v<Args...>)
{
    static_assert(sizeof...(Args) % 2 == 0, "Arguments should be in pairs of (target, func) or (default_tag, func)");

    using ResultType = detail::WhenResult_t<Args...>;
    static_assert(detail::is_when_args_v<ResultType, T, Args...>,
                  "All case values should be comparable with operator== without throwing exceptions, "
                  "and all functions should be invocable with no parameters and return the same type. "
                  "The default_tag-default_func must be the last argument pair if used explicitly.");

    return detail::when_impl<ResultType>(v, std::forward<Args>(args)...);
}

}  // namespace ccc

#else

#ifndef CCC_SUPRESS_WARNINGS
CCC_WARNING("ccc::when requires C++17 or later")
#endif

#endif  // (__cplusplus >= 201703L)

#endif  // !CCC_WHEN_HH
