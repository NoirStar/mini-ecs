#pragma once

#include <concepts>
#include <type_traits>

template<typename T>
concept Component = 
    std::is_copy_constructible_v<T> &&
    std::is_default_constructible_v<T>;