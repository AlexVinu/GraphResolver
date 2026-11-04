#pragma once

#include "core.h"

struct gr_GraphResolver;

struct gr_ModuleSlotDescriptor
{
    uint32_t struct_size;

    gr_ConstructFn construct;
    gr_PostConstructFn post_construct;
    gr_DestructFn destruct;

    gr_modid_t module_id;
    gr_StringView type_name;

    gr_ModIdView deps;
    gr_ModIdView post_deps;
};