#pragma once

#include <type_traits>
#include <memory>

namespace Netcode {

    /*
    dangerous helper class, the copy constructor moves, useful for std::function and std::bind when 
    certain parameters are not move constructible
    */
    template <typename T>
    struct destructive_copy_constructible
    {
        mutable T value;

        destructive_copy_constructible() {}

        destructive_copy_constructible(T && v) : value(std::move(v)) {}

        destructive_copy_constructible(const destructive_copy_constructible<T> & rhs)
            : value(std::move(rhs.value))
        {}

        destructive_copy_constructible(destructive_copy_constructible<T> && rhs) = default;

        destructive_copy_constructible &
            operator=(const destructive_copy_constructible<T> & rhs) = delete;

        destructive_copy_constructible &
            operator=(destructive_copy_constructible<T> && rhs) = delete;
    };

    template <typename T>
    using dcc_t =
        destructive_copy_constructible<typename std::remove_reference<T>::type>;

    template <typename T>
    inline dcc_t<T> move_to_dcc(T && r)
    {
        return dcc_t<T>(std::move(r));
    }
}
