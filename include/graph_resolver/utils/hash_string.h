#pragma once

#include <cstdint>
#include <string_view>

constexpr uint64_t hash_string(std::string_view str) {
    uint64_t hash = 14695981039346656037ULL; // FNV_offset_basis (64-bit)
    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;            // FNV_prime (64-bit)
    }
    return hash;
}