#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag tag{"disabled"};
static_assert(!tag.enabled());
int main()
{
    ccc::tassert<tag>(false).message("{:d}", "text");
}
