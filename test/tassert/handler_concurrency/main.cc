#include "../api.hh"

void first(const ccc::tassert_failure_info& info) noexcept
{
    std::exit(info.tag == "active" ? 42 : 90);
}
void second(const ccc::tassert_failure_info& info) noexcept
{
    std::exit(info.tag == "active" ? 42 : 91);
}
void recursive(const ccc::tassert_failure_info&) noexcept
{
    std::fputs("HANDLER_ONCE\n", stderr);
    ccc::tassert<active>(false);
}

int main(int argc, char** argv)
{
    const std::string_view mode = argc > 1 ? argv[1] : "exchange";
    if (mode == "recursive") {
        ccc::tassert_set_handler(recursive);
        ccc::tassert<active>(false);
    }
    std::atomic<bool> bad{false};
    ccc::tassert_set_handler(first);
    std::vector<std::thread> threads;
    for (int n = 0; n < 8; ++n) {
        threads.emplace_back([&, n] {
            for (int i = 0; i < 10000; ++i) {
                auto old = ccc::tassert_set_handler(n % 2 ? first : second);
                auto current = ccc::tassert_get_handler();
                if ((old != first && old != second) || (current != first && current != second)) {
                    bad = true;
                }
            }
        });
    }
    if (mode == "dispatch") {
        ccc::tassert<active>(false);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    return bad ? 1 : 0;
}
