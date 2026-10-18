#pragma once

#include "core.h"
#include "module_slot.h"

namespace graph_resolver{
    using namespace graph_core;

    struct IGraphResolver
    {
        virtual ~IGraphResolver()                   = default;
        virtual GraphResult CompileGraph()          = 0;
        virtual void ExecuteGraph()                 = 0;
        virtual void DestructInOrder()              = 0;
        virtual void DestroyGraph()                 = 0;
        virtual void AddModule(modid_t indx, ModuleSlotBase* module_base) = 0;
        virtual bool HasModule(modid_t indx)        = 0;
        virtual void* GetModuleInt(modid_t indx)    = 0;
    };

    template<
        class I, class T,
        typename DEPS = TypeList<>,
        typename POSTDEPS = TypeList<>
        >
    GraphResult AddModule(
        IGraphResolver* graph_resolver,
        ConstructFn<T> construct,
        PostConstructFn<T> post_construct = nullptr,
        DestructFn<T> destruct = DefaultDestruct<T>)
    {
        modid_t indx = hash_string(GET_TYPE_NAME(I));
        std::string_view view = GET_TYPE_NAME(T);
        if(graph_resolver->HasModule(indx)){
            GraphResult result;
            result.code = GraphErrorCode::DuplicateModuleID;
            result.SetModName(view); 
            return result;
        }

        ModuleSlotBase* module_base = new ModuleSlot<I, T, DEPS, POSTDEPS>(
            std::move(construct), std::move(post_construct), std::move(destruct));
        graph_resolver->AddModule(indx, module_base);
        return {GraphErrorCode::Success};
    }

    template<class I>
    I* GetModule(IGraphResolver* graph_resolver)
    {
        modid_t indx(hash_string(GET_TYPE_NAME(I)));
        return static_cast<I*>(graph_resolver->GetModuleInt(indx));
    }
}