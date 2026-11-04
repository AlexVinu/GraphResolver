// examples/second/main.cpp
// Example of catching errors

#include <iostream>
#include <string>

#include <graph_resolver/internal/graph_resolver.h>

using namespace std;
// ------------------- DLL part ----------------------------
class Logger
{
public:
    Logger(string prog_name)
        {std::cout << "Logger constructor! Program name: " << prog_name <<endl;}
    ~Logger(){std::cout << "Logger destructor!\n";}
    void Log() {cout << "Logger log!\n";}
};

class Window
{
public:
    Window(){std::cout << "Window constructor!\n";}
    ~Window(){std::cout << "Window destructor!\n";}
    void Run()  {cout << "Window run!\n";}
    void PostConstruct()  {cout << "Window post construct!\n";}
};

class Render
{
public:
    Render(Window* w_ptr){std::cout << "Render constructor! Window module ptr: " <<w_ptr << endl;}
    ~Render(){std::cout << "Render destructor!\n";}
    void Run()  {cout << "Render run!\n";}
    void FuncForWindow()  {cout << "Render Func for window!\n";}
};
class Audio
{
public:
    Audio(){std::cout << "Audio constructor!\n";}
    ~Audio(){std::cout << "Audio destructor!\n";}
    void Run()  {cout << "Audio run!\n";}
};

void ErrorCatcher(const gr_GraphResult& res)
{
    switch(res.code)
    {
        case gr_GraphErrorCode::DuplicateModuleID: 
        {
            cout << "ERROR: Found duplication. ";
            cout.write(res.mod_name, res.mod_name_size);
            cout << " Realized interface twice\n";
            break;
        }
        case gr_GraphErrorCode::GraphNotCompiled:
        {
            cout << "ERROR: Graph wasnt compiled.\n";
            break;
        }
        case gr_GraphErrorCode::CycleDetected:
        {
            cout << "ERROR: Found cycle in graph. ";
            cout << "Trouble with module ";
            cout.write(res.mod_name, res.mod_name_size);
            cout << endl;
            break;
        }
        case gr_GraphErrorCode::MissingDependancy:
        {
            cout << "ERROR: Missing dependancy. ";
            cout << "Missed in module ";
            cout.write(res.mod_name, res.mod_name_size);
            cout << endl;
        }
        default: break;
    }
    //throw runtime_error("Catched error!");
}

void AddWindow(gr_GraphResolver& loader)
{
    if(const auto res = loader.AddModule<Window, gr_TypeList<>, gr_TypeList<Render>>(
        [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
            return static_cast<void*>(new Window());
        }, 
        [](const gr_GraphResolverApi* api, gr_GraphResolver* loader, void* ptr){
            Window* window_ptr = static_cast<Window*>(ptr);
            Render* render_ptr = loader->GetModule<Render>();
            render_ptr->FuncForWindow();
            window_ptr->PostConstruct();
    }); !res)
    {
        ErrorCatcher(res);
    }
}

void AddRender(gr_GraphResolver& loader)
{
    if(const auto res = loader.AddModule<Render, gr_TypeList<Window>>(
        [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
        Window* window_ptr = loader->GetModule<Window>();
        return static_cast<void*>(new Render(window_ptr));
    }); !res)
    {
        ErrorCatcher(res);
    }
}

void AddAudio(gr_GraphResolver& loader)
{
    if(const auto res = loader.AddModule<Audio, gr_TypeList<Logger>>([](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
        return static_cast<void*>(new Audio());
    });!res)
    {
        ErrorCatcher(res);
    }
}

void AddLogger(gr_GraphResolver& loader)
{
    if(const auto res = loader.AddModule<Logger>([](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
        return static_cast<void*>(new Logger("MyEngine"));
    });!res)
    {
        ErrorCatcher(res);
    }
}

void ExampleWithoutLogger()
{
    gr_GraphResolver loader;

    AddWindow(loader);
    AddRender(loader);
    AddAudio(loader);

    if(const auto res = loader.CompileGraph();!res)
    {
        ErrorCatcher(res);
    }

    if(const auto res = loader.ExecuteGraph(); !res)
    {
        ErrorCatcher(res);
    }

    loader.DestroyGraph();
}

void ExampleWithDuplicate()
{
    gr_GraphResolver loader;

    AddWindow(loader);
    AddRender(loader);
    AddAudio(loader);
    AddAudio(loader); // duplicate module to trigger error

    if(const auto res = loader.CompileGraph();!res)
    {
        ErrorCatcher(res);
    }

    if(const auto res = loader.ExecuteGraph(); !res)
    {
        ErrorCatcher(res);
    }

    loader.DestroyGraph();
}

void ExampleWithCycle()
{
    gr_GraphResolver loader;

    AddAudio(loader);
    AddLogger(loader);
    AddRender(loader);

    // Introduce a cycle by adding a dependency from Render to Window and from Window to Render
    if(const auto res = loader.AddModule<Window, gr_TypeList<Render>>(
        [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
        Render* render_ptr = loader->GetModule<Render>();
        return static_cast<void*>(new Window());
    }); !res)
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
}

void ExampleWithMissingDependency()
{
    gr_GraphResolver loader;

    AddRender(loader); // Render depends on Window, but Window is not added

    if(const auto res = loader.CompileGraph();!res)
    {
        ErrorCatcher(res);
    }

    if(const auto res = loader.ExecuteGraph(); !res)
    {
        ErrorCatcher(res);
    }

    loader.DestroyGraph();
}

void ExampleWithCycleInPostDeps()
{
    gr_GraphResolver loader;

    AddAudio(loader);
    AddLogger(loader);
    AddWindow(loader);

    // Introduce a cycle in post dependencies
    if(const auto res = loader.AddModule<Render, gr_TypeList<Window>, gr_TypeList<Window>>(
        [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
        Window* window_ptr = loader->GetModule<Window>();
        return static_cast<void*>(new Render(window_ptr));
    }); !res)
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
}

int main() {
    try {
        std::cout << "Example without logger:\n";
        ExampleWithoutLogger();
        std::cout << "Example with duplicate:\n";
        ExampleWithDuplicate();
        std::cout << "Example with cycle:\n";
        ExampleWithCycle();
        std::cout << "Example with missing dependency:\n";
        ExampleWithMissingDependency();
        std::cout << "Example with cycle in post dependencies:\n";
        ExampleWithCycleInPostDeps();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}