#pragma once
#ifndef CCC_TASSERT_HH
#define CCC_TASSERT_HH

#include "ccc/detail/tassert_config.hh"
#include "ccc/detail/tassert_report.hh"

#include <functional>
#include <source_location>
#include <string_view>
#include <utility>

namespace ccc {
class tassert_tag
{
public:
    consteval explicit tassert_tag(std::string_view name) : name_(name), enabled_(detail::tassert_tag_enabled(name)) {}

    constexpr std::string_view name() const noexcept
    {
        return name_;
    }
    constexpr bool enabled() const noexcept
    {
        return enabled_;
    }

private:
    std::string_view name_;
    bool enabled_;
};

template<const tassert_tag& Tag>
constexpr auto tassert(bool condition, std::source_location location = std::source_location::current()) noexcept
{
    if constexpr (Tag.enabled()) {
        return detail::tassert_result<true>{!condition, Tag.name(), location};
    }
    else {
        return detail::tassert_result<false>{false, Tag.name(), location};
    }
}

template<const tassert_tag& Tag, class Predicate>
    requires requires(Predicate&& predicate) { static_cast<bool>(std::invoke(std::forward<Predicate>(predicate))); }
constexpr auto tassert(Predicate&& predicate, std::source_location location = std::source_location::current())
{
    if constexpr (Tag.enabled()) {
        return detail::tassert_result<true>{!static_cast<bool>(std::invoke(std::forward<Predicate>(predicate))),
                                            Tag.name(),
                                            location};
    }
    else {
        return detail::tassert_result<false>{false, Tag.name(), location};
    }
}

template<const tassert_tag& Tag, class F>
constexpr void if_enabled(F&& body)
{
    if constexpr (Tag.enabled()) {
        std::invoke(std::forward<F>(body));
    }
}
}  // namespace ccc

#endif  // !CCC_TASSERT_HH
