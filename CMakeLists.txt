cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(graph_resolver LANGUAGES CXX)
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(GRAPH_RESOLVER_STANDALONE_DEFAULT ON)
else()
    set(GRAPH_RESOLVER_STANDALONE_DEFAULT OFF)
endif()
option(GRAPH_RESOLVER_BUILD_EXAMPLES "Build graph_resolver examples" ${GRAPH_RESOLVER_STANDALONE_DEFAULT})
option(GRAPH_RESOLVER_BUILD_TESTS "Build graph_resolver tests" ${GRAPH_RESOLVER_STANDALONE_DEFAULT})
unset(GRAPH_RESOLVER_STANDALONE_DEFAULT)

# Target
add_library(graph_resolver)
add_library(graph_resolver::graph_resolver ALIAS graph_resolver)
add_library(GraphResolverPluginAPI INTERFACE)

target_include_directories(GraphResolverPluginAPI INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(graph_resolver PUBLIC cxx_std_20)

# Connection modules and headers
target_sources(graph_resolver
    PRIVATE
        src/graph_resolver.cpp
        src/module_slot.cpp
        src/core.cpp
        src/module_slot_desc.cpp

    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES
            include/graph_resolver/internal/graph_resolver.h
            include/graph_resolver/internal/module_slot.h

            include/graph_resolver/api/graph_resolver_api.h
            include/graph_resolver/api/core.h
            include/graph_resolver/api/module_slot_desc.h

            include/graph_resolver/utils/nameof.hpp
            include/graph_resolver/utils/macro.h
            include/graph_resolver/utils/hash_string.h
)

target_include_directories(graph_resolver
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/utils"
)

if(GRAPH_RESOLVER_BUILD_EXAMPLES)
    add_subdirectory(examples/first)
    add_subdirectory(examples/second)
    add_subdirectory(examples/third)
endif()

if(GRAPH_RESOLVER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()