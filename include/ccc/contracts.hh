/**
 * @file contracts.hh
 * @brief
 */

#pragma once
#ifndef CCC_CONTRACTS_HH
#define CCC_CONTRACTS_HH

#if (__cplusplus >= 202002L)

#include "ccc/detail/config.hh"

#ifndef CCC_MODULE_INTERFACE_UNIT
#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <source_location>
#endif

namespace ccc::contracts {

CCC_MODULE_EXPORT_BEGIN

#undef IGNORE
#undef OBSERVE
#undef ENFORCE

#define IGNORE  0
#define OBSERVE 1
#define ENFORCE 2

#ifndef CCC_CONTRACTS_EVALUATION_SEMANTIC
#define CCC_CONTRACTS_EVALUATION_SEMANTIC ENFORCE
#endif

enum class evaluation_semantic : std::uint_fast8_t {
    ignore = IGNORE,
    observe = OBSERVE,
    enforce = ENFORCE,
};

static_assert(std::is_integral_v<std::remove_cvref_t<decltype(CCC_CONTRACTS_EVALUATION_SEMANTIC)>>);
static_assert((CCC_CONTRACTS_EVALUATION_SEMANTIC) >= 0 && (CCC_CONTRACTS_EVALUATION_SEMANTIC) <= 2);

constexpr auto global_evaluation_semantic = static_cast<evaluation_semantic>(CCC_CONTRACTS_EVALUATION_SEMANTIC);

#undef IGNORE
#undef OBSERVE
#undef ENFORCE

struct info {
    const char* message;
    std::source_location location;
    evaluation_semantic semantic;
};

// ReSharper disable once CppPassValueParameterByConstReference
inline constexpr void default_violation_handler(const info i)
{
    using namespace std::string_view_literals;

    std::cout << "[ccc::contracts]: Contract violated at " << i.location.file_name() << ":" << i.location.line()
              << '\n';
    if (const std::string_view msg = i.message; !msg.empty()) {
        std::cout << "    Message: " << msg << '\n';
    }
}

using violation_handler_type = void (*)(info i);

CCC_MODULE_EXPORT_END

inline constinit violation_handler_type violation_handler = default_violation_handler;

namespace detail {

inline void contract_violation_at_compile_time_here() noexcept {}

}  // namespace detail

CCC_MODULE_EXPORT_BEGIN

inline constexpr void set_violation_handler(const violation_handler_type handler) noexcept
{
    violation_handler = handler;
}

inline constexpr violation_handler_type get_violation_handler() noexcept
{
    return violation_handler;
}

inline constexpr void require(const bool cond,
                              const char* msg = "",
                              const std::source_location loc = std::source_location::current())
{
    if constexpr (global_evaluation_semantic != evaluation_semantic::ignore) {
        if (!cond) {
            if (std::is_constant_evaluated()) {
#ifdef __cpp_exceptions
                throw std::runtime_error(msg);
#else
                detail::contract_violation_at_compile_time_here();
                std::terminate();
#endif
            }

            violation_handler({.message = msg, .location = loc, .semantic = global_evaluation_semantic});

            if constexpr (global_evaluation_semantic == evaluation_semantic::enforce) {
                std::abort();
            }
        }
    }
}

CCC_MODULE_EXPORT_END

}  // namespace ccc::contracts

#else  // (__cplusplus >= 202002L)

#ifndef CCC_SUPRESS_WARNINGS
#warning "ccc.contracts requires C++20 or later"
#endif

#endif  // !(__cplusplus >= 202002L)

#endif  // !CCC_CONTRACTS_HH
