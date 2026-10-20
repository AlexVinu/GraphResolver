#pragma once

#include "graph_resolver/core.h"
#include "module_slot.h"
#include "i_graph_resolver.h"

namespace graph_resolver{
    using namespace graph_core;
    
    class GraphResolver : public IGraphResolver
    {
    public:
        GraphResolver();
        GraphResolver(const GraphResolver&) = delete;
        ~GraphResolver() override;

        template<
            class I, class T,
            typename DEPS = TypeList<>,
            typename POSTDEPS = TypeList<>
            >
        GraphResult AddModule(
            ConstructFn<T> construct,
            PostConstructFn<T> post_construct = nullptr,
            DestructFn<T> destruct = DefaultDestruct<T>) 
        {
            modid_t indx = hash_string(GET_TYPE_NAME(I));
            std::string_view view = GET_TYPE_NAME(T);
            if(HasModule(indx)){
                GraphResult result;
                result.code = GraphErrorCode::DuplicateModuleID;
                result.SetModName(view); 
                return result;
            }

            ModuleSlotBase* module_base = new ModuleSlot<I, T, DEPS, POSTDEPS>(
                std::move(construct), std::move(post_construct), std::move(destruct));
            AddModule(indx, module_base);
            return {GraphErrorCode::Success};
        }

        GraphResult CompileGraph() override;
        GraphResult ExecuteGraph() override;
        void DestructInOrder() override;
        void DestroyGraph() override;

        template<class I>
        I* GetModule()
        {
            modid_t indx(hash_string(GET_TYPE_NAME(I)));
            return static_cast<I*>(GetModuleInt(indx));
        }

        void AddModule(modid_t indx, ModuleSlotBase* module_base) override;
        bool HasModule(modid_t indx) override;
        void* GetModuleInt(modid_t indx) override;

    private:
        struct Impl;
        Impl* m_Impl;
    };
}