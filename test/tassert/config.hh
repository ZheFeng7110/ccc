#pragma once
#ifndef CCC_TEST_TASSERT_CONFIG_HH
#define CCC_TEST_TASSERT_CONFIG_HH
#include <string_view>
namespace ccc::tassert_config {
inline constexpr std::string_view ignore_tags[] = {"disabled", "disabled", "", std::string_view{"a\0b", 3}};
}
#endif  // !CCC_TEST_TASSERT_CONFIG_HH
