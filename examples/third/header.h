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

typedef struct window_adapter
{
    const window_api* g_window_api;
    window* g_window;
} window_adapter;

typedef struct render_api
{
    uint32_t struct_size;

    void (CALL *func_for_window)(render* self);
    void (CALL *run)(render* self);
} render_api;

typedef struct render_adapter
{
    const render_api* g_render_api;
    render* g_render;
} render_adapter;

typedef struct audio_api
{
    uint32_t struct_size;

    void (CALL *run)(audio* self);
} audio_api;

typedef struct audio_adapter
{
    const audio_api* g_audio_api;
    audio* g_audio;
} audio_adapter;

extern "C"
{
    PLUGIN_API void RegisterPlugins(const gr_GraphResolverApi*, gr_GraphResolver*) noexcept;
}