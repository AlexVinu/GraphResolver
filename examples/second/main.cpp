// examples/second/main.cpp
// Example of catching errors

#include "graph_resolver/core.h"
#include "graph_resolver/i_graph_resolver.h"
#include <iostream>
#include <string>
#include <stdexcept>

#include <graph_resolver/graph_resolver.h>

using namespace std;

struct ILogger
{
    virtual ~ILogger() = default;
    virtual void Log() = 0;
};

struct IWindow
{
    virtual ~IWindow() = default;
    virtual void PostConstruct() = 0;
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

// ------------------- DLL part ----------------------------
class Logger : public ILogger
{
public:
    Logger(string prog_name)
        {std::cout << "Logger constructor! Program name: " << prog_name <<endl;}
    ~Logger(){std::cout << "Logger destructor!\n";}
    void Log() override{cout << "Logger log!\n";}
};

class Window : public IWindow
{
public:
    Window(){std::cout << "Window constructor!\n";}
    ~Window(){std::cout << "Window destructor!\n";}
    void Run() override {cout << "Window run!\n";}
    void PostConstruct() override {cout << "Window post construct!\n";}
};

class Render : public IRender
{
public:
    Render(IWindow* w_ptr){std::cout << "Render constructor! Window module ptr: " <<w_ptr << endl;}
    ~Render(){std::cout << "Render destructor!\n";}
    void Run() override {cout << "Render run!\n";}
    void FuncForWindow() override {cout << "Render Func for window!\n";}
};

class AnotherRender : public IRender
{
public:
    AnotherRender(IWindow* w_ptr){std::cout << "Another Render constructor! Window module ptr: " <<w_ptr << endl;}
    ~AnotherRender(){std::cout << "Another Render destructor!\n";}
    void Run() override {cout << "Another Render run!\n";}
    void FuncForWindow() override {cout << "Another Render Func for window!\n";}
};

class Audio : public IAudio
{
public:
    Audio(){std::cout << "Audio constructor!\n";}
    ~Audio(){std::cout << "Audio destructor!\n";}
    void Run() override {cout << "Audio run!\n";}
};

using namespace graph_core;
using namespace graph_resolver;

void ErrorCatcher(const GraphResult& res)
{
    switch(res.code)
    {
        case GraphErrorCode::DuplicateModuleID: 
        {
            cout << "ERROR: Found duplication. ";
            cout << res.mod_name;
            cout << " Realized interface twice\n";
            break;
        }
        case GraphErrorCode::GraphNotCompiled:
        {
            cout << "ERROR: Graph wasnt compiled. ";
            break;
        }
        case GraphErrorCode::CycleDetected:
        {
            cout << "ERROR: Found cycle in graph. ";
            cout << "Trouble with module ";
            cout << res.mod_name << endl;
            break;
        }
        case GraphErrorCode::MissingDependancy:
        {
            cout << "ERROR: Missing dependancy. ";
            cout << "Missed in module ";
            cout << res.mod_name << endl;
        }
        default: break;
    }
    //throw runtime_error("Catched error!");
}

int main() {
    try {
        GraphResolver loader;

        // Miss dependancy for audio module

        // if(const auto res = loader.AddModule<ILogger, Logger>([](IGraphResolver* loader) {
        //     return new Logger("MyEngine");
        // });!res)
        // {
        //     ErrorCatcher(res);
        // }

        // Add cycle to graph
        if(const auto res = loader.AddModule<IWindow, Window, TypeList<IRender>, TypeList<IRender>>(
            [](IGraphResolver* loader) {
                return new Window();
            }, 
            [](IGraphResolver* loader, Window* ptr){
                IRender* render_ptr = GetModule<IRender>(loader);
                render_ptr->FuncForWindow();
                ptr->PostConstruct();
        }); !res)
        {
            ErrorCatcher(res);
        }

        if(const auto res = loader.AddModule<IRender, Render, TypeList<IWindow>>(
            [](IGraphResolver* loader) {
            IWindow* window_ptr = GetModule<IWindow>(loader);
            return new Render(window_ptr);
        }); !res)
        {
            ErrorCatcher(res);
        }

        // Add another module with same interface
        if(const auto res = loader.AddModule<IRender, AnotherRender, TypeList<IWindow>>(
            [](IGraphResolver* loader) {
            IWindow* window_ptr = GetModule<IWindow>(loader);
            return new AnotherRender(window_ptr);
        }); !res)
        {
            ErrorCatcher(res);
        }

        if(const auto res = loader.AddModule<IAudio, Audio, TypeList<ILogger>>([](IGraphResolver* loader) {
            return new Audio();
        });!res)
        {
            ErrorCatcher(res);
        }

        if(const auto res = loader.CompileGraph();!res)
        {
            ErrorCatcher(res);
        }

        if(const auto res = loader.ExecuteGraph(); !res)
        {
            ErrorCatcher(res);
        }

        loader.DestroyGraph();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}