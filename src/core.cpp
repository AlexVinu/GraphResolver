#include "graph_resolver/api/core.h"

static_assert(std::is_standard_layout_v<gr_StringView>, "graph_resolver error: StringView is not standard_layout!");
static_assert(std::is_trivial_v<gr_StringView>, "graph_resolver error: StringView is not trivial!");

static_assert(std::is_standard_layout_v<gr_ModIdView>, "graph_resolver error: ModIdView is not standard_layout!");
static_assert(std::is_trivial_v<gr_ModIdView>, "graph_resolver error: ModIdView is not trivial!");

static_assert(std::is_standard_layout_v<gr_GraphResult>, "graph_resolver error: GraphResult is not standard_layout!");
static_assert(std::is_trivial_v<gr_GraphResult>, "graph_resolver error: GraphResult is not trivial!");