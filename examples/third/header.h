#pragma once

#include "core.h"
#include <graph_resolver/api/graph_resolver_api.h>

typedef struct window window;
typedef struct render render;
typedef struct audio audio;

typedef struct render_api render_api;

typedef struct window_api
{
    uint32_t struct_size;

    void (CALL *post_construct)(window* self, const render_api* render_api_ptr, render* render_ptr);
    void (CALL *run)(window* self);
} window_api;

typedef struct render_api
{
    uint32_t struct_size;

    void (CALL *func_for_window)(render* self);
    void (CALL *run)(render* self);
} render_api;

typedef struct audio_api
{
    uint32_t struct_size;

    void (CALL *run)(audio* self);
} audio_api;

extern "C"
{
    PLUGIN_API void RegisterPlugins(const gr_GraphResolverApi*, gr_GraphResolver*);
    PLUGIN_API const window_api* CALL GetWindowApi();
    PLUGIN_API const render_api* CALL GetRenderApi();
    PLUGIN_API const audio_api* CALL GetAudioApi();
}