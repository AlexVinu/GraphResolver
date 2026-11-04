// examples/first/main.cpp

#include <iostream>
#include <string>

#include <graph_resolver/internal/graph_resolver.h>

using namespace std;

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
    Audio(){std::cout << "Audio constructorr!\n";}
    ~Audio(){std::cout << "Audio destructor!\n";}
    void Run()  {cout << "Audio run!\n";}
};

class Input
{
public:
    Input(Window* window){std::cout << "Input constructor! Window module ptr: " << window << endl;}
    ~Input(){std::cout << "Input destructor!\n";}
    void Run()  {cout << "Input run!\n";}
};

class Physics
{
public:
    Physics(Logger* logger){std::cout << "Physics constructor! Logger module ptr: " << logger << endl;}
    ~Physics(){std::cout << "Physics destructor!\n";}
    void Run()  {cout << "Physics run!\n";}
};

class Scene
{
public:
    Scene(Render* render, Physics* physics){
        std::cout << "Scene constructor! Render module ptr: " << render
                  << ", Physics module ptr: " << physics << endl;
    }
    ~Scene(){std::cout << "Scene destructor!\n";}
    void Run()  {cout << "Scene run!\n";}
};

class Game
{
public:
    Game(Scene* scene, Input* input, Audio* audio){
        std::cout << "Game constructor! Scene module ptr: " << scene
                  << ", Input module ptr: " << input
                  << ", Audio module ptr: " << audio << endl;
    }
    ~Game(){std::cout << "Game destructor!\n";}
    void Run()  {cout << "Game run!\n";}
};

class EngineStats
{
public:
    EngineStats(Render* render, Physics* physics){
        std::cout << "EngineStats constructor! Render module ptr: " << render
                  << ", Physics module ptr: " << physics << endl;
    }
    ~EngineStats(){std::cout << "EngineStats destructor!\n";}
    void Run()  {cout << "EngineStats run!\n";}
};

int main() {
    try {
        std::cout << "Starting program...\n";
        gr_GraphResolver loader;

        std::cout << "Adding modules to graph...\n";

        loader.AddModule<Logger>([](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
            return static_cast<void*>(new Logger("MyEngine"));
        });

        loader.AddModule<Window, gr_TypeList<Logger>, gr_TypeList<Render>>(
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
                return static_cast<void*>(new Window());
            }, 
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader, void* ptr){
                Window* window_ptr = static_cast<Window*>(ptr);
                Render* render_ptr = loader->GetModule<Render>();
                render_ptr->FuncForWindow();
                window_ptr->PostConstruct();
            });

        loader.AddModule<Render, gr_TypeList<Logger, Window>>(
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
            Window* window_ptr = loader->GetModule<Window>();
            return static_cast<void*>(new Render(window_ptr));
        });

        loader.AddModule<Audio, gr_TypeList<Logger>>([](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
            return static_cast<void*>(new Audio());
        });

        loader.AddModule<Input, gr_TypeList<Window>>(
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
                Window* window_ptr = loader->GetModule<Window>();
                return static_cast<void*>(new Input(window_ptr));
            });

        loader.AddModule<Physics, gr_TypeList<Logger>>(
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
                Logger* logger_ptr = loader->GetModule<Logger>();
                return static_cast<void*>(new Physics(logger_ptr));
            });

        loader.AddModule<Scene, gr_TypeList<Render, Physics>>(
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
                Render* render_ptr = loader->GetModule<Render>();
                Physics* physics_ptr = loader->GetModule<Physics>();
                return static_cast<void*>(new Scene(render_ptr, physics_ptr));
            });

        loader.AddModule<Game, gr_TypeList<Scene, Input, Audio>>(
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
                Scene* scene_ptr = loader->GetModule<Scene>();
                Input* input_ptr = loader->GetModule<Input>();
                Audio* audio_ptr = loader->GetModule<Audio>();
                return static_cast<void*>(new Game(scene_ptr, input_ptr, audio_ptr));
            });

        loader.AddModule<EngineStats, gr_TypeList<Render, Physics>>(
            [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) {
                Render* render_ptr = loader->GetModule<Render>();
                Physics* physics_ptr = loader->GetModule<Physics>();
                return static_cast<void*>(new EngineStats(render_ptr, physics_ptr));
            });

        std::cout << "Compiling graph...\n";

        loader.CompileGraph();

        std::cout << "Executing graph...\n";

        loader.ExecuteGraph();

        if (auto* x = loader.GetModule<Render>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<Window>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<Logger>()) {
            x->Log();
        }
        if (auto* x = loader.GetModule<Audio>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<Input>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<Physics>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<Scene>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<Game>()) {
            x->Run();
        }
        if (auto* x = loader.GetModule<EngineStats>()) {
            x->Run();
        }

        std::cout << "Destroying graph...\n";

        loader.DestroyGraph();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}