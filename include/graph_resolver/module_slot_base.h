#pragma once

#include "core.h"

namespace graph_resolver{
    using namespace graph_core;

    class IGraphResolver;
    struct ModuleSlotBase
    {
        virtual void Construct(IGraphResolver*) = 0;
        virtual void PostConstruct(IGraphResolver*) = 0;
        virtual void Destruct() = 0;
        virtual ModIdView GetDeps() const = 0;
        virtual ModIdView GetPostDeps() const = 0;
        virtual void* GetInterfacePtr() const = 0;
        virtual void DestroySelf() = 0;
        virtual std::string_view GetInterfaceName() const = 0;
        virtual std::string_view GetTypeName() const = 0;    
    protected:
        virtual ~ModuleSlotBase() = default;
    };
}