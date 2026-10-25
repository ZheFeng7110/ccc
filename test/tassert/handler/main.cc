#include "../api.hh"

void capture(const ccc::tassert_failure_info& info) noexcept
{
    const bool valid = info.tag == "active" && info.location.line() == 321 &&
                       std::string_view(info.location.file_name()) == "handler_call_site.cc" &&
                       std::string_view(info.location.function_name()).find("main") != std::string_view::npos;
    std::fputs(info.message.empty() ? "BASIC\n" : "MESSAGE\n", stdout);
    std::exit(valid ? 42 : 90);
}
void returns(const ccc::tassert_failure_info&) noexcept
{
    std::fputs("RETURNED\n", stdout);
}

int main(int argc, char** argv)
{
    auto original = ccc::tassert_get_handler();
    if (!original || ccc::tassert_set_handler(capture) != original || ccc::tassert_get_handler() != capture ||
        ccc::tassert_set_handler(returns) != capture || ccc::tassert_set_handler(nullptr) != returns ||
        ccc::tassert_get_handler() != original) {
        return 1;
    }
    ccc::tassert_set_handler(capture);
    ccc::tassert<active>(true);
    ccc::tassert<disabled>(false);
    const std::string_view mode = argc > 1 ? argv[1] : "success";
    if (mode == "return") {
        ccc::tassert_set_handler(returns);
        ccc::tassert<active>(false);
    }
    if (mode == "capture") {
#line 321 "handler_call_site.cc"
        ccc::tassert<active>(false);
    }
    if (mode == "predicate") {
#line 321 "handler_call_site.cc"
        ccc::tassert<active>([] { return false; });
    }
    return 0;
}
