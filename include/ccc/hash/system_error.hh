/**
 * @file system_error.hh
 * @brief `ccc::hash` specializations for std::error_code and std::error_condition
 *        (Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_SYSTEM_ERROR_HH
#define CCC_HASH_SYSTEM_ERROR_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <system_error>

#include "ccc/hash/hash.hh"
#include "ccc/hash/utils.hh"

namespace ccc {

// Standard library categories are unique singletons, so hashing the category
// object address alongside the integer value follows the usual policy
// (e.g. libstdc++ / MSVC STL)

template<>
struct hash<std::error_code, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::error_code& value) const noexcept
    {
        return hash_combine(hash_combine(0U, value.value()), &value.category());
    }
};

template<>
struct hash<std::error_condition, void> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::error_condition& value) const noexcept
    {
        return hash_combine(hash_combine(0U, value.value()), &value.category());
    }
};

}  // namespace ccc

#endif  // !CCC_HASH_SYSTEM_ERROR_HH
