cmake_minimum_required(VERSION 3.28)
project(ExampleFirst CXX)

add_executable(example_first main.cpp)

target_compile_features(example_first PRIVATE cxx_std_20)

target_link_libraries(example_first PRIVATE graph_resolver)