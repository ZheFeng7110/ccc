#include "../api.hh"

int main(int argc, char**)
{
    ccc::tassert<active>(true);
#ifdef TEST_CONFIGURED
    static_assert(!disabled.enabled());
    ccc::tassert<disabled>(false);
#endif
    if (argc > 1) {
#ifdef TEST_FORMAT
        ccc::tassert<active>(false).message("installed {}", 7);
#else
        ccc::tassert<active>(false);
#endif
    }
}
