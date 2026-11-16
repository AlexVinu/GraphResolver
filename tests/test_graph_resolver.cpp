#include "graph_resolver/api/core.h"
#include "graph_resolver/internal/graph_resolver.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{

struct RootModule { CREATE_MODULE(RootModule) };
struct DependentModule { CREATE_MODULE(DependentModule) };
struct MissingModule { CREATE_MODULE(MissingModule) };
struct CycleModuleA { CREATE_MODULE(CycleModuleA) };
struct CycleModuleB { CREATE_MODULE(CycleModuleB) };
struct FirstModule { CREATE_MODULE(FirstModule) };
struct SecondModule { CREATE_MODULE(SecondModule) };
struct ThirdModule { CREATE_MODULE(ThirdModule) };

std::vector<std::string> events;

struct GraphResolverFixture : testing::Test
{
    gr_GraphResolver graph;

    void SetUp() override
    {
        events.clear();
    }
};

TEST_F(GraphResolverFixture, AddsCompilesAndExecutesModule)
{
    const auto add_result = graph.AddModule<RootModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new RootModule);
        });

    ASSERT_TRUE(add_result);
    EXPECT_TRUE(graph.HasModule<RootModule>());
    EXPECT_EQ(graph.GetModule<RootModule>(), nullptr);

    ASSERT_TRUE(graph.CompileGraph());
    ASSERT_TRUE(graph.ExecuteGraph());
    EXPECT_NE(graph.GetModule<RootModule>(), nullptr);
}

TEST_F(GraphResolverFixture, RejectsDuplicateModuleId)
{
    ASSERT_TRUE(graph.AddModule<RootModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new RootModule);
        }));

    const auto result = graph.AddModule<RootModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new RootModule);
        });

    EXPECT_EQ(result.code, gr_GraphErrorCode::DuplicateModuleID);
    EXPECT_TRUE(graph.HasModule<RootModule>());
}

TEST_F(GraphResolverFixture, CompileRejectsMissingDependency)
{
    ASSERT_TRUE((graph.AddModule<DependentModule, gr_TypeList<MissingModule>>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new DependentModule);
        })));

    const auto result = graph.CompileGraph();
    EXPECT_EQ(result.code, gr_GraphErrorCode::MissingDependancy);
}

TEST_F(GraphResolverFixture, CompileIgnoresDuplicateDependencyEntries)
{
    ASSERT_TRUE(graph.AddModule<RootModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new RootModule);
        }));

    const gr_modid_t root_id = RootModule::ID;
    const gr_modid_t dependent_id = DependentModule::ID;
    std::array<gr_modid_t, 2> duplicated_deps = {root_id, root_id};

    gr_ModuleSlotDescriptor desc{};
    desc.struct_size = sizeof(gr_ModuleSlotDescriptor);
    desc.construct = [](const gr_GraphResolverApi*, gr_GraphResolver*)
    {
        return static_cast<void*>(new DependentModule);
    };
    desc.post_construct = nullptr;
    desc.destruct = [](void* ptr)
    {
        delete static_cast<DependentModule*>(ptr);
    };
    desc.module_id = dependent_id;
    const auto dependent_type_name = GET_MODULE_NAME(DependentModule);
    desc.type_name.data = dependent_type_name.data();
    desc.type_name.size = dependent_type_name.size();
    desc.deps = gr_ModIdView::From(duplicated_deps.data(), static_cast<gr_modcount_t>(duplicated_deps.size()));
    desc.post_deps = gr_ModIdView::From(nullptr, 0);

    ASSERT_TRUE(graph.AddModuleFromDescriptor(&desc));
    EXPECT_TRUE(graph.CompileGraph());
}

TEST_F(GraphResolverFixture, CompileRejectsDependencyCycle)
{
    ASSERT_TRUE((graph.AddModule<CycleModuleA, gr_TypeList<CycleModuleB>>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new CycleModuleA);
        })));
    ASSERT_TRUE((graph.AddModule<CycleModuleB, gr_TypeList<CycleModuleA>>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new CycleModuleB);
        })));

    EXPECT_EQ(graph.CompileGraph().code, gr_GraphErrorCode::CycleDetected);
}

TEST_F(GraphResolverFixture, ConstructionAndPostConstructionRespectDependencies)
{
    ASSERT_TRUE((graph.AddModule<RootModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            events.emplace_back("construct-root");
            return static_cast<void*>(new RootModule);
        },
        [](const gr_GraphResolverApi*, gr_GraphResolver*, void*)
        {
            events.emplace_back("post-root");
        })));

    ASSERT_TRUE((graph.AddModule<DependentModule, gr_TypeList<RootModule>,
        gr_TypeList<RootModule>>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            events.emplace_back("construct-dependent");
            return static_cast<void*>(new DependentModule);
        },
        [](const gr_GraphResolverApi*, gr_GraphResolver*, void*)
        {
            events.emplace_back("post-dependent");
        })));

    ASSERT_TRUE(graph.CompileGraph());
    ASSERT_TRUE(graph.ExecuteGraph());

    EXPECT_EQ(events, (std::vector<std::string>{
        "construct-root",
        "construct-dependent",
        "post-root",
        "post-dependent"
    }));
}

TEST_F(GraphResolverFixture, ExecuteRequiresCompileAndCannotRunTwice)
{
    EXPECT_EQ(graph.ExecuteGraph().code, gr_GraphErrorCode::GraphNotCompiled);
    ASSERT_TRUE(graph.CompileGraph());
    ASSERT_TRUE(graph.ExecuteGraph());
    EXPECT_EQ(graph.ExecuteGraph().code, gr_GraphErrorCode::GraphAlreadyExecuted);
}

TEST_F(GraphResolverFixture, DestructInOrderDestroysModulesInReverseDependencyOrder)
{
    ASSERT_TRUE((graph.AddModule<FirstModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new FirstModule);
        },
        nullptr,
        [](void* ptr)
        {
            events.emplace_back("destroy-first");
            delete static_cast<FirstModule*>(ptr);
        })));
    ASSERT_TRUE((graph.AddModule<SecondModule, gr_TypeList<FirstModule>>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new SecondModule);
        },
        nullptr,
        [](void* ptr)
        {
            events.emplace_back("destroy-second");
            delete static_cast<SecondModule*>(ptr);
        })));
    ASSERT_TRUE((graph.AddModule<ThirdModule, gr_TypeList<SecondModule>>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new ThirdModule);
        },
        nullptr,
        [](void* ptr)
        {
            events.emplace_back("destroy-third");
            delete static_cast<ThirdModule*>(ptr);
        })));

    ASSERT_TRUE(graph.CompileGraph());
    ASSERT_TRUE(graph.ExecuteGraph());
    graph.DestructInOrder();

    EXPECT_EQ(events, (std::vector<std::string>{
        "destroy-third",
        "destroy-second",
        "destroy-first"
    }));
    EXPECT_EQ(graph.GetModule<FirstModule>(), nullptr);
    EXPECT_EQ(graph.GetModule<SecondModule>(), nullptr);
    EXPECT_EQ(graph.GetModule<ThirdModule>(), nullptr);
}

TEST_F(GraphResolverFixture, DestroyGraphClearsModulesAndAllowsReuse)
{
    ASSERT_TRUE(graph.AddModule<RootModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new RootModule);
        }));
    ASSERT_TRUE(graph.CompileGraph());
    ASSERT_TRUE(graph.ExecuteGraph());

    graph.DestroyGraph();
    EXPECT_FALSE(graph.HasModule<RootModule>());

    ASSERT_TRUE(graph.AddModule<RootModule>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new RootModule);
        }));
    ASSERT_TRUE(graph.CompileGraph());
    ASSERT_TRUE(graph.ExecuteGraph());
    EXPECT_NE(graph.GetModule<RootModule>(), nullptr);
}

} // namespace
