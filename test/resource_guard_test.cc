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

TEST(ResourceGuard, Defer_Lambda)
{
    int x = 0;
    {
        CCC_MAYBE_UNUSED ccc::defer df{[&] { x = 42; }};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        EXPECT_EQ(x, 0);
    }
    EXPECT_EQ(x, 42);
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

TEST(ResourceGuard, Defer_Function)
{
    ASSERT_THROW(open_file(false), std::runtime_error);
    EXPECT_TRUE(file_on);
    close_file();
    ASSERT_FALSE(file_on);
    ASSERT_NO_THROW(open_file(true));
    EXPECT_TRUE(file_on);
    close_file();
    ASSERT_FALSE(file_on);

    {
        CCC_MAYBE_UNUSED ccc::defer df{&close_file};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        ASSERT_NO_THROW(open_file(true));
        EXPECT_TRUE(file_on);
    }
    EXPECT_FALSE(file_on);

    {
        CCC_MAYBE_UNUSED ccc::defer df{&close_file};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        ASSERT_THROW(open_file(false), std::runtime_error);
        EXPECT_TRUE(file_on);
    }
    EXPECT_FALSE(file_on);
}

TEST(ResourceGuard, Defer_CallableObject)
{
    struct Callable {
        int a = 0;
        void operator()() noexcept
        {
            a = 114514;
        }
    };

    Callable callable;
    EXPECT_EQ(callable.a, 0);
    {
        CCC_MAYBE_UNUSED ccc::defer df{[&] { callable(); }};
        TEST_REPORT("sizeof defer == " + std::to_string(sizeof df));
        callable.a = 1919810;
        EXPECT_EQ(callable.a, 1919810);
    }
    EXPECT_EQ(callable.a, 114514);
}

TEST(ResourceGuard, TryFinally)
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
    EXPECT_TRUE(f.on);

    ccc::try_finally(
        [&] {
            // do something...
            return;
        },
        [&] { f.close(); });
    EXPECT_FALSE(f.on);
}

TEST(ResourceGuard, With)
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
    ASSERT_FALSE(l.is_locked);

    const auto ret = ccc::with(l, [&] {
        EXPECT_TRUE(l.is_locked);
        return 114514;
    });
    EXPECT_EQ(114514, ret);
    EXPECT_FALSE(l.is_locked);
}
