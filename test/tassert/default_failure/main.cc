#include "../api.hh"

struct Local {
    ~Local()
    {
        std::fputs("LOCAL_DESTRUCTOR\n", stdout);
    }
};

void wrapper(std::source_location location = std::source_location::current())
{
    ccc::tassert<active>(false, location);
}

int main(int argc, char** argv)
{
    std::atexit([] { std::fputs("ATEXIT\n", stdout); });
    Local local;
    const std::string_view mode = argc > 1 ? argv[1] : "bare";
    if (mode == "explicit") {
#line 123 "tassert_call_site.cc"
        wrapper();
    }
#ifdef TEST_FORMAT
    if (mode == "message") {
#line 123 "tassert_call_site.cc"
        ccc::tassert<active>(false).message("value={:04d}", 7);
    }
#endif
#line 123 "tassert_call_site.cc"
    ccc::tassert<active>(false);
    std::fputs("CONTINUED\n", stdout);
}
