module;

#include <ranges>
#include <functional>
#include <queue>

module graph_resolver;

using namespace graph_core;

namespace graph_resolver
{
    // ---------------------- Types for graph ----------------------------
    using ModuleMapType = std::unordered_map<modid_t, ModuleSlotBase*>;
    using OrderType = std::vector<modid_t>;
    // --------------------------------------------------------------------

    static void MakeGraph(const ModuleMapType& module_slots,
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

    void GraphResolver::CompileGraph()
    {
        MakeGraph(m_Impl->moduleSlots, m_Impl->initOrder, 
            [](const ModuleSlotBase* mod_slot){return mod_slot->GetDeps();});
        MakeGraph(m_Impl->moduleSlots, m_Impl->postInitOrder, 
            [](const ModuleSlotBase* mod_slot){return mod_slot->GetPostDeps();});   
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

    void MakeGraph(const ModuleMapType& module_slots,
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
            OrderType not_ordered;
            for(const auto&[id, mod]: module_slots)
            {
                auto it = std::ranges::find(exec_order, id);
                if(it == exec_order.end())
                {
                    not_ordered.push_back(id);
                }
            }
        }
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
}
