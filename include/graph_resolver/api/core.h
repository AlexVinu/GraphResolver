#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <string_view>

#include "graph_resolver/utils/macro.h"
#include "graph_resolver/utils/hash_string.h"

#if defined(_WIN32)
    #define CALL __cdecl
#else
    #define CALL
#endif


typedef uint64_t gr_modid_t;
typedef uint16_t gr_modcount_t;

enum class gr_GraphErrorCode : uint8_t
{
    Success             = 0,
    CycleDetected       = 1,
    DuplicateModuleID   = 2,
    MissingDependancy   = 3,
    GraphNotCompiled    = 4,
    GraphAlreadyExecuted= 5,
    InvalidDependencyList = 6,
    GraphNotExecuted    = 7,
    DifferentAPIVersion = 8,

    UNKNOWN             = 255
};

struct gr_StringView
{
    const char* data;
    uint64_t size;

    constexpr const char* begin() const noexcept { return data; }
    constexpr const char* end() const noexcept { return size == 0 ? data : data + size; }
};

struct gr_ModIdView
{
    static constexpr std::size_t capacity = 1024;

    gr_modid_t data[capacity];
    gr_modcount_t count;
    uint8_t overflow;

    static constexpr gr_ModIdView From(const gr_modid_t* src, gr_modcount_t n) noexcept
    {
        gr_ModIdView view{};
        const bool valid = n <= capacity && (n == 0 || src != nullptr);
        view.count = valid ? n : 0;
        view.overflow = !valid;
        if (src != nullptr)
        {
            for (std::size_t i = 0; i < view.count; ++i)
            {
                view.data[i] = src[i];
            }
        }
        return view;
    }

    constexpr const gr_modid_t* begin() const noexcept { return data; }
    constexpr const gr_modid_t* end() const noexcept { return data + count; }
    constexpr size_t size() const noexcept { return count; }
    constexpr bool empty() const noexcept { return count == 0; }
    constexpr bool IsValid() const noexcept { return overflow == 0; }
};
struct gr_GraphResult
{
    static constexpr std::size_t module_name_capacity = 256;

    gr_GraphErrorCode code;

    char mod_name[module_name_capacity];
    uint16_t mod_name_size;
    uint8_t mod_name_truncated;

    void SetModName(std::string_view sv)
    {
        const auto copied_size = std::min(sv.size(), module_name_capacity - 1);
        if (copied_size != 0)
        {
            std::memcpy(mod_name, sv.data(), copied_size);
        }
        mod_name[copied_size] = '\0';
        mod_name_size = static_cast<uint16_t>(copied_size);
        mod_name_truncated = copied_size != sv.size();
    }

    bool IsOk() const noexcept {return code == gr_GraphErrorCode::Success;}
    operator bool() const noexcept {return IsOk();}
};

template<class... T>
struct gr_TypeList
{
    static constexpr gr_modcount_t size = sizeof...(T);
    static inline constexpr gr_modid_t arr[size > 0 ? size : 1] = {hash_string(GET_TYPE_NAME(T))...};

    static constexpr gr_ModIdView get() noexcept
    {
        static_assert(sizeof...(T) <= gr_ModIdView::capacity,
            "gr_TypeList exceeds gr_ModIdView capacity");
        return gr_ModIdView::From(arr, size);
    }
};

template<class T>
struct gr_is_type_list : std::false_type {};

template<class... Ts>
struct gr_is_type_list<gr_TypeList<Ts...>> : std::true_type {}; 

template<class T>
concept gr_IsTypeList = gr_is_type_list<T>::value;

struct gr_GraphResolver;
struct gr_GraphResolverApi;

// ----------------- Functions signatures --------------------
typedef void*(CALL *gr_ConstructFn)(const gr_GraphResolverApi*, gr_GraphResolver*);

typedef void(CALL *gr_PostConstructFn)(const gr_GraphResolverApi*, gr_GraphResolver*, void*);

typedef void(CALL *gr_DestructFn)(void*);

template<class T>
void gr_ModuleSlotDefaultDestruct(void* ptr)
{
    delete static_cast<T*>(ptr);
}

typedef void*(CALL *gr_ConstructFnApi)(const gr_GraphResolverApi*, gr_GraphResolver*) noexcept;

typedef void(CALL *gr_PostConstructFnApi)(const gr_GraphResolverApi*, gr_GraphResolver*, void*) noexcept;

typedef void(CALL *gr_DestructFnApi)(void*) noexcept;
// -----------------------------------------------------------