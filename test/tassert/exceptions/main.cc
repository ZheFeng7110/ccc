#include "../api.hh"

int throwing_argument(bool should_throw)
{
    std::fputs("ARGUMENT\n", stdout);
    if (should_throw) {
        throw std::runtime_error("argument");
    }
    return 0;
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
        ccc::tassert<active>([&] {
            if (argc > 0) {
                throw std::runtime_error("predicate");
            }
            return true;
        });
    }
    catch (const std::runtime_error&) {
        ++caught;
    }
    ccc::tassert<disabled>([&] {
        if (argc > 0) {
            throw std::runtime_error("disabled");
        }
        return true;
    });
#ifdef TEST_FORMAT
    const bool failed = argc > 1 && std::string_view(argv[1]) == "failed";
    try {
        ccc::tassert<active>(!failed).message("{}", throwing_argument(argc > 0));
    }
    catch (const std::runtime_error&) {
        ++caught;
    }
    try {
        ccc::tassert<disabled>(false).message("{}", throwing_argument(argc > 0));
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
