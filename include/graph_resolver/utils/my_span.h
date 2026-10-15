#pragma once

#include <cstddef>

template<typename T>
struct MySpan
{
    const T* data;
    size_t count;

    constexpr const T* begin() const noexcept { return data; }
    constexpr const T* end() const noexcept { return data + count; }
    constexpr size_t size() const noexcept { return count; }
    constexpr bool empty() const noexcept { return count == 0; }
};