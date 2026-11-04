cmake_minimum_required(VERSION 3.28)
project(ExampleThirdPlugin LANGUAGES CXX)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR
        "The third example plugin must be built with GNU GCC. "
        "Set PLUGIN_CXX_COMPILER to a GNU g++ executable.")
endif()

if(NOT TARGET GraphResolverPluginAPI)
    add_library(GraphResolverPluginAPI INTERFACE)
    target_include_directories(GraphResolverPluginAPI INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/../../../include"
    )
endif()

add_library(example_third_lib SHARED plugin.cpp)

target_include_directories(example_third_lib PRIVATE "${CMAKE_CURRENT_LIST_DIR}/..")
target_compile_definitions(example_third_lib PRIVATE PLUGIN)
target_compile_features(example_third_lib PRIVATE cxx_std_20)
target_link_libraries(example_third_lib PRIVATE GraphResolverPluginAPI)

if(WIN32)
    target_compile_definitions(example_third_lib PRIVATE WIN)
elseif(APPLE)
    target_compile_definitions(example_third_lib PRIVATE MAC)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(example_third_lib PRIVATE LIN)
endif()

install(TARGETS example_third_lib
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
)