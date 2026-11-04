#include "graph_resolver/api/module_slot_desc.h"

static_assert(std::is_standard_layout_v<gr_ModuleSlotDescriptor>, "graph_resolver error: ModuleSlotDescriptor is not standard_layout!");
static_assert(std::is_trivial_v<gr_ModuleSlotDescriptor>, "graph_resolver error: ModuleSlotDescriptor is not trivial!");