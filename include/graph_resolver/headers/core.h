#pragma once

#include <cstdint>
#include "graph_resolver/utils/my_span.h"
#include "graph_resolver/utils/macro.h"
#include "graph_resolver/utils/hash_string.h"

namespace graph_core{

    using modid_t = uint64_t;

    using ModIdView = MySpan<modid_t>;
    using StringView = MySpan<const char>;

    enum class GraphErrorCode : uint8_t
    {
        Success = 0,
        CycleDetected,
        DuplicateModuleID
    };

    struct GraphResult
    {
        GraphErrorCode code;
        StringView mod_name;

        bool IsOk() const {return code == GraphErrorCode::Success;}
        operator bool() const {return IsOk();}
    };

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