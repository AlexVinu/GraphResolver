#include "graph_resolver/internal/module_slot.h"

#include <cstddef>
#include <string>
#include <memory>
#include <string_view>

using module_ptr_t = std::unique_ptr<void, gr_DestructFn>;

struct gr_ModuleSlot::Impl
{
    module_ptr_t m_ModulePtr;
    gr_ConstructFn m_Construct;
    gr_PostConstructFn m_PostConstruct;
    gr_DestructFn m_Destruct;

    gr_ModIdView m_Deps;
    gr_ModIdView m_PostDeps;
    std::string m_TypeName;
};

gr_ModuleSlot::gr_ModuleSlot(
        std::string_view type_name,
        gr_ConstructFn fn1,
        gr_PostConstructFn fn2,
        gr_DestructFn fn_d,
        gr_ModIdView deps,
        gr_ModIdView post_deps)
{
    p_Impl = new Impl
    {
        module_ptr_t(nullptr, nullptr),
        fn1,
        fn2,
        fn_d,
        deps,
        post_deps,
        std::string(type_name)
    };
}

gr_ModuleSlot::~gr_ModuleSlot() 
{
    Destruct();
    delete p_Impl;
}

void gr_ModuleSlot::Construct(const gr_GraphResolverApi* api, gr_GraphResolver* loader)  
{
    if(p_Impl->m_Construct) 
        p_Impl->m_ModulePtr = module_ptr_t(p_Impl->m_Construct(api, loader), p_Impl->m_Destruct);
}
void gr_ModuleSlot::PostConstruct(const gr_GraphResolverApi* api, gr_GraphResolver* loader)  
{
    if(p_Impl->m_PostConstruct && p_Impl->m_ModulePtr) p_Impl->m_PostConstruct(api, loader, p_Impl->m_ModulePtr.get());
}
void gr_ModuleSlot::Destruct()  
{
    p_Impl->m_ModulePtr = nullptr;
}

gr_ModIdView gr_ModuleSlot::GetDeps() const  {return p_Impl->m_Deps;}

gr_ModIdView gr_ModuleSlot::GetPostDeps() const  {return p_Impl->m_PostDeps;}

void* gr_ModuleSlot::GetModulePtr() const  {return p_Impl->m_ModulePtr.get();}

void gr_ModuleSlot::DestroySelf()  {delete this;}

gr_StringView gr_ModuleSlot::GetTypeName() const  
{
    return {p_Impl->m_TypeName.data(), p_Impl->m_TypeName.size()};
}
