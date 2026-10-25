#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag tag{"active"};
struct X {};
int main()
{
    ccc::tassert<tag>(true).message("{}", X{});
}
