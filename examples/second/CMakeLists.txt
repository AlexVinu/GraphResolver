cmake_minimum_required(VERSION 3.28)
project(ExampleSecond CXX)

add_executable(example_second main.cpp)

target_compile_features(example_second PRIVATE cxx_std_20)

target_link_libraries(example_second PRIVATE graph_resolver)