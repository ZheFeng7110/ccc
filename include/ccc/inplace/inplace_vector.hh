/**
 * @file inplace_vector.hh
 * @brief Fixed-capacity contiguous container with in-place storage (C++11 minimum).
 *
 * Implements ccc::inplace_vector following the C++26 std::inplace_vector interface.
 */

#pragma once
#ifndef CCC_INPLACE_VECTOR_HH
#define CCC_INPLACE_VECTOR_HH

#include "ccc/detail/config.hh"

#ifndef CCC_MODULE_INTERFACE_UNIT
#include <cstddef>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#if (__cplusplus >= 202002L)
#include <compare>
#endif
#include <utility>

#include "ccc/utility.hh"

#if (__cplusplus >= 202002L)
#include "ccc/contracts.hh"
#endif

#endif

#include "ccc/detail/assertions.hh"

namespace ccc {

CCC_MODULE_EXPORT_BEGIN

template<typename T, std::size_t N>
class inplace_vector
{
    static_assert(N > 0, "inplace_vector capacity must be greater than zero");

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    alignas(T) unsigned char store_[sizeof(T) * N];
    size_type size_;

    CCC_CPP20_CONSTEXPR pointer ptr_at(size_type i) noexcept
    {
        return reinterpret_cast<pointer>(store_ + i * sizeof(T));
    }
    CCC_CPP20_CONSTEXPR const_pointer ptr_at(size_type i) const noexcept
    {
        return reinterpret_cast<const_pointer>(store_ + i * sizeof(T));
    }

public:
    // ---------- Static capacity ----------

    CCC_NO_DISCARD static constexpr size_type capacity() noexcept
    {
        return N;
    }
    CCC_NO_DISCARD static constexpr size_type max_size() noexcept
    {
        return N;
    }
    static CCC_CPP20_CONSTEXPR void reserve(size_type new_cap)
    {
        if (new_cap > N) {
            throw std::bad_alloc();
        }
    }
    static CCC_CPP20_CONSTEXPR void shrink_to_fit() noexcept {}

    // ---------- Size ----------

    CCC_NO_DISCARD constexpr size_type size() const noexcept
    {
        return size_;
    }
    CCC_NO_DISCARD constexpr bool empty() const noexcept
    {
        return size_ == 0;
    }
    CCC_NO_DISCARD constexpr bool full() const noexcept
    {
        return size_ == N;
    }

    // ---------- Element access ----------

    CCC_CPP20_CONSTEXPR reference operator[](size_type i) noexcept
    {
        CCC_DETAIL_ASSERT(i < size_, "[ccc.inplace_vector]: index out of range");
        return *ptr_at(i);
    }
    CCC_CPP20_CONSTEXPR const_reference operator[](size_type i) const noexcept
    {
        CCC_DETAIL_ASSERT(i < size_, "[ccc.inplace_vector]: index out of range");
        return *ptr_at(i);
    }
    CCC_CPP20_CONSTEXPR reference at(size_type i)
    {
        if (i >= size_) {
            throw std::out_of_range("[ccc.inplace_vector]: index out of range");
        }
        return *ptr_at(i);
    }
    CCC_CPP20_CONSTEXPR const_reference at(size_type i) const
    {
        if (i >= size_) {
            throw std::out_of_range("[ccc.inplace_vector]: index out of range");
        }
        return *ptr_at(i);
    }
    CCC_CPP20_CONSTEXPR reference front() noexcept
    {
        CCC_DETAIL_ASSERT(!empty(), "[ccc.inplace_vector]: front() on empty vector");
        return *ptr_at(0);
    }
    CCC_CPP20_CONSTEXPR const_reference front() const noexcept
    {
        CCC_DETAIL_ASSERT(!empty(), "[ccc.inplace_vector]: front() on empty vector");
        return *ptr_at(0);
    }
    CCC_CPP20_CONSTEXPR reference back() noexcept
    {
        CCC_DETAIL_ASSERT(!empty(), "[ccc.inplace_vector]: back() on empty vector");
        return *ptr_at(size_ - 1);
    }
    CCC_CPP20_CONSTEXPR const_reference back() const noexcept
    {
        CCC_DETAIL_ASSERT(!empty(), "[ccc.inplace_vector]: back() on empty vector");
        return *ptr_at(size_ - 1);
    }
    CCC_CPP20_CONSTEXPR pointer data() noexcept
    {
        return ptr_at(0);
    }
    CCC_CPP20_CONSTEXPR const_pointer data() const noexcept
    {
        return ptr_at(0);
    }

    // ---------- Iterators ----------

    CCC_CPP20_CONSTEXPR iterator begin() noexcept
    {
        return ptr_at(0);
    }
    CCC_CPP20_CONSTEXPR const_iterator begin() const noexcept
    {
        return ptr_at(0);
    }
    CCC_CPP20_CONSTEXPR const_iterator cbegin() const noexcept
    {
        return ptr_at(0);
    }
    CCC_CPP20_CONSTEXPR iterator end() noexcept
    {
        return ptr_at(size_);
    }
    CCC_CPP20_CONSTEXPR const_iterator end() const noexcept
    {
        return ptr_at(size_);
    }
    CCC_CPP20_CONSTEXPR const_iterator cend() const noexcept
    {
        return ptr_at(size_);
    }

    reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }
    const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cend());
    }
    reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }

    // ---------- Constructors / destructor ----------

    constexpr inplace_vector() noexcept : size_(0) {}

    CCC_CPP20_CONSTEXPR inplace_vector(const inplace_vector& other) : size_(other.size_)
    {
        std::uninitialized_copy(other.begin(), other.end(), ptr_at(0));
    }

    CCC_CPP20_CONSTEXPR inplace_vector(inplace_vector&& other) noexcept(std::is_nothrow_move_constructible<T>::value)
        : size_(other.size_)
    {
        std::uninitialized_copy(std::make_move_iterator(other.begin()),
                                std::make_move_iterator(other.end()),
                                ptr_at(0));
    }

    CCC_CPP20_CONSTEXPR inplace_vector(std::initializer_list<T> ilist) : size_(ilist.size())
    {
        if (ilist.size() > N) {
            throw std::bad_alloc();
        }
        std::uninitialized_copy(ilist.begin(), ilist.end(), ptr_at(0));
    }

    CCC_CPP20_CONSTEXPR ~inplace_vector()
    {
        ccc::destroy(begin(), end());
    }

    // ---------- Assignment ----------

    CCC_CPP20_CONSTEXPR inplace_vector& operator=(const inplace_vector& other)
    {
        if (this == &other) {
            return *this;
        }
        assign(other.begin(), other.end());
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_vector& operator=(inplace_vector&& other) noexcept(
        std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value)
    {
        if (this == &other) {
            return *this;
        }
        const size_type common = (std::min)(size_, other.size_);
        std::move(other.begin(), other.begin() + common, ptr_at(0));
        if (size_ > other.size_) {
            ccc::destroy(ptr_at(common), ptr_at(size_));
        }
        else if (other.size_ > size_) {
            std::uninitialized_copy(std::make_move_iterator(other.begin() + common),
                                    std::make_move_iterator(other.end()),
                                    ptr_at(common));
        }
        size_ = other.size_;
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_vector& operator=(std::initializer_list<T> ilist)
    {
        assign(ilist);
        return *this;
    }

    // ---------- assign ----------

    CCC_CPP20_CONSTEXPR void assign(size_type count, const value_type& value)
    {
        if (count > N) {
            throw std::bad_alloc();
        }
        if (count < size_) {
            std::fill_n(ptr_at(0), count, value);
            ccc::destroy(ptr_at(count), ptr_at(size_));
        }
        else {
            std::fill_n(ptr_at(0), size_, value);
            std::uninitialized_fill_n(ptr_at(size_), count - size_, value);
        }
        size_ = count;
    }

    template<typename InputIt, typename = ccc::enable_if_t<!std::is_integral<InputIt>::value>>
    CCC_CPP20_CONSTEXPR void assign(InputIt first, InputIt last)
    {
        const size_type count = static_cast<size_type>(std::distance(first, last));
        if (count > N) {
            throw std::bad_alloc();
        }
        if (count < size_) {
            std::copy(first, last, ptr_at(0));
            ccc::destroy(ptr_at(count), ptr_at(size_));
        }
        else {
            const InputIt mid = std::next(first, static_cast<difference_type>(size_));
            std::copy(first, mid, ptr_at(0));
            std::uninitialized_copy(mid, last, ptr_at(size_));
        }
        size_ = count;
    }

    CCC_CPP20_CONSTEXPR void assign(std::initializer_list<T> ilist)
    {
        assign(ilist.begin(), ilist.end());
    }

    template<typename Range>
    CCC_CPP20_CONSTEXPR void assign_range(Range&& range)
    {
        assign(std::begin(range), std::end(range));
    }

    // ---------- resize ----------

    CCC_CPP20_CONSTEXPR void resize(size_type new_size)
    {
        if (new_size > N) {
            throw std::out_of_range("[ccc.inplace_vector]: new_size too large");
        }
        if (new_size > size_) {
            if CCC_CPP17_CONSTEXPR (!is_trivially_default_constructible_v<value_type>) {
#if (__cplusplus >= 201703L)
                std::uninitialized_default_construct_n(ptr_at(size_), new_size - size_);
#else
                for (size_type i = size_; i < new_size; ++i) {
                    construct_at(ptr_at(i));
                }
#endif
            }
        }
        else if (new_size < size_) {
            ccc::destroy(ptr_at(new_size), ptr_at(size_));
        }
        size_ = new_size;
    }

    CCC_CPP20_CONSTEXPR void resize(size_type new_size, const value_type& value)
    {
        if (new_size > N) {
            throw std::out_of_range("[ccc.inplace_vector]: new_size too large");
        }
        if (new_size > size_) {
            std::uninitialized_fill_n(ptr_at(size_), new_size - size_, value);
        }
        else if (new_size < size_) {
            ccc::destroy(ptr_at(new_size), ptr_at(size_));
        }
        size_ = new_size;
    }

    // ---------- clear ----------

    CCC_CPP20_CONSTEXPR void clear() noexcept
    {
        ccc::destroy(begin(), end());
        size_ = 0;
    }

    // ---------- push_back ----------

    CCC_CPP20_CONSTEXPR reference push_back(const value_type& value)
    {
        if (full()) {
            throw std::bad_alloc();
        }
        construct_at(ptr_at(size_), value);
        ++size_;
        return back();
    }

    CCC_CPP20_CONSTEXPR reference push_back(value_type&& value)
    {
        if (full()) {
            throw std::bad_alloc();
        }
        construct_at(ptr_at(size_), std::move(value));
        ++size_;
        return back();
    }

    CCC_CPP20_CONSTEXPR pointer unchecked_push_back(const value_type& value)
    {
        construct_at(ptr_at(size_), value);
        ++size_;
        return std::addressof(back());
    }

    CCC_CPP20_CONSTEXPR pointer unchecked_push_back(value_type&& value)
    {
        construct_at(ptr_at(size_), std::move(value));
        ++size_;
        return std::addressof(back());
    }

    CCC_CPP20_CONSTEXPR pointer try_push_back(const value_type& value)
    {
        if (full()) {
            return nullptr;
        }
        return unchecked_push_back(value);
    }

    CCC_CPP20_CONSTEXPR pointer try_push_back(value_type&& value)
    {
        if (full()) {
            return nullptr;
        }
        return unchecked_push_back(std::move(value));
    }

    // ---------- emplace_back ----------

    template<typename... Args>
    CCC_CPP20_CONSTEXPR reference emplace_back(Args&&... args)
    {
        if (full()) {
            throw std::bad_alloc();
        }
        construct_at(ptr_at(size_), std::forward<Args>(args)...);
        ++size_;
        return back();
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR pointer unchecked_emplace_back(Args&&... args)
    {
        construct_at(ptr_at(size_), std::forward<Args>(args)...);
        ++size_;
        return std::addressof(back());
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR pointer try_emplace_back(Args&&... args)
    {
        if (full()) {
            return nullptr;
        }
        return unchecked_emplace_back(std::forward<Args>(args)...);
    }

    // ---------- pop_back ----------

    CCC_CPP20_CONSTEXPR void pop_back() noexcept
    {
        if (!empty()) {
            --size_;
            ccc::destroy_at(ptr_at(size_));
        }
    }

    // ---------- emplace ----------

    template<typename... Args>
    CCC_CPP20_CONSTEXPR iterator emplace(const_iterator pos, Args&&... args)
    {
        const auto index = static_cast<size_type>(std::distance(cbegin(), pos));
        CCC_DETAIL_ASSERT(cbegin() <= pos && pos <= cend(), "[ccc.inplace_vector]: iterator out of range");
        if (full()) {
            throw std::bad_alloc();
        }
        if (index == size_) {
            construct_at(ptr_at(size_), std::forward<Args>(args)...);
            ++size_;
            return ptr_at(index);
        }
        // Move the last element into uninitialised storage
        construct_at(ptr_at(size_), std::move(*ptr_at(size_ - 1)));
        // Shift the middle range right by one
        std::move_backward(ptr_at(index), ptr_at(size_ - 1), ptr_at(size_));
        // Replace element at index
        ccc::destroy_at(ptr_at(index));
        construct_at(ptr_at(index), std::forward<Args>(args)...);
        ++size_;
        return ptr_at(index);
    }

    // ---------- insert (single element) ----------

    CCC_CPP20_CONSTEXPR iterator insert(const_iterator pos, const value_type& value)
    {
        return emplace(pos, value);
    }

    CCC_CPP20_CONSTEXPR iterator insert(const_iterator pos, value_type&& value)
    {
        return emplace(pos, std::move(value));
    }

    // ---------- insert (count copies) ----------

    CCC_CPP20_CONSTEXPR iterator insert(const_iterator pos, size_type count, const value_type& value)
    {
        const auto index = static_cast<size_type>(std::distance(cbegin(), pos));
        CCC_DETAIL_ASSERT(cbegin() <= pos && pos <= cend(), "[ccc.inplace_vector]: iterator out of range");
        if (count == 0) {
            return ptr_at(index);
        }
        if (size_ + count > N) {
            throw std::bad_alloc();
        }

        const size_type tail = size_ - index;
        const size_type uninit = (std::min)(tail, count);

        // Move-construct elements that land in uninitialised storage (back to front)
        for (size_type i = 0; i < uninit; ++i) {
            construct_at(ptr_at(size_ + count - 1 - i), std::move(*ptr_at(size_ - 1 - i)));
        }
        // Move-assign the remaining tail elements backward
        if (tail > uninit) {
            std::move_backward(ptr_at(index), ptr_at(size_ - uninit), ptr_at(size_ + count - uninit));
        }

        // Fill the gap: assign to existing slots, construct in new slots
        const size_type fill_existing = (std::min)(count, size_ - index);
        if (fill_existing > 0) {
            std::fill_n(ptr_at(index), fill_existing, value);
        }
        if (count > fill_existing) {
            std::uninitialized_fill_n(ptr_at(size_), count - fill_existing, value);
        }

        size_ += count;
        return ptr_at(index);
    }

    // ---------- insert (range) ----------

    template<typename InputIt, typename = ccc::enable_if_t<!std::is_integral<InputIt>::value>>
    CCC_CPP20_CONSTEXPR iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        const auto index = static_cast<size_type>(std::distance(cbegin(), pos));
        CCC_DETAIL_ASSERT(cbegin() <= pos && pos <= cend(), "[ccc.inplace_vector]: iterator out of range");
        const auto count = static_cast<size_type>(std::distance(first, last));
        if (count == 0) {
            return ptr_at(index);
        }
        if (size_ + count > N) {
            throw std::bad_alloc();
        }

        const size_type tail = size_ - index;
        const size_type uninit = (std::min)(tail, count);

        for (size_type i = 0; i < uninit; ++i) {
            construct_at(ptr_at(size_ + count - 1 - i), std::move(*ptr_at(size_ - 1 - i)));
        }
        if (tail > uninit) {
            std::move_backward(ptr_at(index), ptr_at(size_ - uninit), ptr_at(size_ + count - uninit));
        }

        const size_type fill_existing = (std::min)(count, size_ - index);
        if (fill_existing > 0) {
            std::copy_n(first, fill_existing, ptr_at(index));
        }
        if (count > fill_existing) {
            std::uninitialized_copy(std::next(first, static_cast<difference_type>(fill_existing)), last, ptr_at(size_));
        }

        size_ += count;
        return ptr_at(index);
    }

    CCC_CPP20_CONSTEXPR iterator insert(const_iterator pos, std::initializer_list<T> ilist)
    {
        return insert(pos, ilist.begin(), ilist.end());
    }

    template<typename Range>
    CCC_CPP20_CONSTEXPR iterator insert_range(const_iterator pos, Range&& range)
    {
        return insert(pos, std::begin(range), std::end(range));
    }

    // ---------- append_range / try_append_range ----------

    template<typename Range>
    CCC_CPP20_CONSTEXPR void append_range(Range&& range)
    {
        insert(cend(), std::begin(range), std::end(range));
    }

    template<typename Range>
    CCC_CPP20_CONSTEXPR bool try_append_range(Range&& range)
    {
        const auto r_size = static_cast<size_type>(std::distance(std::begin(range), std::end(range)));
        if (size_ + r_size > N) {
            return false;
        }
        append_range(std::forward<Range>(range));
        return true;
    }

    // ---------- erase ----------

    CCC_CPP20_CONSTEXPR iterator erase(const_iterator pos)
    {
        return erase(pos, pos + 1);
    }

    CCC_CPP20_CONSTEXPR iterator erase(const_iterator first, const_iterator last)
    {
        CCC_DETAIL_ASSERT(cbegin() <= first && first <= last && last <= cend(),
                          "[ccc.inplace_vector]: erase range out of range");
        const auto first_idx = static_cast<size_type>(std::distance(cbegin(), first));
        const auto last_idx = static_cast<size_type>(std::distance(cbegin(), last));
        if (first_idx == last_idx) {
            return ptr_at(first_idx);
        }

        iterator new_end = std::move(ptr_at(last_idx), ptr_at(size_), ptr_at(first_idx));
        ccc::destroy(new_end, ptr_at(size_));
        size_ -= (last_idx - first_idx);
        return ptr_at(first_idx);
    }

    // ---------- swap ----------

    CCC_CPP20_CONSTEXPR void swap(inplace_vector& other) noexcept(ccc::is_nothrow_swappable<T>::value &&
                                                                  std::is_nothrow_move_constructible<T>::value)
    {
        using std::swap;
        const size_type common = (std::min)(size_, other.size_);

        std::swap_ranges(ptr_at(0), ptr_at(common), other.ptr_at(0));

        if (size_ > other.size_) {
            std::uninitialized_copy(std::make_move_iterator(ptr_at(common)),
                                    std::make_move_iterator(ptr_at(size_)),
                                    other.ptr_at(common));
            ccc::destroy(ptr_at(common), ptr_at(size_));
        }
        else if (other.size_ > size_) {
            std::uninitialized_copy(std::make_move_iterator(other.ptr_at(common)),
                                    std::make_move_iterator(other.ptr_at(other.size_)),
                                    ptr_at(common));
            ccc::destroy(other.ptr_at(common), other.ptr_at(other.size_));
        }

        swap(size_, other.size_);
    }

    // ---------- Comparisons ----------

#if (__cplusplus >= 202002L)

    friend auto operator<=>(const inplace_vector& a, const inplace_vector& b)
    {
        return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end());
    }

    friend bool operator==(const inplace_vector& a, const inplace_vector& b)
    {
        return a.size_ == b.size_ && std::equal(a.begin(), a.end(), b.begin());
    }

#else

    friend bool operator==(const inplace_vector& a, const inplace_vector& b)
    {
        return a.size_ == b.size_ && std::equal(a.begin(), a.end(), b.begin());
    }
    friend bool operator!=(const inplace_vector& a, const inplace_vector& b)
    {
        return !(a == b);
    }
    friend bool operator<(const inplace_vector& a, const inplace_vector& b)
    {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
    }
    friend bool operator>(const inplace_vector& a, const inplace_vector& b)
    {
        return b < a;
    }
    friend bool operator<=(const inplace_vector& a, const inplace_vector& b)
    {
        return !(b < a);
    }
    friend bool operator>=(const inplace_vector& a, const inplace_vector& b)
    {
        return !(a < b);
    }

#endif
};

// ---------- Non-member swap ----------

template<typename T, std::size_t N>
CCC_CPP20_CONSTEXPR void swap(inplace_vector<T, N>& a, inplace_vector<T, N>& b) noexcept(noexcept(a.swap(b)))
{
    a.swap(b);
}

// ---------- Non-member erase / erase_if ----------

template<typename T, std::size_t N, typename U>
CCC_CPP20_CONSTEXPR typename inplace_vector<T, N>::size_type erase(inplace_vector<T, N>& c, const U& value)
{
    auto it = std::remove(c.begin(), c.end(), value);
    auto r = static_cast<typename inplace_vector<T, N>::size_type>(std::distance(it, c.end()));
    c.erase(it, c.end());
    return r;
}

template<typename T, std::size_t N, typename Pred>
CCC_CPP20_CONSTEXPR typename inplace_vector<T, N>::size_type erase_if(inplace_vector<T, N>& c, Pred pred)
{
    auto it = std::remove_if(c.begin(), c.end(), pred);
    auto r = static_cast<typename inplace_vector<T, N>::size_type>(std::distance(it, c.end()));
    c.erase(it, c.end());
    return r;
}

CCC_MODULE_EXPORT_END

}  // namespace ccc

#endif  // !CCC_INPLACE_VECTOR_HH
