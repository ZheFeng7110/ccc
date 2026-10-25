#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag tag{"x"};
struct X {};
int main()
{
    auto a = ccc::tassert<tag>(true);
    a.message("text");
}
