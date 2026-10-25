#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag tag{"x"};
int main()
{
    auto a = ccc::tassert<tag>(true);
    auto b = static_cast<decltype(a)&&>(a);
}
