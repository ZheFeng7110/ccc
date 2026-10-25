#include "../api.hh"
#include "ccc/all.hh"

#ifdef TEST_EXPECT_BACKEND
static_assert(CCC_TASSERT_FORMAT_BACKEND == TEST_EXPECT_BACKEND);
#endif
static_assert(active.enabled());
static_assert(active.name() == "active");
static_assert(!std::is_copy_constructible_v<decltype(ccc::tassert<active>(true))>);
static_assert(!std::is_move_constructible_v<decltype(ccc::tassert<active>(true))>);
static_assert(std::is_nothrow_destructible_v<decltype(ccc::tassert<active>(true))>);

int main(int argc, char**)
{
    if (argc > 1) {
        ccc::tassert<active>(false);
    }
    ccc::tassert<active>(true);
#if defined(TEST_EXPECT_BACKEND) && TEST_EXPECT_BACKEND == 1
    ccc::tassert<active>(true).message("auto {}", 7);
#endif
    ccc::tassert<active>([] { return true; });
    int calls = 0;
    ccc::if_enabled<active>([&] { ++calls; });
    return calls != 1;
}
