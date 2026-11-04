#pragma once

#include "graph_resolver/api/core.h"
#include <string_view>

class gr_ModuleSlot
{
public:
    gr_ModuleSlot(
        std::string_view type_name,
        gr_ConstructFn fn1,
        gr_PostConstructFn fn2,
        gr_DestructFn fn_d,
        gr_ModIdView deps = gr_ModIdView::From(nullptr, 0),
        gr_ModIdView post_deps = gr_ModIdView::From(nullptr, 0));
    
    ~gr_ModuleSlot();

    void Construct(const gr_GraphResolverApi* api, gr_GraphResolver* loader);
    void PostConstruct(const gr_GraphResolverApi* api, gr_GraphResolver* loader);
    void Destruct();

    gr_ModIdView GetDeps() const;
    gr_ModIdView GetPostDeps() const;
    void* GetModulePtr() const;
    void DestroySelf();
    gr_StringView GetTypeName() const;
private:
    struct Impl;
    Impl* p_Impl;
};

template<
    class T,
    typename DEPS = gr_TypeList<>,
    typename POSTDEPS = gr_TypeList<>
    >
gr_ModuleSlot* CreateModuleSlot(gr_ConstructFn construct,
    gr_PostConstructFn post_construct,
    gr_DestructFn destruct)
{
    return new gr_ModuleSlot(
        GET_TYPE_NAME(T),
        construct,
        post_construct,
        destruct,
        DEPS::get(),
        POSTDEPS::get()
    );
}