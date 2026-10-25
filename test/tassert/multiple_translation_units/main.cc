#include "shared.hh"

int main(int argc, char**)
{
#ifdef TEST_EMPTY_MODULE_CONFIG
    static_assert(disabled.enabled());
#else
    static_assert(!disabled.enabled());
#endif
    const auto handler = install_handler();
    if (ccc::tassert_get_handler() != handler) {
        return 1;
    }
#ifndef TEST_EMPTY_MODULE_CONFIG
    ccc::tassert<disabled>(false);
#endif
    ccc::tassert<shared_tag>(true);
    if (argc > 1) {
        ccc::tassert<shared_tag>(false);
    }
    shared_failure();
}
