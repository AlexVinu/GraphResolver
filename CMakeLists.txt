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

# Fetch content
include(FetchContent)

# Fetch nameof
FetchContent_Declare(
    nameof
    GIT_REPOSITORY https://github.com/Neargye/nameof.git
    GIT_TAG        v0.10.4 
)
FetchContent_MakeAvailable(nameof)

# Target
add_library(graph_resolver)
add_library(graph_resolver::graph_resolver ALIAS graph_resolver)
target_compile_features(graph_resolver PUBLIC cxx_std_20)

# Connection modules and headers
target_sources(graph_resolver
    PRIVATE
        src/graph_resolver.cpp

    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES
            include/graph_resolver/graph_resolver.h
            include/graph_resolver/i_graph_resolver.h
            include/graph_resolver/core.h
            include/graph_resolver/module_slot_base.h
            include/graph_resolver/module_slot.h

            include/graph_resolver/utils/macro.h
            include/graph_resolver/utils/hash_string.h
)

target_include_directories(graph_resolver
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/utils"
)

# Linking externals
target_link_libraries(graph_resolver
    PUBLIC
        nameof::nameof
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