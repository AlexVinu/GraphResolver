#include <ranges>
#include <functional>
#include <queue>

#include "graph_resolver/headers/graph_resolver.h"

namespace graph_resolver
{
    using namespace graph_core;
    // ---------------------- Types for graph ----------------------------
    using ModuleMapType = std::unordered_map<modid_t, ModuleSlotBase*>;
    using OrderType = std::vector<modid_t>;
    // --------------------------------------------------------------------

    static GraphResult MakeGraph(const ModuleMapType& module_slots,
            OrderType& exec_order,
            std::function<const ModIdView(const ModuleSlotBase*)> get_func);

    struct GraphResolver::Impl
    {
        ModuleMapType moduleSlots;
        OrderType initOrder;
        OrderType postInitOrder;
        void Clear()
        {
            moduleSlots.clear();
            initOrder.clear();
            postInitOrder.clear();
        }
    };

    GraphResolver::GraphResolver()
    {
        m_Impl = new Impl;
    }
    GraphResolver::~GraphResolver()
    {
        DestroyGraph();
        delete m_Impl;
    }

    void GraphResolver::DestructInOrder()
    {
        for (const auto idx : std::views::reverse(m_Impl->initOrder))
        {
            m_Impl->moduleSlots[idx]->Destruct();
        }
    }

    GraphResult GraphResolver::CompileGraph()
    {
        const auto init_success = MakeGraph(m_Impl->moduleSlots, m_Impl->initOrder, 
            [](const ModuleSlotBase* mod_slot){return mod_slot->GetDeps();});
        if(!init_success) return init_success;
        
        const auto post_init_success = MakeGraph(m_Impl->moduleSlots, m_Impl->postInitOrder, 
            [](const ModuleSlotBase* mod_slot){return mod_slot->GetPostDeps();});
        if(!post_init_success) return post_init_success;
        
        return {GraphErrorCode::Success};
    }

    void GraphResolver::ExecuteGraph()
    {
        for(const auto idx: m_Impl->initOrder)
        {
            const auto& mod = m_Impl->moduleSlots[idx];
            mod->Construct(this);
        }
        for(const auto idx: m_Impl->postInitOrder)
        {
            const auto& mod = m_Impl->moduleSlots[idx];
            mod->PostConstruct(this);
        }
    }

    void GraphResolver::DestroyGraph()
    {
        for (const auto idx : std::views::reverse(m_Impl->initOrder))
        {
            m_Impl->moduleSlots[idx]->DestroySelf();
        }
        m_Impl->Clear();
    }

    GraphResult MakeGraph(const ModuleMapType& module_slots,
            OrderType& exec_order,
            std::function<const ModIdView(const ModuleSlotBase*)> get_func)
    {
        exec_order.clear();
        
        // 1. Count degrees + fill dependencies from node
        std::unordered_map<modid_t, int> in_degree;
        std::unordered_map<modid_t, OrderType> dependents;

        for(const auto&[type, module]: module_slots)
        {
            const auto deps = get_func(module);
            in_degree[type] = 0;
            for(const auto dep: deps)
            {
                in_degree[type]+=1;
                dependents[dep].push_back(type);
            }
        }
        // 2. Push nodes without degrees
        std::queue<modid_t> ready;
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
            StringView trouble_mod_name;
            for(const auto&[id, mod]: module_slots)
            {
                auto it = std::ranges::find(exec_order, id);
                if(it == exec_order.end())
                {
                    trouble_mod_name = mod->GetInterfaceName();
                    break;
                }
            }
            return {GraphErrorCode::CycleDetected, trouble_mod_name};
        }
        return {GraphErrorCode::Success};
    }

    // -------------- Private methods -----------------
    void* GraphResolver::GetModuleInt(modid_t indx)
    {
        auto it = m_Impl->moduleSlots.find(indx);
        if (it != m_Impl->moduleSlots.end())
        {
            return it->second->GetInterfacePtr();
        }
        return nullptr;
    }

    void GraphResolver::AddModule(modid_t indx, ModuleSlotBase *module_base)
    {
        m_Impl->moduleSlots[indx] = module_base;
    }

    bool GraphResolver::HasModule(modid_t indx)
    {
        return m_Impl->moduleSlots.contains(indx);
    }
}
