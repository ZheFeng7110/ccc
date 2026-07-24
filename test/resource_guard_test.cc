/**
 * @file resource_guard_test.cc
 * @brief
 */

#ifndef TEST_USE_MODULE
#include "ccc/resource_guard.hh"
#endif

#include "test_pch.hh"

#include <cstdint>
#include <cstring>

#include <stdexcept>
#include <iostream>
#include <string>

#include <type_traits>

#ifdef TEST_USE_MODULE
#include "ccc/detail/config.hh"
import ccc.resource_guard;
#endif

#define TEST_REPORT(msg) \
    (std::cout << "In File `" << __FILE__ << ":" << __LINE__ << "` has message: \n    " << (msg) << std::endl)

TEST_CASE("ResourceGuard - Defer_Lambda")
{
    int x = 0;
    {
        auto action = [&] { x = 42; };
        CCC_MAYBE_UNUSED ccc::defer<decltype(action)> df{action};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        CHECK(x == 0);
    }
    CHECK(x == 42);
}

namespace {

bool file_on = false;
void open_file(const bool will_succeed)
{
    file_on = true;
    if (!will_succeed) {
        throw std::runtime_error("Failed to open file");
    }
}
void close_file() noexcept
{
    file_on = false;
}

}  // namespace

TEST_CASE("ResourceGuard - Defer_Function")
{
    REQUIRE_THROWS_AS(open_file(false), std::runtime_error);
    CHECK(file_on);
    close_file();
    REQUIRE_FALSE(file_on);
    REQUIRE_NOTHROW(open_file(true));
    CHECK(file_on);
    close_file();
    REQUIRE_FALSE(file_on);

    {
        CCC_MAYBE_UNUSED ccc::defer<void (*)()> df{&close_file};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        REQUIRE_NOTHROW(open_file(true));
        CHECK(file_on);
    }
    CHECK_FALSE(file_on);

    {
        CCC_MAYBE_UNUSED ccc::defer<void (*)()> df{&close_file};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        REQUIRE_THROWS_AS(open_file(false), std::runtime_error);
        CHECK(file_on);
    }
    CHECK_FALSE(file_on);
}

TEST_CASE("ResourceGuard - Defer_CallableObject")
{
    struct Callable {
        int a = 0;
        void operator()() noexcept
        {
            a = 114514;
        }
    };

    Callable callable;
    CHECK(callable.a == 0);
    {
        auto action = [&] { callable(); };
        CCC_MAYBE_UNUSED ccc::defer<decltype(action)> df{action};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        callable.a = 1919810;
        CHECK(callable.a == 1919810);
    }
    CHECK(callable.a == 114514);
}

TEST_CASE("ResourceGuard - TryFinally")
{
    struct File {
        bool on = false;
        void open()
        {
            on = true;
        }
        void close() noexcept
        {
            on = false;
        }
    };

    File f;
    f.open();
    CHECK(f.on);

    ccc::try_finally(
        [&] {
            // do something...
            return;
        },
        [&] { f.close(); });
    CHECK_FALSE(f.on);
}

TEST_CASE("ResourceGuard - With")
{
    struct Lock {
        bool is_locked = false;
        void lock() noexcept
        {
            is_locked = true;
        }
        void unlock() noexcept
        {
            is_locked = false;
        }

        void with_start() noexcept
        {
            lock();
        }
        void with_end() noexcept
        {
            unlock();
        }
    };

    Lock l;
    REQUIRE_FALSE(l.is_locked);

    const auto ret = ccc::with(l, [&] {
        CHECK(l.is_locked);
        return 114514;
    });
    CHECK(114514 == ret);
    CHECK_FALSE(l.is_locked);
}
