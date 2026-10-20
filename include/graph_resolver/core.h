#pragma once

#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <cassert>

#include "utils/macro.h"
#include "utils/hash_string.h"

namespace graph_core{

    using modid_t = uint64_t;
    using modcount_t = uint16_t;

    enum class GraphErrorCode : uint8_t
    {
        Success             = 0,
        CycleDetected       = 1,
        DuplicateModuleID   = 2,
        MissingDependancy   = 3,
        GraphNotCompiled    = 4
    };

    struct ModIdView
    {
        modid_t data[1024];
        modcount_t count;

        static constexpr ModIdView From(const modid_t* src, modcount_t n) noexcept
        {
            assert(n <= 1024 && "ModIdView: Too many elements! Exceeds maximum capacity of 1024.");

            ModIdView view{};
            view.count = n > 1024 ? 1024 : n;
            
            std::copy(src, src + view.count, view.data);
            
            return view;
        }

        constexpr const modid_t* begin() const noexcept { return data; }
        constexpr const modid_t* end() const noexcept { return data + count; }
        constexpr size_t size() const noexcept { return count; }
        constexpr bool empty() const noexcept { return count == 0; }
    };

    static_assert(std::is_standard_layout_v<ModIdView>, "graph_resolver error: ModIdView is not standard_layout!");
    static_assert(std::is_trivial_v<ModIdView>, "graph_resolver error: ModIdView is not trivial!");
    struct GraphResult
    {
        GraphErrorCode code;
        char mod_name[256];

        void SetModName(std::string_view sv)
        {
            constexpr size_t max_len = sizeof(mod_name) - 1;
            size_t len = (sv.size() < max_len) ? sv.size() : max_len;

            std::memcpy(mod_name, sv.data(), len);
            mod_name[len] = '\0';
        }

        bool IsOk() const {return code == GraphErrorCode::Success;}
        operator bool() const {return IsOk();}
    };

    static_assert(std::is_standard_layout_v<GraphResult>, "graph_resolver error: GraphResult is not standard_layout!");
    static_assert(std::is_trivial_v<GraphResult>, "graph_resolver error: GraphResult is not trivial!");

    template<class... T>
    struct TypeList
    {
        static constexpr modcount_t size = sizeof...(T);
        static inline constexpr modid_t arr[size > 0 ? size : 1] = {hash_string(GET_TYPE_NAME(T))...};

        static constexpr ModIdView get() noexcept
        {
            return ModIdView::From(arr, size);
        }
    };

    template<class T>
    struct is_type_list : std::false_type {};

    template<class... Ts>
    struct is_type_list<TypeList<Ts...>> : std::true_type {}; 

    template<class T>
    concept IsTypeList = is_type_list<T>::value;
}