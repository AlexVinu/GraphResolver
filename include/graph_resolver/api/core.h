#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <cassert>
#include <string_view>
#include <string>

#include "graph_resolver/api/mod_id.h"

#if defined(_WIN32)
    #define CALL __cdecl
#else
    #define CALL
#endif

using gr_modname_t = std::string;

constexpr gr_modid_t hash_string(std::string_view str)
{
    // FNV-1a 128-bit offset basis
    uint64_t high = 0x6C62272E07BB0142ULL;
    uint64_t low  = 0x62B821756295C58DULL;

    // FNV-1a 128-bit prime
    constexpr uint64_t prime_high = 0x0000000001000000ULL;
    constexpr uint64_t prime_low  = 0x13BULL;

    for (unsigned char c : str)
    {
        low ^= c;

        const uint64_t a = low;
        const uint64_t b = prime_low;

        const uint64_t a0 = static_cast<uint32_t>(a);
        const uint64_t a1 = a >> 32;
        const uint64_t b0 = static_cast<uint32_t>(b);
        const uint64_t b1 = b >> 32;

        const uint64_t w0 = a0 * b0;
        const uint64_t t  = a1 * b0 + (w0 >> 32);
        const uint64_t w1 = (t & 0xFFFFFFFFULL) + a0 * b1;
        const uint64_t carry =
            a1 * b1 + (t >> 32) + (w1 >> 32);

        const uint64_t new_low =
            (w1 << 32) | (w0 & 0xFFFFFFFFULL);

        high = high * prime_low + low * prime_high + carry;
        low = new_low;
    }

    return { high, low };
}

#define CREATE_MODULE(X)\
    inline static constexpr gr_modid_t ID = hash_string(#X);\
    inline static constexpr gr_modname_t NAME = #X; 

template<typename T>
concept gr_HasID = requires
{
    requires std::same_as<
        std::remove_cvref_t<decltype(T::ID)>,
        gr_modid_t
    >;
};

template<typename T>
concept gr_HasName = requires 
{
    requires std::same_as<
        std::remove_cvref_t<decltype(T::NAME)>,
        gr_modname_t
    >;
};

template<typename T>
concept gr_IsModule = gr_HasID<T> && gr_HasName<T>;

template<typename T> requires gr_IsModule<T>
constexpr gr_modid_t gr_GetModuleId(){return T::ID;}

template<typename T> requires gr_IsModule<T>
constexpr auto gr_GetModuleName(){return T::NAME;}

#define GET_MODULE_NAME(X) gr_GetModuleName<X>()
#define GET_MODULE_ID(X) gr_GetModuleId<X>()

using gr_modcount_t = uint64_t;

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

inline gr_GraphResult MakeGraphResult(gr_GraphErrorCode code = gr_GraphErrorCode::Success,
    std::string_view name = {}) noexcept
{
    gr_GraphResult result{code, {}, 0, 0};
    if (!name.empty())
    {
        result.SetModName(name);
    }
    return result;
}

template<class... T>
    requires (gr_HasID<T> && ...)
class gr_TypeList
{
public:
    static constexpr gr_modcount_t size = sizeof...(T);
    static inline constexpr gr_modid_t arr[size > 0 ? size : 1] = {T::ID...};

    static consteval gr_ModIdView get() noexcept
    {
        static_assert(sizeof...(T) <= gr_ModIdView::capacity,
            "gr_TypeList exceeds gr_ModIdView capacity");
        return gr_ModIdView::From(arr, size);
    }
private:
    static consteval bool check_collisions() noexcept
    {
        for (std::size_t i = 0; i < size; ++i)
        {
            for (std::size_t j = i + 1; j < size; ++j)
            {
                if (arr[i] == arr[j])
                    return true;
            }
        }
        return false;
    };
    static_assert(!check_collisions(), 
        "gr_TypeList collisions found! Same type added or hasher given a collision");
};

template<class T>
struct gr_is_type_list : std::false_type {};

template<class... Ts>
struct gr_is_type_list<gr_TypeList<Ts...>> : std::true_type {}; 

template<class T>
concept gr_IsTypeList = gr_is_type_list<T>::value;

class gr_GraphResolver;
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