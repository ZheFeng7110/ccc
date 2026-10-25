#include "ccc/tassert.hh"
inline constexpr ccc::tassert_tag tag{"x"};
struct X {};
int main()
{
    ccc::tassert<tag>([value = 0] {
        (void)value;
        return X{};
    });
}
