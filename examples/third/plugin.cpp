#include "graph_resolver/core.h"
#include "graph_resolver/i_graph_resolver.h"
#include "header.h"

#include <iostream>

using namespace std;

class Window : public IWindow
{
public:
    Window(){std::cout << "Window constructor!\n";}
    ~Window(){std::cout << "Window destructor!\n";}
    void Run() override {cout << "Window run!\n";}
    void PostConstruct(IRender* ptr) override 
    {
        cout << "Window post construct! Call IRender function\n";
        ptr->FuncForWindow();
    }
};

class Render : public IRender
{
public:
    Render(IWindow* w_ptr){std::cout << "Render constructor! Window module ptr: " <<w_ptr << endl;}
    ~Render(){std::cout << "Render destructor!\n";}
    void Run() override {cout << "Render run!\n";}
    void FuncForWindow() override {cout << "Render Func for window!\n";}
};

class Audio : public IAudio
{
public:
    Audio(){std::cout << "Audio constructor!\n";}
    ~Audio(){std::cout << "Audio destructor!\n";}
    void Run() override {cout << "Audio run!\n";}
};

PLUGIN_API void RegisterPlugins(GetIGraphResolverFn fn)
{
    auto* loader = fn();
    graph_resolver::AddModule<IAudio, Audio>(
        loader, 
    [](graph_resolver::IGraphResolver* ptr){return new Audio;});

    graph_resolver::AddModule<IWindow, Window, graph_core::TypeList<>, graph_core::TypeList<IRender>>(
        loader, 
    [](graph_resolver::IGraphResolver* ptr){return new Window;},
    [](graph_resolver::IGraphResolver* ptr, Window* window_ptr){
        IRender* render_ptr = graph_resolver::GetModule<IRender>(ptr);
        window_ptr->PostConstruct(render_ptr);
    });

    graph_resolver::AddModule<IRender, Render, graph_core::TypeList<IWindow>>(
        loader,
    [](graph_resolver::IGraphResolver* ptr){
        IWindow* window_ptr = graph_resolver::GetModule<IWindow>(ptr);
        return new Render(window_ptr);
    });

}