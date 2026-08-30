#define CATCH_CONFIG_MAIN

// Catch2 v2.13.10 uses std::nothrow (catch.hpp) without including <new>;
// libc++ 22 no longer provides it transitively in C++23 mode.
#include <new>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 5311)
#endif

#include <catch2/catch.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif
