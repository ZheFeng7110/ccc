#pragma once
#ifndef CCC_DETAIL_TASSERT_REPORT_HH
#define CCC_DETAIL_TASSERT_REPORT_HH

#include "ccc/detail/tassert_config.hh"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

namespace ccc {
struct tassert_failure_info {
    std::string_view tag;
    std::source_location location;
    std::string_view message;
};

using tassert_handler = void (*)(const tassert_failure_info& info) noexcept;

namespace detail {
inline std::atomic_flag tassert_output_lock = ATOMIC_FLAG_INIT;

inline void tassert_write(std::string_view text) noexcept
{
    if (!text.empty()) {
        std::fwrite(text.data(), 1, text.size(), stderr);
    }
}

[[noreturn]] inline void tassert_default_handler(const tassert_failure_info& info) noexcept
{
    while (tassert_output_lock.test_and_set(std::memory_order_acquire)) {
    }
    std::fputs("[ccc.tassert] assertion failed\ntag: ", stderr);
    tassert_write(info.tag);
    std::fputs("\nlocation: ", stderr);
    std::fputs(info.location.file_name(), stderr);
    std::fprintf(stderr,
                 ":%lu:%lu\nfunction: %s\n",
                 static_cast<unsigned long>(info.location.line()),
                 static_cast<unsigned long>(info.location.column()),
                 info.location.function_name());
    if (!info.message.empty()) {
        std::fputs("message: ", stderr);
        tassert_write(info.message);
        std::fputc('\n', stderr);
    }
    std::fflush(stderr);
    tassert_output_lock.clear(std::memory_order_release);
    std::exit(EXIT_FAILURE);
}

inline std::atomic<tassert_handler> tassert_current_handler{tassert_default_handler};
inline thread_local bool tassert_dispatching = false;
}  // namespace detail

inline tassert_handler tassert_set_handler(tassert_handler handler) noexcept
{
    return detail::tassert_current_handler.exchange(handler ? handler : detail::tassert_default_handler,
                                                    std::memory_order_relaxed);
}

inline tassert_handler tassert_get_handler() noexcept
{
    return detail::tassert_current_handler.load(std::memory_order_relaxed);
}

namespace detail {
[[noreturn]] inline void tassert_dispatch(const tassert_failure_info& info) noexcept
{
    if (tassert_dispatching) {
        std::fputs("[ccc.tassert] recursive assertion failure\n", stderr);
        std::fflush(stderr);
        std::_Exit(EXIT_FAILURE);
    }
    tassert_dispatching = true;
    const auto handler = tassert_get_handler();
    handler(info);
    std::exit(EXIT_FAILURE);
}

template<bool Enabled>
class tassert_result
{
public:
    constexpr tassert_result(bool failed, std::string_view tag, std::source_location location) noexcept
        : failed_(failed), tag_(tag), location_(location)
    {
    }

    tassert_result(const tassert_result&) = delete;
    tassert_result(tassert_result&&) = delete;
    tassert_result& operator=(const tassert_result&) = delete;
    tassert_result& operator=(tassert_result&&) = delete;

    constexpr ~tassert_result() noexcept
    {
        if constexpr (Enabled) {
            if (failed_) {
                failed_ = false;
                tassert_dispatch({tag_, location_, {}});
            }
        }
    }

#if CCC_TASSERT_FORMAT_BACKEND == 1 || CCC_TASSERT_FORMAT_BACKEND == 2
    template<class... Args>
#if CCC_TASSERT_FORMAT_BACKEND == 1
    void message(std::format_string<Args...> format, Args&&... args) && noexcept
    {
#else
    void message(fmt::format_string<Args...> format, Args&&... args) && noexcept
    {
#endif
        if constexpr (Enabled) {
            if (failed_) {
                failed_ = false;
                std::string message;
                try {
#if CCC_TASSERT_FORMAT_BACKEND == 1
                    message = std::format(format, std::forward<Args>(args)...);
#else
                    message = fmt::format(format, std::forward<Args>(args)...);
#endif
                }
                catch (...) {
                    tassert_dispatch({tag_, location_, "ccc.tassert: message formatting failed"});
                }
                tassert_dispatch({tag_, location_, message});
            }
        }
    }
#else
    template<class... Args>
    void message(Args&&...) &&
    {
        static_assert(sizeof...(Args) == static_cast<std::size_t>(-1),
                      "ccc.tassert .message requires a formatting backend; select standard (1) or fmt (2)");
    }
#endif

private:
    bool failed_;
    std::string_view tag_;
    std::source_location location_;
};
}  // namespace detail
}  // namespace ccc

#endif  // !CCC_DETAIL_TASSERT_REPORT_HH
