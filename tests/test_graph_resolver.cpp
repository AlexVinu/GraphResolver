#include "graph_resolver/internal/graph_resolver.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace
{

struct RootModule {};
struct DependentModule {};
struct MissingModule {};
struct CycleModuleA {};
struct CycleModuleB {};
struct FirstModule {};
struct SecondModule {};
struct ThirdModule {};

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
