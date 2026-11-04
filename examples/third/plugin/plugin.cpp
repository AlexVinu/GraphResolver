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

extern "C" PLUGIN_API const window_api* CALL GetWindowApi()
{
    return &g_window_api;
}

extern "C" PLUGIN_API const render_api* CALL GetRenderApi()
{
    return &g_render_api;
}

extern "C" PLUGIN_API const audio_api* CALL GetAudioApi()
{
    return &g_audio_api;
}

extern "C" PLUGIN_API void CALL RegisterPlugins(const gr_GraphResolverApi* graph_api, gr_GraphResolver* graph)
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
    AddModule<audio>(
        graph_api, 
        graph,
        &audio_result,
    [](const gr_GraphResolverApi* api, gr_GraphResolver* loader)
    {
        auto audio_impl = new AudioImpl;
        return static_cast<void*>(new audio{audio_impl});
    },
    nullptr,
    [](void* ptr)
    {
        auto audio_ptr = static_cast<audio*>(ptr);
        delete audio_ptr->impl;
        delete audio_ptr;
    }
);

    gr_GraphResult window_result;
    AddModule<window, gr_TypeList<>, gr_TypeList<render>>(
        graph_api, 
        graph,
        &window_result,
    [](const gr_GraphResolverApi* api, gr_GraphResolver* graph)
    {
        auto window_impl = new WindowImpl;
        return static_cast<void*>(new window{window_impl});
    },
    [](const gr_GraphResolverApi* api, gr_GraphResolver* graph, void* ptr)
    {
        auto self_ptr = static_cast<window*>(ptr);
        render* render_ptr = GetModule<render>(api, graph);
        g_window_api.post_construct(self_ptr, &g_render_api, render_ptr);
    },
    [](void* ptr)
    {
        auto window_ptr = static_cast<window*>(ptr);
        delete window_ptr->impl;
        delete window_ptr;
    }
);

    gr_GraphResult render_result;
    AddModule<render, gr_TypeList<window>>(
        graph_api,
        graph,
        &render_result,
    [](const gr_GraphResolverApi* api, gr_GraphResolver* graph)
    {
        window* window_ptr = GetModule<window>(api, graph);
        auto render_impl = new RenderImpl(&g_window_api, window_ptr);
        return static_cast<void*>(new render(render_impl));
    },
    nullptr,
    [](void* ptr)
    {
        auto render_ptr = static_cast<render*>(ptr);
        delete render_ptr->impl;
        delete render_ptr;
    }
);

}