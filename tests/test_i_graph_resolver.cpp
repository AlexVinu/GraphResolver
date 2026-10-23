#include "graph_resolver/i_graph_resolver.h"
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
struct IGraphResolverFixture : public testing::Test
{
    IGraphResolver* i_graph_resolver;

    void SetUp() override
    {
        i_graph_resolver = new GraphResolver;
    }
    void TearDown() override
    {
        delete i_graph_resolver;
    }
};

TEST_F(IGraphResolverFixture, AddModule_SuccessfullyAddsNewModule)
{
    auto result = AddModule<IFirstModule, FirstModOfIFirst>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfIFirst;});
    ASSERT_TRUE(result);

    EXPECT_TRUE(HasModule<IFirstModule>(i_graph_resolver));
    ASSERT_TRUE(i_graph_resolver->CompileGraph());
    ASSERT_TRUE(i_graph_resolver->ExecuteGraph());

    EXPECT_NE(GetModule<IFirstModule>(i_graph_resolver), nullptr);
}

TEST_F(IGraphResolverFixture, AddModule_MultipleModulesWithDifferentInterfaces)
{
    AddModule<IFirstModule, FirstModOfIFirst>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfIFirst;});

    auto result = AddModule<ISecondModule, FirstModOfISecond>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfISecond;});
    
    ASSERT_TRUE(result);
    EXPECT_TRUE(HasModule<ISecondModule>(i_graph_resolver));
    EXPECT_TRUE(HasModule<IFirstModule>(i_graph_resolver));
    ASSERT_TRUE(i_graph_resolver->CompileGraph());
    ASSERT_TRUE(i_graph_resolver->ExecuteGraph());

    EXPECT_NE(GetModule<IFirstModule>(i_graph_resolver), nullptr);
    EXPECT_NE(GetModule<ISecondModule>(i_graph_resolver), nullptr);
}

TEST_F(IGraphResolverFixture, AddModule_DuplicateInterface)
{
    AddModule<IFirstModule, FirstModOfIFirst>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfIFirst;});

    auto res_ = AddModule<IFirstModule, SecondModOfIFirst>(i_graph_resolver,
        [](IGraphResolver*){return new SecondModOfIFirst;});
    
    EXPECT_EQ(res_.code, GraphErrorCode::DuplicateModuleID);
    EXPECT_TRUE(HasModule<IFirstModule>(i_graph_resolver));
    ASSERT_TRUE(i_graph_resolver->CompileGraph());
    ASSERT_TRUE(i_graph_resolver->ExecuteGraph());

    EXPECT_NE(GetModule<IFirstModule>(i_graph_resolver), nullptr);
}

TEST_F(IGraphResolverFixture, Compile_MissDependancy)
{
    AddModule<IFirstModule, FirstModOfIFirst, TypeList<ISecondModule>>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfIFirst;});

    ASSERT_EQ(i_graph_resolver->CompileGraph().code, GraphErrorCode::MissingDependancy);
}

TEST_F(IGraphResolverFixture, Compile_DependancyCycle)
{
    AddModule<IFirstModule, FirstModOfIFirst, TypeList<ISecondModule>>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfIFirst;});
    AddModule<ISecondModule, FirstModOfISecond, TypeList<IFirstModule>>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfISecond;});
    
    ASSERT_EQ(i_graph_resolver->CompileGraph().code, GraphErrorCode::CycleDetected);
}

TEST_F(IGraphResolverFixture, Compile_PostDependancyCycle)
{
    AddModule<IFirstModule, FirstModOfIFirst, TypeList<>, TypeList<ISecondModule>>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfIFirst;});
    AddModule<ISecondModule, FirstModOfISecond, TypeList<>, TypeList<IFirstModule>>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfISecond;});
    
    ASSERT_EQ(i_graph_resolver->CompileGraph().code, GraphErrorCode::CycleDetected);
}

TEST_F(IGraphResolverFixture, DestroyInOrder_TakeModuleAfterDestruction)
{
    AddModule<IFirstModule, FirstModOfIFirst>(i_graph_resolver,
        [](IGraphResolver*){return new FirstModOfIFirst;});
    i_graph_resolver->CompileGraph();
    i_graph_resolver->ExecuteGraph();
    ASSERT_NE(GetModule<IFirstModule>(i_graph_resolver), nullptr);
    i_graph_resolver->DestructInOrder();
    ASSERT_EQ(GetModule<IFirstModule>(i_graph_resolver), nullptr);
}