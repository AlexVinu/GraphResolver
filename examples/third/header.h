#pragma once

#include "core.h"

#include <graph_resolver/i_graph_resolver.h>

struct IRender;

struct IWindow
{
    virtual ~IWindow() = default;
    virtual void PostConstruct(IRender*) = 0;
    virtual void Run() = 0;
};

struct IRender
{
    virtual ~IRender() = default;
    virtual void FuncForWindow() = 0;
    virtual void Run() = 0;
};

struct IAudio
{
    virtual ~IAudio() = default;
    virtual void Run() = 0;
};

using GetIGraphResolverFn = graph_resolver::IGraphResolver* (*)();

extern "C"
{
    graph_resolver::IGraphResolver* GetIGraphResolver();
    PLUGIN_API void RegisterPlugins(GetIGraphResolverFn);
}