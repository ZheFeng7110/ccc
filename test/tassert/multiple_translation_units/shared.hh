#pragma once
#ifndef CCC_TEST_TASSERT_SHARED_HH
#define CCC_TEST_TASSERT_SHARED_HH
#include "../api.hh"
inline constexpr ccc::tassert_tag shared_tag{"shared"};
ccc::tassert_handler install_handler();
void shared_failure();
#endif  // !CCC_TEST_TASSERT_SHARED_HH
