#include "header.h"

#include <cstddef>
#include <iostream>

using namespace std;

struct WindowImpl
{
    WindowImpl(){std::cout << "WindowImpl constructor!\n";}
    ~WindowImpl(){std::cout << "WindowImpl destructor!\n";}
    void Run(){cout << "WindowImpl run!\n";}
    void PostConstruct(const render_api* render_api_ptr, render* render_ptr)  
    {
        cout << "WindowImpl post construct! Call Render function\n";
        render_api_ptr->func_for_window(render_ptr);
    }
};

struct window
{
    WindowImpl* impl;
};

static void WindowRunImpl(window* self)
{
    if(self && self->impl) self->impl->Run();
}

static void WindowPostConstructImpl(window* self, const render_api* render_api_ptr, render* render_ptr)
{
    if(self && self->impl) self->impl->PostConstruct(render_api_ptr, render_ptr);
}

static const window_api g_window_api = {
    sizeof(window_api),
    &WindowPostConstructImpl,
    &WindowRunImpl
};

struct RenderImpl
{
    RenderImpl(const window_api* w_api, window* w_ptr){std::cout << "RenderImpl constructor! Window module ptr: " <<w_ptr << endl;}
    ~RenderImpl(){std::cout << "RenderImpl destructor!\n";}
    void Run()  {cout << "RenderImpl run!\n";}
    void FuncForWindow()  {cout << "RenderImpl Func for window!\n";}
};

struct render
{
    RenderImpl* impl;
};

static void RenderRunImpl(render* self)
{
    if(self && self->impl) self->impl->Run();
}

static void RenderFuncForWindowImpl(render* self)
{
    if(self && self->impl) self->impl->FuncForWindow();
}

static const render_api g_render_api = {
    sizeof(render_api),
    &RenderFuncForWindowImpl,
    &RenderRunImpl
};

struct AudioImpl
{
    AudioImpl(){std::cout << "AudioImpl constructor!\n";}
    ~AudioImpl(){std::cout << "AudioImpl destructor!\n";}
    void Run(){cout << "AudioImpl run!\n";}
};

struct audio
{
    AudioImpl* impl;
};

static void AudioRunImpl(audio* self)
{
    if(self && self->impl) self->impl->Run();
}

static const audio_api g_audio_api = {
    sizeof(audio_api),
    &AudioRunImpl
};

extern "C" PLUGIN_API void CALL RegisterPlugins(const gr_GraphResolverApi* graph_api, gr_GraphResolver* graph) noexcept
{
    try
    {
        std::cout << "Plugin compiled by: ";
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

        gr_GraphResult audio_result;
        AddModule<audio_adapter>(
            graph_api, 
            graph,
            &audio_result,
        [](const gr_GraphResolverApi* api, gr_GraphResolver* loader) noexcept
        {
            auto audio_impl = new AudioImpl;
            auto audio_ = new audio{audio_impl};
            return static_cast<void*>(new audio_adapter{&g_audio_api, audio_});
        },
        nullptr,
        [](void* ptr) noexcept
        {
            auto audio_adapter_ptr = static_cast<audio_adapter*>(ptr);
            delete audio_adapter_ptr->g_audio->impl;
            delete audio_adapter_ptr->g_audio;
            delete audio_adapter_ptr;
        }
    );

        gr_GraphResult window_result;
        AddModule<window_adapter, gr_TypeList<>, gr_TypeList<render_adapter>>(
            graph_api, 
            graph,
            &window_result,
        [](const gr_GraphResolverApi* api, gr_GraphResolver* graph) noexcept
        {
            auto window_impl = new WindowImpl;
            auto window_ = new window{window_impl};
            return static_cast<void*>(new window_adapter{&g_window_api, window_});
        },
        [](const gr_GraphResolverApi* api, gr_GraphResolver* graph, void* ptr) noexcept
        {
            auto self_ptr = static_cast<window_adapter*>(ptr);
            auto* render_adapter_ptr = GetModule<render_adapter>(api, graph);
            g_window_api.post_construct(self_ptr->g_window, &g_render_api, render_adapter_ptr->g_render);
        },
        [](void* ptr) noexcept
        {
            auto window_adapter_ptr = static_cast<window_adapter*>(ptr);
            delete window_adapter_ptr->g_window->impl;
            delete window_adapter_ptr->g_window;
            delete window_adapter_ptr;
        }
    );

        gr_GraphResult render_result;
        AddModule<render_adapter, gr_TypeList<window_adapter>>(
            graph_api,
            graph,
            &render_result,
        [](const gr_GraphResolverApi* api, gr_GraphResolver* graph) noexcept
        {
            window_adapter* window_adapter_ptr = GetModule<window_adapter>(api, graph);
            if(!window_adapter_ptr) {std::cout << "[PLUGIN ERROR]: window_adapter_ptr not found!\n";}
            auto render_impl = new RenderImpl(&g_window_api, window_adapter_ptr->g_window);
            auto render_ = new render{render_impl};
            return static_cast<void*>(new render_adapter{&g_render_api, render_});
        },
        nullptr,
        [](void* ptr) noexcept
        {
            auto render_adapter_ptr = static_cast<render_adapter*>(ptr);
            delete render_adapter_ptr->g_render->impl;
            delete render_adapter_ptr->g_render;
            delete render_adapter_ptr;
        }
        );
    }
    catch(...)
    {
        std::cerr << "[ERROR]: Plugin throwed a runtime error!\n";
    }
}