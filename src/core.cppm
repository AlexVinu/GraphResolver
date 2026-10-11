module;

#include <string_view>

#include "macro.h"

export module graph_core;

export namespace graph_core {
    constexpr uint64_t hash_string(std::string_view str) {
        uint64_t hash = 14695981039346656037ULL; // FNV_offset_basis (64-bit)
        for (char c : str) {
            hash ^= static_cast<uint64_t>(c);
            hash *= 1099511628211ULL;            // FNV_prime (64-bit)
        }
        return hash;
    }

    using modid_t = uint64_t;

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

    using ModIdView = MySpan<modid_t>;

    template<class... T>
    struct TypeList
    {
        static constexpr std::size_t size = sizeof...(T);
        static inline constexpr modid_t arr[size > 0 ? size : 1] = {hash_string(GET_TYPE_NAME(T))...};

        static constexpr ModIdView get() noexcept
        {
            return ModIdView{size > 0 ? arr : nullptr, size};
        }
    };

    template<class T>
    struct is_type_list : std::false_type {};

    template<class... Ts>
    struct is_type_list<TypeList<Ts...>> : std::true_type {}; 

    template<class T>
    concept IsTypeList = is_type_list<T>::value;
}