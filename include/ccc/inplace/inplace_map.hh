/**
 * @file inplace_map.hh
 * @brief Fixed-capacity ordered map with in-place storage (C++11 minimum).
 */

#pragma once
#ifndef CCC_INPLACE_INPLACE_MAP_HH
#define CCC_INPLACE_INPLACE_MAP_HH

#include "ccc/detail/config.hh"

#ifndef CCC_MODULE_INTERFACE_UNIT
#include <cstddef>
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

#if (__cplusplus >= 202002L)
#include <compare>
#endif

#include "ccc/utility.hh"
#include "ccc/inplace/inplace_vector.hh"
#endif

namespace ccc {

CCC_MODULE_EXPORT
template<typename Key, typename T, std::size_t N, typename Compare, typename KeyContainer, typename MappedContainer>
class inplace_map;

namespace detail {

template<typename ValueType>
struct inplace_map_arrow_proxy {
    ValueType value_;
    CCC_CPP20_CONSTEXPR const ValueType* operator->() const noexcept
    {
        return &value_;
    }
    CCC_CPP20_CONSTEXPR ValueType* operator->() noexcept
    {
        return &value_;
    }
};

template<typename Key,
         typename T,
         std::size_t N,
         typename Compare,
         typename KeyContainer,
         typename MappedContainer,
         bool IsConst>
class inplace_map_iterator
{
    friend class ccc::inplace_map<Key, T, N, Compare, KeyContainer, MappedContainer>;

    template<typename, typename, std::size_t, typename, typename, typename, bool>
    friend class inplace_map_iterator;

    using map_ptr_ =
        typename ccc::conditional_t<IsConst,
                                    const ccc::inplace_map<Key, T, N, Compare, KeyContainer, MappedContainer>*,
                                    ccc::inplace_map<Key, T, N, Compare, KeyContainer, MappedContainer>*>;

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::pair<const Key, T>;
    using difference_type = std::ptrdiff_t;
    using reference = std::pair<const Key&, typename ccc::conditional_t<IsConst, const T&, T&>>;
    using pointer = inplace_map_arrow_proxy<value_type>;

    using ptr_type = map_ptr_;

private:
    map_ptr_ map_;
    std::size_t idx_;

    CCC_CPP20_CONSTEXPR inplace_map_iterator(map_ptr_ m, std::size_t i) noexcept : map_(m), idx_(i) {}

public:
    CCC_CPP20_CONSTEXPR inplace_map_iterator() noexcept : map_(nullptr), idx_(0) {}

    template<bool OtherConst, typename = ccc::enable_if_t<IsConst && !OtherConst>>
    CCC_CPP20_CONSTEXPR inplace_map_iterator(
        const inplace_map_iterator<Key, T, N, Compare, KeyContainer, MappedContainer, OtherConst>& other) noexcept
        : map_(other.ptr()), idx_(other.index())
    {
    }

    CCC_CPP20_CONSTEXPR reference operator*() const
    {
        return reference(map_->keys_[idx_], map_->values_[idx_]);
    }

    CCC_CPP20_CONSTEXPR pointer operator->() const
    {
        return pointer{{map_->keys_[idx_], map_->values_[idx_]}};
    }

    CCC_CPP20_CONSTEXPR reference operator[](difference_type n) const
    {
        const auto i = static_cast<std::size_t>(static_cast<difference_type>(idx_) + n);
        return reference(map_->keys_[i], map_->values_[i]);
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator& operator++() noexcept
    {
        ++idx_;
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator operator++(int) noexcept
    {
        auto t = *this;
        ++idx_;
        return t;
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator& operator--() noexcept
    {
        --idx_;
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator operator--(int) noexcept
    {
        auto t = *this;
        --idx_;
        return t;
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator& operator+=(difference_type n) noexcept
    {
        idx_ = static_cast<std::size_t>(static_cast<difference_type>(idx_) + n);
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator& operator-=(difference_type n) noexcept
    {
        idx_ = static_cast<std::size_t>(static_cast<difference_type>(idx_) - n);
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator operator+(difference_type n) const noexcept
    {
        return inplace_map_iterator(map_, static_cast<std::size_t>(static_cast<difference_type>(idx_) + n));
    }

    CCC_CPP20_CONSTEXPR inplace_map_iterator operator-(difference_type n) const noexcept
    {
        return inplace_map_iterator(map_, static_cast<std::size_t>(static_cast<difference_type>(idx_) - n));
    }

    CCC_CPP20_CONSTEXPR difference_type operator-(const inplace_map_iterator& o) const noexcept
    {
        return static_cast<difference_type>(idx_) - static_cast<difference_type>(o.idx_);
    }

    CCC_CPP20_CONSTEXPR bool operator==(const inplace_map_iterator& o) const noexcept
    {
        return idx_ == o.idx_;
    }

    CCC_CPP20_CONSTEXPR bool operator!=(const inplace_map_iterator& o) const noexcept
    {
        return idx_ != o.idx_;
    }

    CCC_CPP20_CONSTEXPR bool operator<(const inplace_map_iterator& o) const noexcept
    {
        return idx_ < o.idx_;
    }

    CCC_CPP20_CONSTEXPR bool operator<=(const inplace_map_iterator& o) const noexcept
    {
        return idx_ <= o.idx_;
    }

    CCC_CPP20_CONSTEXPR bool operator>(const inplace_map_iterator& o) const noexcept
    {
        return idx_ > o.idx_;
    }

    CCC_CPP20_CONSTEXPR bool operator>=(const inplace_map_iterator& o) const noexcept
    {
        return idx_ >= o.idx_;
    }

    CCC_CPP20_CONSTEXPR ptr_type ptr() const noexcept
    {
        return map_;
    }

    CCC_CPP20_CONSTEXPR std::size_t index() const noexcept
    {
        return idx_;
    }

    friend CCC_CPP20_CONSTEXPR inplace_map_iterator operator+(difference_type n,
                                                              const inplace_map_iterator& it) noexcept
    {
        return it + n;
    }
};

}  // namespace detail

CCC_MODULE_EXPORT_BEGIN

template<typename Key,
         typename T,
         std::size_t N,
         typename Compare = std::less<Key>,
         typename KeyContainer = ccc::inplace_vector<Key, N>,
         typename MappedContainer = ccc::inplace_vector<T, N>>
class inplace_map
{
public:
    static_assert(N > 0, "inplace_map capacity must be greater than zero");

    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const key_type, mapped_type>;
    using key_compare = Compare;
    using reference = std::pair<const key_type&, mapped_type&>;
    using const_reference = std::pair<const key_type&, const mapped_type&>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = detail::inplace_map_iterator<Key, T, N, Compare, KeyContainer, MappedContainer, false>;
    using const_iterator = detail::inplace_map_iterator<Key, T, N, Compare, KeyContainer, MappedContainer, true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using key_container_type = KeyContainer;
    using mapped_container_type = MappedContainer;

    struct value_compare {
        friend class inplace_map;
        key_compare comp_;
        value_compare(key_compare c) : comp_(c) {}

    public:
        using is_transparent = void;
        CCC_CPP20_CONSTEXPR bool operator()(const value_type& a, const value_type& b) const
        {
            return comp_(a.first, b.first);
        }
    };

private:
    key_container_type keys_;
    mapped_container_type values_;
    CCC_MAYBE_UNUSED key_compare comp_;

    friend class detail::inplace_map_iterator<Key, T, N, Compare, KeyContainer, MappedContainer, false>;
    friend class detail::inplace_map_iterator<Key, T, N, Compare, KeyContainer, MappedContainer, true>;

    CCC_CPP20_CONSTEXPR const_iterator lower_bound_impl_(const key_type& key) const
    {
        return const_iterator(
            this,
            static_cast<size_type>(
                std::distance(keys_.cbegin(), std::lower_bound(keys_.cbegin(), keys_.cend(), key, comp_))));
    }

    CCC_CPP20_CONSTEXPR iterator lower_bound_impl_(const key_type& key)
    {
        return iterator(this,
                        static_cast<size_type>(
                            std::distance(keys_.cbegin(), std::lower_bound(keys_.cbegin(), keys_.cend(), key, comp_))));
    }

    CCC_CPP20_CONSTEXPR const_iterator find_impl_(const key_type& key) const
    {
        auto it = lower_bound_impl_(key);
        if (it != cend() && !comp_(key, keys_[it.index()])) {
            return it;
        }
        return cend();
    }

    CCC_CPP20_CONSTEXPR iterator find_impl_(const key_type& key)
    {
        auto it = lower_bound_impl_(key);
        if (it != end() && !comp_(key, keys_[it.index()])) {
            return it;
        }
        return end();
    }

public:
    CCC_NO_DISCARD static constexpr size_type capacity() noexcept
    {
        return N;
    }
    CCC_NO_DISCARD static constexpr size_type max_size() noexcept
    {
        return N;
    }
    CCC_NO_DISCARD CCC_CPP20_CONSTEXPR size_type size() const noexcept
    {
        return keys_.size();
    }
    CCC_NO_DISCARD constexpr bool empty() const noexcept
    {
        return keys_.empty();
    }
    CCC_NO_DISCARD constexpr bool full() const noexcept
    {
        return keys_.full();
    }

    CCC_CPP20_CONSTEXPR inplace_map() : comp_() {}
    CCC_CPP20_CONSTEXPR explicit inplace_map(const key_compare& comp) : comp_(comp) {}

    template<typename InputIt>
    CCC_CPP20_CONSTEXPR inplace_map(InputIt first, InputIt last) : comp_()
    {
        insert(first, last);
    }

    template<typename InputIt>
    CCC_CPP20_CONSTEXPR inplace_map(InputIt first, InputIt last, const key_compare& comp) : comp_(comp)
    {
        insert(first, last);
    }

    CCC_CPP20_CONSTEXPR inplace_map(std::initializer_list<value_type> ilist) : comp_()
    {
        insert(ilist);
    }

    CCC_CPP20_CONSTEXPR inplace_map(std::initializer_list<value_type> ilist, const key_compare& comp) : comp_(comp)
    {
        insert(ilist);
    }

    CCC_CPP20_CONSTEXPR inplace_map(const inplace_map& other)
        : keys_(other.keys_), values_(other.values_), comp_(other.comp_)
    {
    }

    CCC_CPP20_CONSTEXPR inplace_map(inplace_map&& other) noexcept(
        std::is_nothrow_move_constructible<key_container_type>::value &&
        std::is_nothrow_move_constructible<mapped_container_type>::value)
        : keys_(std::move(other.keys_)), values_(std::move(other.values_)), comp_(std::move(other.comp_))
    {
    }

    CCC_CPP20_CONSTEXPR ~inplace_map() = default;

    CCC_CPP20_CONSTEXPR inplace_map& operator=(const inplace_map& other)
    {
        if (this == &other) {
            return *this;
        }
        keys_ = other.keys_;
        values_ = other.values_;
        comp_ = other.comp_;
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_map& operator=(inplace_map&& other) noexcept(
        std::is_nothrow_move_assignable<key_container_type>::value &&
        std::is_nothrow_move_assignable<mapped_container_type>::value)
    {
        if (this == &other) {
            return *this;
        }
        keys_ = std::move(other.keys_);
        values_ = std::move(other.values_);
        comp_ = std::move(other.comp_);
        return *this;
    }

    CCC_CPP20_CONSTEXPR inplace_map& operator=(std::initializer_list<value_type> ilist)
    {
        clear();
        insert(ilist);
        return *this;
    }

    CCC_CPP20_CONSTEXPR mapped_type& at(const key_type& key)
    {
        auto it = find_impl_(key);
        if (it == end()) {
            throw std::out_of_range("[ccc.inplace_map]: key not found");
        }
        return values_[it.index()];
    }

    CCC_CPP20_CONSTEXPR const mapped_type& at(const key_type& key) const
    {
        auto it = find_impl_(key);
        if (it == cend()) {
            throw std::out_of_range("[ccc.inplace_map]: key not found");
        }
        return values_[it.index()];
    }

    CCC_CPP20_CONSTEXPR mapped_type& operator[](const key_type& key)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            return values_[idx];
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), key);
        values_.insert(values_.cbegin() + static_cast<difference_type>(idx), mapped_type{});
        return values_[idx];
    }

    CCC_CPP20_CONSTEXPR mapped_type& operator[](key_type&& key)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            return values_[idx];
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), std::move(key));
        values_.insert(values_.cbegin() + static_cast<difference_type>(idx), mapped_type{});
        return values_[idx];
    }

    CCC_CPP20_CONSTEXPR iterator begin() noexcept
    {
        return iterator(this, 0);
    }
    CCC_CPP20_CONSTEXPR const_iterator begin() const noexcept
    {
        return const_iterator(this, 0);
    }
    CCC_CPP20_CONSTEXPR const_iterator cbegin() const noexcept
    {
        return const_iterator(this, 0);
    }
    CCC_CPP20_CONSTEXPR iterator end() noexcept
    {
        return iterator(this, keys_.size());
    }
    CCC_CPP20_CONSTEXPR const_iterator end() const noexcept
    {
        return const_iterator(this, keys_.size());
    }
    CCC_CPP20_CONSTEXPR const_iterator cend() const noexcept
    {
        return const_iterator(this, keys_.size());
    }
    CCC_CPP20_CONSTEXPR reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }
    CCC_CPP20_CONSTEXPR const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    CCC_CPP20_CONSTEXPR const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cend());
    }
    CCC_CPP20_CONSTEXPR reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }
    CCC_CPP20_CONSTEXPR const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    CCC_CPP20_CONSTEXPR const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }

    CCC_CPP20_CONSTEXPR void clear() noexcept
    {
        keys_.clear();
        values_.clear();
    }

    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> insert(const value_type& value)
    {
        return insert_impl_(value.first, value.second);
    }

    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> insert(value_type&& value)
    {
        return insert_impl_(std::move(value.first), std::move(value.second));
    }

    CCC_CPP20_CONSTEXPR iterator insert(const_iterator hint, const value_type& value)
    {
        return insert_hint_impl_(hint, value.first, value.second);
    }

    CCC_CPP20_CONSTEXPR iterator insert(const_iterator hint, value_type&& value)
    {
        return insert_hint_impl_(hint, std::move(value.first), std::move(value.second));
    }

    template<typename InputIt>
    CCC_CPP20_CONSTEXPR void insert(InputIt first, InputIt last)
    {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    CCC_CPP20_CONSTEXPR void insert(std::initializer_list<value_type> ilist)
    {
        insert(ilist.begin(), ilist.end());
    }

    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(const key_type& key, mapped_type&& value)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            values_[idx] = std::move(value);
            return {iterator(this, idx), false};
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), key);
        values_.insert(values_.cbegin() + static_cast<difference_type>(idx), std::move(value));
        return {iterator(this, idx), true};
    }

    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(key_type&& key, mapped_type&& value)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            values_[idx] = std::move(value);
            return {iterator(this, idx), false};
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), std::move(key));
        values_.insert(values_.cbegin() + static_cast<difference_type>(idx), std::move(value));
        return {iterator(this, idx), true};
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> emplace(key_type&& key, Args&&... args)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            return {iterator(this, idx), false};
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), std::move(key));
        values_.emplace(values_.cbegin() + static_cast<difference_type>(idx), std::forward<Args>(args)...);
        return {iterator(this, idx), true};
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR iterator emplace_hint(const_iterator hint, key_type&& key, Args&&... args)
    {
        if (hint != cend()) {
            const auto h_idx = hint.index();
            if ((h_idx == 0 || comp_(keys_[h_idx - 1], key)) && (h_idx == keys_.size() || !comp_(key, keys_[h_idx]))) {
                if (h_idx < keys_.size() && !comp_(key, keys_[h_idx]) && !comp_(keys_[h_idx], key)) {
                    return iterator(this, h_idx);
                }
                if (!keys_.full()) {
                    keys_.insert(keys_.cbegin() + static_cast<difference_type>(h_idx), std::move(key));
                    values_.emplace(values_.cbegin() + static_cast<difference_type>(h_idx),
                                    std::forward<Args>(args)...);
                    return iterator(this, h_idx);
                }
                throw std::bad_alloc();
            }
        }
        return emplace(std::move(key), std::forward<Args>(args)...).first;
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            return {iterator(this, idx), false};
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), key);
        values_.emplace(values_.cbegin() + static_cast<difference_type>(idx), std::forward<Args>(args)...);
        return {iterator(this, idx), true};
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            return {iterator(this, idx), false};
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), std::move(key));
        values_.emplace(values_.cbegin() + static_cast<difference_type>(idx), std::forward<Args>(args)...);
        return {iterator(this, idx), true};
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR iterator try_emplace(const_iterator hint, const key_type& key, Args&&... args)
    {
        if (hint != cend()) {
            const auto h_idx = hint.index();
            if ((h_idx == 0 || comp_(keys_[h_idx - 1], key)) && (h_idx == keys_.size() || !comp_(key, keys_[h_idx]))) {
                if (h_idx < keys_.size() && !comp_(key, keys_[h_idx]) && !comp_(keys_[h_idx], key)) {
                    return iterator(this, h_idx);
                }
                if (!keys_.full()) {
                    keys_.insert(keys_.cbegin() + static_cast<difference_type>(h_idx), key);
                    values_.emplace(values_.cbegin() + static_cast<difference_type>(h_idx),
                                    std::forward<Args>(args)...);
                    return iterator(this, h_idx);
                }
                throw std::bad_alloc();
            }
        }
        return try_emplace(key, std::forward<Args>(args)...).first;
    }

    template<typename... Args>
    CCC_CPP20_CONSTEXPR iterator try_emplace(const_iterator hint, key_type&& key, Args&&... args)
    {
        if (hint != cend()) {
            const auto h_idx = hint.index();
            if ((h_idx == 0 || comp_(keys_[h_idx - 1], key)) && (h_idx == keys_.size() || !comp_(key, keys_[h_idx]))) {
                if (h_idx < keys_.size() && !comp_(key, keys_[h_idx]) && !comp_(keys_[h_idx], key)) {
                    return iterator(this, h_idx);
                }
                if (!keys_.full()) {
                    keys_.insert(keys_.cbegin() + static_cast<difference_type>(h_idx), std::move(key));
                    values_.emplace(values_.cbegin() + static_cast<difference_type>(h_idx),
                                    std::forward<Args>(args)...);
                    return iterator(this, h_idx);
                }
                throw std::bad_alloc();
            }
        }
        return try_emplace(std::move(key), std::forward<Args>(args)...).first;
    }

    CCC_CPP20_CONSTEXPR size_type erase(const key_type& key)
    {
        auto it = find_impl_(key);
        if (it == end()) {
            return 0;
        }
        const auto idx = it.index();
        keys_.erase(keys_.cbegin() + static_cast<difference_type>(idx));
        values_.erase(values_.cbegin() + static_cast<difference_type>(idx));
        return 1;
    }

    CCC_CPP20_CONSTEXPR iterator erase(const_iterator pos)
    {
        const auto idx = pos.index();
        keys_.erase(keys_.cbegin() + static_cast<difference_type>(idx));
        values_.erase(values_.cbegin() + static_cast<difference_type>(idx));
        return iterator(this, idx);
    }

    CCC_CPP20_CONSTEXPR iterator erase(const_iterator first, const_iterator last)
    {
        const auto f_idx = first.index();
        const auto l_idx = last.index();
        if (f_idx == l_idx) {
            return iterator(this, f_idx);
        }
        keys_.erase(keys_.cbegin() + static_cast<difference_type>(f_idx),
                    keys_.cbegin() + static_cast<difference_type>(l_idx));
        values_.erase(values_.cbegin() + static_cast<difference_type>(f_idx),
                      values_.cbegin() + static_cast<difference_type>(l_idx));
        return iterator(this, f_idx);
    }

    CCC_CPP20_CONSTEXPR void swap(inplace_map& other) noexcept(ccc::is_nothrow_swappable<key_container_type>::value &&
                                                               ccc::is_nothrow_swappable<mapped_container_type>::value)
    {
        using std::swap;
        swap(keys_, other.keys_);
        swap(values_, other.values_);
        swap(comp_, other.comp_);
    }

    friend CCC_CPP20_CONSTEXPR void swap(inplace_map& a, inplace_map& b) noexcept(noexcept(a.swap(b)))
    {
        a.swap(b);
    }

    CCC_CPP20_CONSTEXPR size_type count(const key_type& key) const
    {
        return contains(key) ? 1 : 0;
    }

    CCC_CPP20_CONSTEXPR bool contains(const key_type& key) const
    {
        return find_impl_(key) != cend();
    }

    CCC_CPP20_CONSTEXPR iterator find(const key_type& key)
    {
        return find_impl_(key);
    }
    CCC_CPP20_CONSTEXPR const_iterator find(const key_type& key) const
    {
        return find_impl_(key);
    }
    CCC_CPP20_CONSTEXPR iterator lower_bound(const key_type& key)
    {
        return lower_bound_impl_(key);
    }
    CCC_CPP20_CONSTEXPR const_iterator lower_bound(const key_type& key) const
    {
        return lower_bound_impl_(key);
    }

    CCC_CPP20_CONSTEXPR iterator upper_bound(const key_type& key)
    {
        auto it = lower_bound_impl_(key);
        if (it != end() && !comp_(key, keys_[it.index()])) {
            ++it;
        }
        return it;
    }

    CCC_CPP20_CONSTEXPR const_iterator upper_bound(const key_type& key) const
    {
        auto it = lower_bound_impl_(key);
        if (it != cend() && !comp_(key, keys_[it.index()])) {
            ++it;
        }
        return it;
    }

    CCC_CPP20_CONSTEXPR std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        auto lb = lower_bound(key);
        auto ub = lb;
        if (ub != end() && !comp_(key, keys_[ub.index()])) {
            ++ub;
        }
        return {lb, ub};
    }

    CCC_CPP20_CONSTEXPR std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        auto lb = lower_bound(key);
        auto ub = lb;
        if (ub != cend() && !comp_(key, keys_[ub.index()])) {
            ++ub;
        }
        return {lb, ub};
    }

    CCC_CPP20_CONSTEXPR key_compare key_comp() const
    {
        return comp_;
    }
    CCC_CPP20_CONSTEXPR value_compare value_comp() const
    {
        return value_compare(comp_);
    }
    CCC_CPP20_CONSTEXPR const key_container_type& keys() const noexcept
    {
        return keys_;
    }
    CCC_CPP20_CONSTEXPR const mapped_container_type& values() const noexcept
    {
        return values_;
    }

#if (__cplusplus >= 202002L)

    friend auto operator<=>(const inplace_map& a, const inplace_map& b)
    {
        return std::lexicographical_compare_three_way(a.cbegin(), a.cend(), b.cbegin(), b.cend());
    }

    friend bool operator==(const inplace_map& a, const inplace_map& b)
    {
        return a.size() == b.size() && std::equal(a.cbegin(), a.cend(), b.cbegin());
    }

#else

    friend bool operator==(const inplace_map& a, const inplace_map& b)
    {
        return a.size() == b.size() && std::equal(a.cbegin(), a.cend(), b.cbegin());
    }

    friend bool operator!=(const inplace_map& a, const inplace_map& b)
    {
        return !(a == b);
    }

    friend bool operator<(const inplace_map& a, const inplace_map& b)
    {
        return std::lexicographical_compare(a.cbegin(), a.cend(), b.cbegin(), b.cend());
    }

    friend bool operator>(const inplace_map& a, const inplace_map& b)
    {
        return b < a;
    }

    friend bool operator<=(const inplace_map& a, const inplace_map& b)
    {
        return !(b < a);
    }

    friend bool operator>=(const inplace_map& a, const inplace_map& b)
    {
        return !(a < b);
    }

#endif

private:
    template<typename K, typename V>
    CCC_CPP20_CONSTEXPR std::pair<iterator, bool> insert_impl_(K&& key, V&& value)
    {
        auto it = lower_bound_impl_(key);
        const auto idx = it.index();
        if (idx < keys_.size() && !comp_(key, keys_[idx])) {
            return {iterator(this, idx), false};
        }
        if (keys_.full()) {
            throw std::bad_alloc();
        }
        keys_.insert(keys_.cbegin() + static_cast<difference_type>(idx), std::forward<K>(key));
        values_.insert(values_.cbegin() + static_cast<difference_type>(idx), std::forward<V>(value));
        return {iterator(this, idx), true};
    }

    template<typename K, typename V>
    CCC_CPP20_CONSTEXPR iterator insert_hint_impl_(const_iterator hint, K&& key, V&& value)
    {
        if (hint != cend()) {
            const auto h_idx = hint.index();
            if ((h_idx == 0 || comp_(keys_[h_idx - 1], key)) && (h_idx == keys_.size() || !comp_(key, keys_[h_idx]))) {
                if (h_idx < keys_.size() && !comp_(key, keys_[h_idx]) && !comp_(keys_[h_idx], key)) {
                    return iterator(this, h_idx);
                }
                if (!keys_.full()) {
                    keys_.insert(keys_.cbegin() + static_cast<difference_type>(h_idx), std::forward<K>(key));
                    values_.insert(values_.cbegin() + static_cast<difference_type>(h_idx), std::forward<V>(value));
                    return iterator(this, h_idx);
                }
                throw std::bad_alloc();
            }
        }
        return insert_impl_(std::forward<K>(key), std::forward<V>(value)).first;
    }
};

CCC_MODULE_EXPORT_END

}  // namespace ccc

#endif  // !CCC_INPLACE_INPLACE_MAP_HH
