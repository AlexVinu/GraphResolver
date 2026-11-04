cmake_minimum_required(VERSION 3.28)

if(NOT DEFINED CMAKE_CXX_COMPILER)
    find_program(EXAMPLE_THIRD_CLANGXX NAMES clang++ clang++-21 clang++-20)
    if(EXAMPLE_THIRD_CLANGXX)
        set(CMAKE_CXX_COMPILER "${EXAMPLE_THIRD_CLANGXX}" CACHE FILEPATH
            "Clang compiler used to build the example application")
    endif()
endif()

project(ExampleThird LANGUAGES CXX)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
        "The third example application must be built with Clang. "
        "Set CMAKE_CXX_COMPILER to clang++.")
endif()

include(ExternalProject)

if(NOT TARGET graph_resolver::graph_resolver)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../.." "${CMAKE_CURRENT_BINARY_DIR}/graph_resolver")
endif()

if(APPLE)
    set(_gnu_cxx_compiler_names
        g++-16 g++-15 g++-14 g++-13 g++-12 g++-11
    )
else()
    set(_gnu_cxx_compiler_names
        g++ g++-16 g++-15 g++-14 g++-13 g++-12 g++-11
    )
endif()

find_program(_EXAMPLE_THIRD_GXX NAMES ${_gnu_cxx_compiler_names})
unset(_gnu_cxx_compiler_names)

set(PLUGIN_CXX_COMPILER "${_EXAMPLE_THIRD_GXX}" CACHE FILEPATH
    "GNU C++ compiler used to build the plugin")
if(NOT PLUGIN_CXX_COMPILER)
    message(FATAL_ERROR
        "A GNU C++ compiler (g++) is required for the plugin. "
        "Install GCC or set PLUGIN_CXX_COMPILER to its g++ executable.")
endif()

set(EXAMPLE_THIRD_PLUGIN_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/plugin-install")
if(WIN32)
    set(EXAMPLE_THIRD_PLUGIN_FILENAME "example_third_lib.dll")
elseif(APPLE)
    set(EXAMPLE_THIRD_PLUGIN_FILENAME "libexample_third_lib.dylib")
else()
    set(EXAMPLE_THIRD_PLUGIN_FILENAME "libexample_third_lib.so")
endif()

add_subdirectory(app)

set(plugin_cmake_args
    "-DCMAKE_CXX_COMPILER:FILEPATH=${PLUGIN_CXX_COMPILER}"
    "-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>"
)

ExternalProject_Add(example_third_plugin
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/plugin"
    BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/plugin-build"
    INSTALL_DIR "${EXAMPLE_THIRD_PLUGIN_INSTALL_DIR}"
    CMAKE_ARGS ${plugin_cmake_args}
)

add_dependencies(example_third example_third_plugin)
