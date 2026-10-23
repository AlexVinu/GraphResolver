#include "graph_resolver/core.h"
#include <gtest/gtest.h>
#include <graph_resolver/graph_resolver.h>

using namespace graph_resolver;
using namespace graph_core;

// Interfaces of modules
struct IFirstModule
{
    virtual ~IFirstModule() = default;
};
struct ISecondModule
{
    virtual ~ISecondModule() = default;
};

// Realizations of modules
struct FirstModOfIFirst: public IFirstModule
{

};
struct SecondModOfIFirst: public IFirstModule
{

};
struct FirstModOfISecond: public ISecondModule
{

};

// Fixtures
struct GraphResolverFixture : public testing::Test
{
    GraphResolver graph_resolver;
};

TEST_F(GraphResolverFixture, AddModule_SuccessfullyAddsNewModule)
{
    auto result = graph_resolver.AddModule<IFirstModule, FirstModOfIFirst>(
        [](IGraphResolver*){return new FirstModOfIFirst;});
    ASSERT_TRUE(result);

    EXPECT_TRUE(graph_resolver.HasModule<IFirstModule>());
    ASSERT_TRUE(graph_resolver.CompileGraph());
    ASSERT_TRUE(graph_resolver.ExecuteGraph());

    EXPECT_NE(graph_resolver.GetModule<IFirstModule>(), nullptr);
}

TEST_F(GraphResolverFixture, AddModule_MultipleModulesWithDifferentInterfaces)
{
    graph_resolver.AddModule<IFirstModule, FirstModOfIFirst>(
        [](IGraphResolver*){return new FirstModOfIFirst;});

    auto result = graph_resolver.AddModule<ISecondModule, FirstModOfISecond>(
        [](IGraphResolver*){return new FirstModOfISecond;});
    
    ASSERT_TRUE(result);
    EXPECT_TRUE(graph_resolver.HasModule<ISecondModule>());
    EXPECT_TRUE(graph_resolver.HasModule<IFirstModule>());
    ASSERT_TRUE(graph_resolver.CompileGraph());
    ASSERT_TRUE(graph_resolver.ExecuteGraph());

    EXPECT_NE(graph_resolver.GetModule<IFirstModule>(), nullptr);
    EXPECT_NE(graph_resolver.GetModule<ISecondModule>(), nullptr);
}

TEST_F(GraphResolverFixture, AddModule_DuplicateInterface)
{
    graph_resolver.AddModule<IFirstModule, FirstModOfIFirst>(
        [](IGraphResolver*){return new FirstModOfIFirst;});

    auto res_ = graph_resolver.AddModule<IFirstModule, SecondModOfIFirst>(
        [](IGraphResolver*){return new SecondModOfIFirst;});
    
    EXPECT_EQ(res_.code, GraphErrorCode::DuplicateModuleID);
    EXPECT_TRUE(graph_resolver.HasModule<IFirstModule>());
    ASSERT_TRUE(graph_resolver.CompileGraph());
    ASSERT_TRUE(graph_resolver.ExecuteGraph());

    EXPECT_NE(graph_resolver.GetModule<IFirstModule>(), nullptr);
}

TEST_F(GraphResolverFixture, Compile_MissDependancy)
{
    graph_resolver.AddModule<IFirstModule, FirstModOfIFirst, TypeList<ISecondModule>>(
        [](IGraphResolver*){return new FirstModOfIFirst;});

    ASSERT_EQ(graph_resolver.CompileGraph().code, GraphErrorCode::MissingDependancy);
}

TEST_F(GraphResolverFixture, Compile_DependancyCycle)
{
    graph_resolver.AddModule<IFirstModule, FirstModOfIFirst, TypeList<ISecondModule>>(
        [](IGraphResolver*){return new FirstModOfIFirst;});
    graph_resolver.AddModule<ISecondModule, FirstModOfISecond, TypeList<IFirstModule>>(
        [](IGraphResolver*){return new FirstModOfISecond;});
    
    ASSERT_EQ(graph_resolver.CompileGraph().code, GraphErrorCode::CycleDetected);
}

TEST_F(GraphResolverFixture, Compile_PostDependancyCycle)
{
    graph_resolver.AddModule<IFirstModule, FirstModOfIFirst, TypeList<>, TypeList<ISecondModule>>(
        [](IGraphResolver*){return new FirstModOfIFirst;});
    graph_resolver.AddModule<ISecondModule, FirstModOfISecond, TypeList<>, TypeList<IFirstModule>>(
        [](IGraphResolver*){return new FirstModOfISecond;});
    
    ASSERT_EQ(graph_resolver.CompileGraph().code, GraphErrorCode::CycleDetected);
}

TEST_F(GraphResolverFixture, DestroyInOrder_TakeModuleAfterDestruction)
{
    graph_resolver.AddModule<IFirstModule, FirstModOfIFirst>(
        [](IGraphResolver*){return new FirstModOfIFirst;});
    graph_resolver.CompileGraph();
    graph_resolver.ExecuteGraph();
    ASSERT_NE(graph_resolver.GetModule<IFirstModule>(), nullptr);
    graph_resolver.DestructInOrder();
    ASSERT_EQ(graph_resolver.GetModule<IFirstModule>(), nullptr);
}