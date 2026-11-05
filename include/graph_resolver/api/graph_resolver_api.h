#pragma once

#include "module_slot_desc.h"

struct gr_GraphResolverApi
{
    uint32_t struct_size;
    void (CALL *execute_graph)(gr_GraphResolver* self, gr_GraphResult* result);
    void (CALL *compile_graph)(gr_GraphResolver* self, gr_GraphResult* result);
    void (CALL *destruct_in_order)(gr_GraphResolver* self);
    void (CALL *destroy_graph)(gr_GraphResolver* self);
    bool (CALL *has_module)(gr_GraphResolver* self, gr_modid_t indx);
    void* (CALL *get_module_int)(gr_GraphResolver* self, gr_modid_t indx);
    void (CALL *add_module_from_descriptor)(gr_GraphResolver* self, gr_ModuleSlotDescriptor* desc, gr_GraphResult* result);
};

template<
    class T,
    typename DEPS = gr_TypeList<>,
    typename POSTDEPS = gr_TypeList<>
    >
gr_ModuleSlotDescriptor CreateModuleSlotDescriptor(
    gr_ConstructFn construct,
    gr_PostConstructFn post_construct = nullptr,
    gr_DestructFn destruct = gr_ModuleSlotDefaultDestruct<T>)
{
    gr_ModuleSlotDescriptor desc{};
    desc.struct_size = sizeof(gr_ModuleSlotDescriptor);
    desc.construct = construct;
    desc.post_construct = post_construct;
    desc.destruct = destruct;
    desc.module_id = hash_string(GET_TYPE_NAME(T));
    const std::string_view type_name = GET_TYPE_NAME(T);
    desc.type_name = {type_name.data(), type_name.size()};
    desc.deps = DEPS::get();
    desc.post_deps = POSTDEPS::get();

    return desc;
}

template<
    class T,
    typename DEPS = gr_TypeList<>,
    typename POSTDEPS = gr_TypeList<>
    >
void AddModule(
    const gr_GraphResolverApi* graph_api,
    gr_GraphResolver* graph_resolver,
    gr_GraphResult* result,
    gr_ConstructFn construct,
    gr_PostConstructFn post_construct = nullptr,
    gr_DestructFn destruct = gr_ModuleSlotDefaultDestruct<T>)
{
    *result = {gr_GraphErrorCode::Success, ""};
    gr_modid_t indx = hash_string(GET_TYPE_NAME(T));
    std::string_view view = GET_TYPE_NAME(T);
    if(graph_api->has_module(graph_resolver, indx)){
        result->code = gr_GraphErrorCode::DuplicateModuleID;
        result->SetModName(view);
        return;
    }

    auto module_slot_desc = CreateModuleSlotDescriptor<
                                                    T, DEPS, POSTDEPS>
                                                    (construct, post_construct, destruct);
    
    graph_api->add_module_from_descriptor(graph_resolver, &module_slot_desc, result);
}

template<class T>
T* GetModule(const gr_GraphResolverApi* graph_api, gr_GraphResolver* graph_resolver)
{
    gr_modid_t indx(hash_string(GET_TYPE_NAME(T)));
    return static_cast<T*>(graph_api->get_module_int(graph_resolver, indx));
}

template<class T>
bool HasModule(const gr_GraphResolverApi* graph_api, gr_GraphResolver* graph_resolver)
{
    gr_modid_t indx(hash_string(GET_TYPE_NAME(T)));
    return graph_api->has_module(graph_resolver, indx);
}