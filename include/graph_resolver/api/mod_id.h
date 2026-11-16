#pragma once

#include <cstdint>
#include <functional>

struct Id128
{
    uint64_t high;
    uint64_t low;

    constexpr bool operator==(const Id128&) const = default;
};

using gr_modid_t = Id128;

template<>
struct std::hash<Id128>
{
    std::size_t operator()(const Id128& id) const noexcept
    {
        const auto high_hash = std::hash<uint64_t>{}(id.high);
        const auto low_hash = std::hash<uint64_t>{}(id.low);
        return high_hash ^ (low_hash + 0x9e3779b9 + (high_hash << 6) + (high_hash >> 2));
    }
};
