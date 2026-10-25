#pragma once
#ifndef CCC_TEST_TASSERT_ARRAY_HH
#define CCC_TEST_TASSERT_ARRAY_HH
#include <array>
#include <string_view>
namespace ccc::tassert_config {
inline constexpr std::array ignore_tags = {std::string_view{"disabled"},
                                           std::string_view{"disabled"},
                                           std::string_view{""},
                                           std::string_view{"a\0b", 3}};
}
#endif  // !CCC_TEST_TASSERT_ARRAY_HH
