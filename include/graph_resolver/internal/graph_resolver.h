#pragma once

#include "graph_resolver/internal/module_slot.h"
#include "graph_resolver/api/graph_resolver_api.h"

extern "C" const gr_GraphResolverApi* CALL gr_GetGraphResolverApi();
class gr_GraphResolver
{
public:
    gr_GraphResolver();
    gr_GraphResolver(const gr_GraphResolver&) = delete;
    ~gr_GraphResolver();

    template<
        class T,
        typename DEPS = gr_TypeList<>,
        typename POSTDEPS = gr_TypeList<>
        >
    gr_GraphResult AddModule(
        gr_ConstructFn construct,
        gr_PostConstructFn post_construct = nullptr,
        gr_DestructFn destruct = gr_ModuleSlotDefaultDestruct<T>) 
    {
        gr_modid_t indx = hash_string(GET_TYPE_NAME(T));
        std::string_view view = GET_TYPE_NAME(T);
        if(HasModule(indx)){
            gr_GraphResult result{};
            result.code = gr_GraphErrorCode::DuplicateModuleID;
            result.SetModName(view); 
            return result;
        }

        gr_ModuleSlot* module_slot = CreateModuleSlot<T, DEPS, POSTDEPS>(construct, post_construct, destruct);
        AddModule(indx, module_slot);
        return {gr_GraphErrorCode::Success};
    }

    template<class T>
    T* GetModule()
    {
        gr_modid_t indx(hash_string(GET_TYPE_NAME(T)));
        return static_cast<T*>(GetModuleInt(indx));
    }

    template<class T>
    bool HasModule()
    {
        gr_modid_t indx = hash_string(GET_TYPE_NAME(T));
        return HasModule(indx);
    }

    gr_GraphResult AddModuleFromDescriptor(gr_ModuleSlotDescriptor* desc);

    gr_GraphResult CompileGraph();
    gr_GraphResult ExecuteGraph();
    void DestructInOrder();
    void DestroyGraph();

    void AddModule(gr_modid_t indx, gr_ModuleSlot* module_slot);
    bool HasModule(gr_modid_t indx);
    void* GetModuleInt(gr_modid_t indx);

private:
    struct Impl;
    Impl* p_Impl;
};