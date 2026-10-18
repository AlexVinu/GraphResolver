// examples/third/main.cpp
// Example of COM-like style

#include "graph_resolver/i_graph_resolver.h"
#include "header.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>

#include <graph_resolver/graph_resolver.h>

#if defined(WIN)
  #include <windows.h>
  using LibHandle = HMODULE;
  #define LoadLib(path) LoadLibraryA(path)
  #define GetProc(handle, name) GetProcAddress((HMODULE)(handle), name)
  #define CloseLib(handle) FreeLibrary((HMODULE)(handle))
#else
  #include <dlfcn.h>
  using LibHandle = void*;
  #define LoadLib(path) dlopen(path, RTLD_NOW)
  #define GetProc(handle, name) dlsym(handle, name)
  #define CloseLib(handle) dlclose(handle)
#endif

#if defined(MAC)
#include <mach-o/dyld.h>
#include <dlfcn.h>

std::string get_executable_dir() {
    char buf[1024];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        std::filesystem::path p(buf);
        return p.parent_path().string(); // Возвращает папку, где лежит сам example_third
    }
    return "";
}
#endif

using namespace graph_core;
using namespace graph_resolver;
using namespace std;

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
        default: break;
    }
    throw runtime_error("Catched error!");
}

GraphResolver loader;

graph_resolver::IGraphResolver* GetIGraphResolver()
{
    return &loader;
}

int main() 
{
#if defined(WIN)
    std::string lib_path = "example_third_lib.dll";
#elif defined(MAC)
    std::string lib_path = get_executable_dir() + "/libexample_third_lib.dylib";
#else
    std::string lib_path = "./libexample_third_lib.so";
#endif

    std::cout << "Trying to load from: " << lib_path << '\n';

    LibHandle lib = LoadLib(lib_path.c_str());

    if (!lib) {
    #if defined(MAC) || defined(LIN)
        std::cerr << "Failed to load plugin! Error: " << dlerror() << '\n';
    #else
        std::cerr << "Failed to load plugin! Error code: " << GetLastError() << '\n';
    #endif
        return 1;
    }

    using RegisterPluginsFn = void(*)(GetIGraphResolverFn);
    auto register_plugins = reinterpret_cast<RegisterPluginsFn>(GetProc(lib, "RegisterPlugins"));
    if (register_plugins) {
        register_plugins(GetIGraphResolver);
    }
    else {
        std::cerr << "Trouble with register_plugins\n";
    }

    loader.CompileGraph();
    loader.ExecuteGraph();

    if(IWindow* window_ptr = loader.GetModule<IWindow>())
    {
        window_ptr->Run();
    }
    else {std::cerr << "Window wasnt loaded!\n";}
    if(IRender* render_ptr = loader.GetModule<IRender>())
    {
        render_ptr->Run();
    } else {std::cerr << "Render wasnt loaded!\n";}
    if(IAudio* audio_ptr = loader.GetModule<IAudio>())
    {
        audio_ptr->Run();
    } else {std::cerr << "Audio wasnt loaded!\n";}
    
    loader.DestroyGraph();
    CloseLib(lib);
    std::cout << "End of example!\n";
}