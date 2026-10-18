// examples/first/main.cpp

#include "graph_resolver/i_graph_resolver.h"
#include <iostream>
#include <string>

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

class Audio : public IAudio
{
public:
    Audio(){std::cout << "Audio constructorr!\n";}
    ~Audio(){std::cout << "Audio destructor!\n";}
    void Run() override {cout << "Audio run!\n";}
};

int main() {
    try {
        using namespace graph_core;
        using namespace graph_resolver;
        GraphResolver loader;

        loader.AddModule<ILogger, Logger>([](IGraphResolver* loader) {
            return new Logger("MyEngine");
        });

        loader.AddModule<IWindow, Window, TypeList<ILogger>, TypeList<IRender>>(
            [](IGraphResolver* loader) {
                return new Window();
            }, 
            [](IGraphResolver* loader, Window* ptr){
                IRender* render_ptr = GetModule<IRender>(loader);
                render_ptr->FuncForWindow();
                ptr->PostConstruct();
            });

        loader.AddModule<IRender, Render, TypeList<ILogger, IWindow>>(
            [](IGraphResolver* loader) {
            IWindow* window_ptr = GetModule<IWindow>(loader);
            return new Render(window_ptr);
        });

        loader.AddModule<IAudio, Audio, TypeList<ILogger>>([](IGraphResolver* loader) {
            return new Audio();
        });

        loader.CompileGraph();
        loader.ExecuteGraph();

        if (auto* x = loader.GetModule<IRender>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<IWindow>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<ILogger>()) {
            x->Log();
        }
        if (auto* x = loader.GetModule<IAudio>()) {
            x->Run();
        }

        loader.DestroyGraph();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}