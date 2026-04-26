/**
 * @file criterion_traits.hh
 * @brief
 */

#pragma once
#ifndef CCC_EXPECTED_CRITERION_TRAITS_HH
#define CCC_EXPECTED_CRITERION_TRAITS_HH

namespace ccc {

template<typename T, typename = void_t<decltype(std::declval<T>().has_value())>>
class criterion_traits
{
public:
    using value_type = T::value_type;

private:
    T instance_{};

public:
    constexpr bool has_value() noexcept
    {
        return instance_.has_value();
    }


};

}  // namespace ccc

#endif  // !CCC_EXPECTED_CRITERION_TRAITS_HH
