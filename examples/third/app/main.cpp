// examples/third/main.cpp
// Example of COM-like style

#include "graph_resolver/api/graph_resolver_api.h"
#include "header.h"

#include <iostream>
#include <string>
#include <filesystem>

#include <graph_resolver/internal/graph_resolver.h>

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

using namespace std;

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

gr_GraphResolver loader;

int main() 
{
    std::cout << "Application compiled by: ";
#if defined(__clang__)

    std::cout << "Clang "
              << __clang_major__ << "."
              << __clang_minor__ << "."
              << __clang_patchlevel__ << '\n';

#elif defined(_MSC_VER)

    std::cout << "MSVC " << _MSC_VER << '\n';

#elif defined(__GNUC__)

    std::cout << "GCC "
              << __GNUC__ << "."
              << __GNUC_MINOR__ << "."
              << __GNUC_PATCHLEVEL__ << '\n';

#else
    std::cout << "Unknown compiler\n";
#endif

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

    using RegisterPluginsFn = void(CALL *)(const gr_GraphResolverApi*, gr_GraphResolver*);
    auto register_plugins = reinterpret_cast<RegisterPluginsFn>(GetProc(lib, "RegisterPlugins"));
    if (register_plugins) {
        register_plugins(gr_GetGraphResolverApi(), &loader);
    }
    else {
        std::cerr << "Trouble with register_plugins\n";
    }

    loader.CompileGraph();
    loader.ExecuteGraph();

    if(window_adapter* window_adapter_ptr = loader.GetModule<window_adapter>())
    {
        window_adapter_ptr->g_window_api->run(window_adapter_ptr->g_window);
    }
    else {std::cerr << "Window wasnt loaded!\n";}
    if(render_adapter* render_adapter_ptr = loader.GetModule<render_adapter>())
    {
        render_adapter_ptr->g_render_api->run(render_adapter_ptr->g_render);
    } else {std::cerr << "Render wasnt loaded!\n";}
    if(audio_adapter* audio_adapter_ptr = loader.GetModule<audio_adapter>())
    {
        audio_adapter_ptr->g_audio_api->run(audio_adapter_ptr->g_audio);
    } else {std::cerr << "Audio wasnt loaded!\n";}
    
    loader.DestroyGraph();
    CloseLib(lib);
    std::cout << "End of example!\n";
}