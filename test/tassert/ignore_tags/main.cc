#include "../api.hh"

inline constexpr char other_address[] = "disabled";
inline constexpr ccc::tassert_tag same_text{other_address};
inline constexpr ccc::tassert_tag child{"disabled.child"};
inline constexpr ccc::tassert_tag case_sensitive{"Disabled"};
inline constexpr ccc::tassert_tag empty{""};
inline constexpr ccc::tassert_tag embedded{std::string_view{"a\0b", 3}};
inline constexpr ccc::tassert_tag shorter{"a"};
#ifdef TEST_EMPTY_CONFIG
static_assert(disabled.enabled() && same_text.enabled() && empty.enabled() && embedded.enabled());
#else
static_assert(!disabled.enabled() && !same_text.enabled() && !empty.enabled() && !embedded.enabled());
#endif
static_assert(child.enabled() && case_sensitive.enabled() && shorter.enabled() && active.enabled());

int main()
{
#ifndef TEST_EMPTY_CONFIG
    ccc::tassert<disabled>(false);
    ccc::tassert<same_text>(false);
    ccc::tassert<embedded>(false);
    ccc::tassert<empty>(false);
#endif
    return 0;
}
