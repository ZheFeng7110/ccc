#include "../api.hh"

struct Truth {
    int* conversions;
    explicit operator bool() const
    {
        ++*conversions;
        return true;
    }
};

int main()
{
    static_assert(!disabled.enabled());
    int calls = 0;
    int conversions = 0;
    ccc::tassert<active>([&] {
        ++calls;
        return Truth{&conversions};
    });
    ccc::tassert<disabled>([&] {
        ++calls;
        return Truth{&conversions};
    });
    if (calls != 1 || conversions != 1) {
        return 1;
    }
    ccc::tassert<disabled>(++calls == 0);
    ccc::tassert<disabled>([value = ++calls] { return value == 0; });
    if (calls != 3) {
        return 2;
    }
#ifdef TEST_FORMAT
    ccc::tassert<disabled>(false).message("{}", ++calls);
    ccc::tassert<active>(true).message("{}", ++calls);
    if (calls != 5) {
        return 3;
    }
#endif
    return 0;
}
