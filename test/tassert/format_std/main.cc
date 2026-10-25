#include "../format_support.hh"
#include "../api.hh"

void capture(const ccc::tassert_failure_info& info) noexcept
{
    std::fwrite(info.message.data(), 1, info.message.size(), stdout);
    std::fputc('\n', stdout);
    std::exit(info.message == "zero" || info.message == "0007 custom temporary" ? 42 : 90);
}

int main(int argc, char** argv)
{
    ccc::tassert_set_handler(capture);
    const std::string_view mode = argc > 1 ? argv[1] : "success";
    ccc::tassert<active>(true).message("{}", Item{true});
    ccc::tassert<disabled>(false).message("{}", Item{true});
    if (formatter_calls != 0) {
        return 1;
    }
    if (mode == "zero") {
        ccc::tassert<active>(false).message("zero");
    }
    if (mode == "format") {
        ccc::tassert<active>(false).message("{:04d} {} {}", 7, Item{false}, std::string("temporary"));
    }
    return 0;
}
