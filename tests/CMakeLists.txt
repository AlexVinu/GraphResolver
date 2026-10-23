cmake_minimum_required(VERSION 3.28)
project(tests_graph_resolver CXX)

include(FetchContent)

# Fetch googletest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_graph_resolver
        test_graph_resolver.cpp)

add_executable(test_i_graph_resolver
        test_i_graph_resolver.cpp)

target_link_libraries(test_graph_resolver
    PRIVATE
    GTest::gtest_main
    graph_resolver       
)

target_link_libraries(test_i_graph_resolver
    PRIVATE
    GTest::gtest_main
    graph_resolver       
)

include(GoogleTest)
gtest_discover_tests(test_graph_resolver)
gtest_discover_tests(test_i_graph_resolver)