#include "../format_support.hh"
#include "../api.hh"
#include <new>

inline bool fail_allocation = false;

void* operator new(std::size_t size)
{
    if (fail_allocation) {
        throw std::bad_alloc{};
    }
    if (void* data = std::malloc(size ? size : 1)) {
        return data;
    }
    throw std::bad_alloc{};
}
void operator delete(void* data) noexcept
{
    std::free(data);
}
void operator delete(void* data, std::size_t) noexcept
{
    std::free(data);
}

void capture(const ccc::tassert_failure_info& info) noexcept
{
    const bool valid = info.tag == "active" && info.message.find("formatting failed") != std::string_view::npos &&
                       (fail_allocation || formatter_calls == 1);
    std::fputs("FORMAT_FALLBACK\n", stdout);
    std::exit(valid ? 42 : 90);
}
int main(int argc, char**)
{
    ccc::tassert_set_handler(capture);
    fail_allocation = true;
    ccc::tassert<active>(true).message("{}", Item{true});
    ccc::tassert<disabled>(false).message("{}", Item{true});
    fail_allocation = false;
    if (argc > 1) {
        fail_allocation = true;
        ccc::tassert<active>(false).message("{:01000}", 7);
    }
    ccc::tassert<active>(false).message("{}", Item{true});
}
