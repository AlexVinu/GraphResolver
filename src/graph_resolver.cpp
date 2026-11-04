#include <cstddef>
#include <ranges>
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>

#include "graph_resolver/internal/graph_resolver.h"


// ---------------------- Types for graph ----------------------------
using ModuleMapType = std::unordered_map<gr_modid_t, gr_ModuleSlot*>;
using OrderType = std::vector<gr_modid_t>;
// --------------------------------------------------------------------

static gr_GraphResult MakeGraph(const ModuleMapType& module_slots,
        OrderType& exec_order,
        std::function<const gr_ModIdView(const gr_ModuleSlot*)> get_func);

static gr_ModuleSlot* CreateModuleSlotFromDescriptor(gr_ModuleSlotDescriptor* desc)
{
    return new gr_ModuleSlot(
        std::string_view(desc->type_name.data, desc->type_name.size),
        desc->construct,
        desc->post_construct,
        desc->destruct,
        desc->deps,
        desc->post_deps
    );
}

struct gr_GraphResolver::Impl
{
    ModuleMapType moduleSlots;
    OrderType initOrder;
    OrderType postInitOrder;
    bool isCompiled = false;
    bool isExecuted = false;
    void Clear()
    {
        moduleSlots.clear();
        initOrder.clear();
        postInitOrder.clear();
        isCompiled = false;
        isExecuted = false;
    }
};

gr_GraphResolver::gr_GraphResolver()
{
    p_Impl = new Impl;
}
gr_GraphResolver::~gr_GraphResolver()
{
    DestroyGraph();
    delete p_Impl;
}

void gr_GraphResolver::DestructInOrder()
{
    for (const auto idx : std::views::reverse(p_Impl->initOrder))
    {
        p_Impl->moduleSlots[idx]->Destruct();
    }
}

gr_GraphResult gr_GraphResolver::CompileGraph()
{
    p_Impl->isCompiled = false;

    const auto init_success = MakeGraph(p_Impl->moduleSlots, p_Impl->initOrder, 
        [](const gr_ModuleSlot* mod_slot){return mod_slot->GetDeps();});
    if(!init_success) return init_success;
    
    const auto post_init_success = MakeGraph(p_Impl->moduleSlots, p_Impl->postInitOrder, 
        [](const gr_ModuleSlot* mod_slot){return mod_slot->GetPostDeps();});
    if(!post_init_success) return post_init_success;
    
    p_Impl->isCompiled = true;
    p_Impl->isExecuted = false;

    return {gr_GraphErrorCode::Success};
}

gr_GraphResult gr_GraphResolver::ExecuteGraph()
{
    if(p_Impl->isExecuted) 
    {
        return {gr_GraphErrorCode::GraphAlreadyExecuted};
    }
    
    if(!p_Impl->isCompiled) 
    {
        return {gr_GraphErrorCode::GraphNotCompiled};
    }

    auto api = gr_GetGraphResolverApi();

    for(const auto idx: p_Impl->initOrder)
    {
        const auto& mod = p_Impl->moduleSlots[idx];
        mod->Construct(api, this);
    }
    for(const auto idx: p_Impl->postInitOrder)
    {
        const auto& mod = p_Impl->moduleSlots[idx];
        mod->PostConstruct(api, this);
    }

    p_Impl->isExecuted = true;
    return {gr_GraphErrorCode::Success};
}

void gr_GraphResolver::DestroyGraph()
{
    // First - destroy compiled graph
    for (const auto idx : std::views::reverse(p_Impl->initOrder))
    {
        p_Impl->moduleSlots[idx]->DestroySelf();
        p_Impl->moduleSlots.erase(idx);
    }
    // Second - if graph wasnt fully compiled
    for(const auto&[idx, mod]: p_Impl->moduleSlots)
    {
        mod->DestroySelf();
    }

    p_Impl->Clear();
}

gr_GraphResult MakeGraph(const ModuleMapType& module_slots,
        OrderType& exec_order,
        std::function<const gr_ModIdView(const gr_ModuleSlot*)> get_func)
{
    exec_order.clear();
    
    // 1. Count degrees + fill dependencies from node
    std::unordered_map<gr_modid_t, int> in_degree;
    std::unordered_map<gr_modid_t, OrderType> dependents;

    for(const auto&[type, module]: module_slots)
    {
        const auto deps = get_func(module);
        if (!deps.IsValid())
        {
            gr_GraphResult result{};
            result.code = gr_GraphErrorCode::InvalidDependencyList;
            const auto name = module->GetTypeName();
            result.SetModName(std::string_view(name.data, name.size));
            return result;
        }
        in_degree[type] = 0;
        for(const auto dep: deps)
        {
            if(!module_slots.contains(dep)) 
            {
                gr_GraphResult result{};
                result.code = gr_GraphErrorCode::MissingDependancy;
                auto name = module->GetTypeName();
                result.SetModName(std::string_view(name.data, name.size));
                return result;
            }

            in_degree[type]+=1;
            dependents[dep].push_back(type);
        }
    }
    // 2. Push nodes without degrees
    std::queue<gr_modid_t> ready;
    for(const auto&[mod, deg]: in_degree)
        if(deg == 0) 
            ready.push(mod);
    
    // 3. Make initialization order
    exec_order.clear();
    while (!ready.empty()) 
    {
        const auto current = ready.front();
        ready.pop();
        exec_order.push_back(current);
        for(const auto next: dependents[current])
            if(--in_degree[next] == 0)
                ready.push(next);
    }
    // 4. Final check
    if (exec_order.size() != module_slots.size())
    {
        gr_StringView trouble_mod_name{};
        for(const auto&[id, mod]: module_slots)
        {
            auto it = std::ranges::find(exec_order, id);
            if(it == exec_order.end())
            {
                trouble_mod_name = mod->GetTypeName();
                break;
            }
        }
        gr_GraphResult result{};
        result.code = gr_GraphErrorCode::CycleDetected;
        result.SetModName(std::string_view(trouble_mod_name.data, trouble_mod_name.size));
        return result;
    }
    return {gr_GraphErrorCode::Success};
}

gr_GraphResult gr_GraphResolver::AddModuleFromDescriptor(gr_ModuleSlotDescriptor* desc)
{
    if (!desc
        || desc->struct_size < sizeof(gr_ModuleSlotDescriptor)
        || (desc->type_name.size != 0 && desc->type_name.data == nullptr))
    {
        return {gr_GraphErrorCode::UNKNOWN};
    }

    gr_modid_t indx = desc->module_id;
    if (!desc->deps.IsValid() || !desc->post_deps.IsValid())
    {
        gr_GraphResult result{};
        result.code = gr_GraphErrorCode::InvalidDependencyList;
        result.SetModName(std::string_view(desc->type_name.data, desc->type_name.size));
        return result;
    }

    if(HasModule(indx)){
        gr_GraphResult result{};
        result.code = gr_GraphErrorCode::DuplicateModuleID;
        result.SetModName(std::string_view(desc->type_name.data, desc->type_name.size));
        return result;
    }

    gr_ModuleSlot* module_slot = CreateModuleSlotFromDescriptor(desc);
    AddModule(indx, module_slot);
    return {gr_GraphErrorCode::Success};
}

void* gr_GraphResolver::GetModuleInt(gr_modid_t indx)
{
    auto it = p_Impl->moduleSlots.find(indx);
    if (it != p_Impl->moduleSlots.end())
    {
        return it->second->GetModulePtr();
    }
    return nullptr;
}

void gr_GraphResolver::AddModule(gr_modid_t indx, gr_ModuleSlot* module_slot)
{
    p_Impl->moduleSlots[indx] = module_slot;
}

bool gr_GraphResolver::HasModule(gr_modid_t indx)
{
    return p_Impl->moduleSlots.contains(indx);
}

// -------------- c-style API -----------------
void CALL ExecuteGraph(gr_GraphResolver* self, gr_GraphResult* result)
{
    *result = self->ExecuteGraph();
}

void CALL CompileGraph(gr_GraphResolver* self, gr_GraphResult* result)
{
    *result = self->CompileGraph();
}

void CALL DestructInOrder(gr_GraphResolver* self)
{
    self->DestructInOrder();
}

void CALL DestroyGraph(gr_GraphResolver* self)
{
    self->DestroyGraph();
}

bool CALL HasModule(gr_GraphResolver* self, gr_modid_t indx)
{
    return self->HasModule(indx);
}

void* CALL GetModuleInt(gr_GraphResolver* self, gr_modid_t indx)
{
    return self->GetModuleInt(indx);
}

void CALL AddModuleFromDescriptor(gr_GraphResolver* self, gr_ModuleSlotDescriptor* desc, gr_GraphResult* result)
{
    *result = self->AddModuleFromDescriptor(desc);
}

static const gr_GraphResolverApi g_graph_resolver_api = {
    sizeof(gr_GraphResolverApi),
    &ExecuteGraph,
    &CompileGraph,
    &DestructInOrder,
    &DestroyGraph,
    &HasModule,
    &GetModuleInt,
    &AddModuleFromDescriptor
};

const gr_GraphResolverApi* CALL gr_GetGraphResolverApi()
{
    return &g_graph_resolver_api;
}
