#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag tag{"x"};
struct X {};
int main()
{
    std::string f = "{}";
    ccc::tassert<tag>(true).message(f, 1);
}
