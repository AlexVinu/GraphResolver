#include "graph_resolver/internal/graph_resolver.h"

#include <gtest/gtest.h>

namespace
{

struct ApiModule
{
    int value = 42;
};

TEST(GraphResolverApiTest, PluginApiRegistersExecutesAndDestroysModule)
{
    gr_GraphResolver graph;
    const auto* api = gr_GetGraphResolverApi();

    ASSERT_NE(api, nullptr);
    ASSERT_GE(api->struct_size, sizeof(gr_GraphResolverApi));
    ASSERT_NE(api->add_module_from_descriptor, nullptr);
    ASSERT_NE(api->compile_graph, nullptr);
    ASSERT_NE(api->execute_graph, nullptr);
    ASSERT_NE(api->get_module_int, nullptr);
    ASSERT_NE(api->destroy_graph, nullptr);

    gr_GraphResult add_result{};
    AddModule<ApiModule>(
        api,
        &graph,
        &add_result,
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new ApiModule);
        });

    ASSERT_TRUE(add_result);
    EXPECT_TRUE(api->has_module(&graph, hash_string(GET_TYPE_NAME(ApiModule))));
    EXPECT_EQ(api->get_module_int(&graph, hash_string(GET_TYPE_NAME(ApiModule))), nullptr);

    gr_GraphResult result{};
    api->compile_graph(&graph, &result);
    ASSERT_TRUE(result);
    api->execute_graph(&graph, &result);
    ASSERT_TRUE(result);

    auto* module = static_cast<ApiModule*>(
        api->get_module_int(&graph, hash_string(GET_TYPE_NAME(ApiModule))));
    ASSERT_NE(module, nullptr);
    EXPECT_EQ(module->value, 42);

    api->destroy_graph(&graph);
    EXPECT_FALSE(api->has_module(&graph, hash_string(GET_TYPE_NAME(ApiModule))));
}

TEST(GraphResolverApiTest, ReportsDuplicateModuleId)
{
    gr_GraphResolver graph;
    const auto* api = gr_GetGraphResolverApi();

    const auto construct = [](const gr_GraphResolverApi*, gr_GraphResolver*)
    {
        return static_cast<void*>(new ApiModule);
    };

    gr_GraphResult first_result{};
    AddModule<ApiModule>(api, &graph, &first_result, construct);
    ASSERT_TRUE(first_result);

    gr_GraphResult duplicate_result{};
    AddModule<ApiModule>(api, &graph, &duplicate_result, construct);

    EXPECT_EQ(duplicate_result.code, gr_GraphErrorCode::DuplicateModuleID);
}

} // namespace
