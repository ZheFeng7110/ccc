/**
 * @file smart_ptr.cc
 * @brief Tests for ccc::hash specializations of std::unique_ptr and std::shared_ptr
 */

#ifndef TEST_USE_MODULE
#include "ccc/hash.hh"
#endif

#include "test_pch.hh"

#include <cstddef>
#include <memory>
#include <type_traits>

#ifdef TEST_USE_MODULE
import ccc.hash;
#endif

TEST_CASE("Hash - Unique pointer")
{
    auto first = std::unique_ptr<int>(new int(42));
    std::unique_ptr<int> empty{};

    CHECK(ccc::hash<std::unique_ptr<int>>()(first) == ccc::hash<std::unique_ptr<int>>()(first));
    CHECK(ccc::hash<std::unique_ptr<int>>()(first) != ccc::hash<std::unique_ptr<int>>()(empty));

    // Null pointer hashes to 0
    CHECK(ccc::hash<std::unique_ptr<int>>()(empty) == 0U);

    // Array form
    auto array_ptr = std::unique_ptr<int[]>(new int[3]{1, 2, 3});
    CHECK(ccc::hash<std::unique_ptr<int[]>>()(array_ptr) == ccc::hash<std::unique_ptr<int[]>>()(array_ptr));
    CHECK(ccc::hash<std::unique_ptr<int[]>>()(array_ptr) !=
          ccc::hash<std::unique_ptr<int[]>>()(std::unique_ptr<int[]>{}));
}

TEST_CASE("Hash - Shared pointer")
{
    auto shared = std::shared_ptr<int>(new int(42));
    auto alias = shared;
    std::shared_ptr<int> empty{};

    // Copies share the same stored pointer and must hash equally
    CHECK(ccc::hash<std::shared_ptr<int>>()(shared) == ccc::hash<std::shared_ptr<int>>()(alias));
    CHECK(ccc::hash<std::shared_ptr<int>>()(shared) != ccc::hash<std::shared_ptr<int>>()(empty));
    CHECK(ccc::hash<std::shared_ptr<int>>()(empty) == 0U);

    // Distinct objects with equal values hash differently
    auto other = std::shared_ptr<int>(new int(42));
    CHECK(ccc::hash<std::shared_ptr<int>>()(shared) != ccc::hash<std::shared_ptr<int>>()(other));

    // Array form. std::shared_ptr<T[]> array support is a C++17 feature
    // (P0674): pre-C++17 libc++ rejects both the raw-pointer constructor and
    // std::make_shared<T[]>, libstdc++ only offers the constructor as an
    // extension in C++11/14, and MSVC's unconstrained std::make_shared
    // generic overload breaks make_shared<T[]> until its C++20 constraints.
#if (__cplusplus >= 202002L)
    auto array_ptr = std::make_shared<int[]>(3);
    CHECK(ccc::hash<std::shared_ptr<int[]>>()(array_ptr) == ccc::hash<std::shared_ptr<int[]>>()(array_ptr));
#elif (__cplusplus >= 201703L)
    auto array_ptr = std::shared_ptr<int[]>(new int[3]);
    CHECK(ccc::hash<std::shared_ptr<int[]>>()(array_ptr) == ccc::hash<std::shared_ptr<int[]>>()(array_ptr));
#endif
}
