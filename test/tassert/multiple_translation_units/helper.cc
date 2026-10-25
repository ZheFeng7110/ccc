#ifdef TEST_MIXED
#include "ccc/tassert.hh"
#undef TEST_IMPORT_ALL
#undef TEST_IMPORT_TASSERT
#endif
#include "shared.hh"

void shared_capture(const ccc::tassert_failure_info& info) noexcept
{
    std::fputs("SHARED_HANDLER\n", stdout);
    std::exit(info.tag == "shared" ? 42 : 90);
}
ccc::tassert_handler install_handler()
{
    ccc::tassert_set_handler(shared_capture);
    return shared_capture;
}
void shared_failure()
{
    ccc::tassert<shared_tag>(false);
}
