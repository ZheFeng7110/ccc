#include "../api.hh"

int throwing_argument()
{
    std::fputs("ARGUMENT\n", stdout);
    throw std::runtime_error("argument");
}
void capture(const ccc::tassert_failure_info& info) noexcept
{
    std::fputs("BASIC_DURING_UNWIND\n", stdout);
    std::exit(info.message.empty() ? 42 : 90);
}
int main(int argc, char** argv)
{
    ccc::tassert_set_handler(capture);
    int caught = 0;
    try {
        ccc::tassert<active>([]() -> bool { throw std::runtime_error("predicate"); });
    }
    catch (const std::runtime_error&) {
        ++caught;
    }
    ccc::tassert<disabled>([]() -> bool { throw std::runtime_error("disabled"); });
#ifdef TEST_FORMAT
    const bool failed = argc > 1 && std::string_view(argv[1]) == "failed";
    try {
        ccc::tassert<active>(!failed).message("{}", throwing_argument());
    }
    catch (const std::runtime_error&) {
        ++caught;
    }
    try {
        ccc::tassert<disabled>(false).message("{}", throwing_argument());
    }
    catch (const std::runtime_error&) {
        ++caught;
    }
    return caught != 3;
#else
    (void)argc;
    (void)argv;
    return caught != 1;
#endif
}
