#pragma once
#ifndef CCC_TEST_TASSERT_EMPTY_HH
#define CCC_TEST_TASSERT_EMPTY_HH
#include <array>
#include <string_view>
namespace ccc::tassert_config {
inline constexpr std::array<std::string_view, 0> ignore_tags{};
}
#endif  // !CCC_TEST_TASSERT_EMPTY_HH
