#pragma once

#include "core.h"

namespace graph_resolver{
    using namespace graph_core;

    class GraphResolver;

    struct ModuleSlotBase
    {
        virtual void Construct(GraphResolver*) = 0;
        virtual void PostConstruct(GraphResolver*) = 0;
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