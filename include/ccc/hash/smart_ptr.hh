/**
 * @file smart_ptr.hh
 * @brief `ccc::hash` specializations for std::unique_ptr and std::shared_ptr
 *        (Compile-time-capable after C++20)
 */

#pragma once
#ifndef CCC_HASH_SMART_PTR_HH
#define CCC_HASH_SMART_PTR_HH

#include "ccc/detail/config.hh"

#include <cstddef>
#include <memory>
#include <type_traits>

#include "ccc/hash/hash.hh"

namespace ccc {

template<typename T, typename Deleter>
struct hash<
    std::unique_ptr<T, Deleter>,
    typename std::enable_if<detail::is_hashable<typename std::unique_ptr<T, Deleter>::pointer>::value, void>::type> {
    inline CCC_CPP20_CONSTEXPR std::size_t operator()(const std::unique_ptr<T, Deleter>& value) noexcept(
        detail::is_nothrow_hashable<typename std::unique_ptr<T, Deleter>::pointer>::value)
    {
        return hash<typename std::unique_ptr<T, Deleter>::pointer>()(value.get());
    }
};

template<typename T>
struct hash<std::shared_ptr<T>,
            typename std::enable_if<detail::is_hashable<typename std::remove_extent<T>::type*>::value, void>::type> {
    inline std::size_t operator()(const std::shared_ptr<T>& value) noexcept(
        detail::is_nothrow_hashable<typename std::remove_extent<T>::type*>::value)
    {
        // Hash the stored pointer itself (not the pointee), matching the
        // standard library policy for smart pointers
        return hash<typename std::remove_extent<T>::type*>()(value.get());
    }
};

}  // namespace ccc

#endif  // !CCC_HASH_SMART_PTR_HH
