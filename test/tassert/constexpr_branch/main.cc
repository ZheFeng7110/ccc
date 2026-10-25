#include "../api.hh"

struct NotCallable {};
struct RvalueCallable {
    int* calls;
    void operator()() &&
    {
        ++*calls;
    }
    void operator()() & = delete;
};

int main()
{
    static_assert(!disabled.enabled());
    int calls = 0;
    if constexpr (disabled.enabled()) {
        ccc::tassert<disabled>(++calls == 0);
    }
    ccc::if_enabled<disabled>([&] {
        ccc::tassert<disabled>([value = ++calls] { return value == 0; });
#ifdef TEST_FORMAT
        ccc::tassert<disabled>(false).message("{}", ++calls);
#endif
    });
    ccc::if_enabled<disabled>(NotCallable{});
    ccc::if_enabled<active>(RvalueCallable{&calls});
    return calls != 1;
}
