cmake_minimum_required(VERSION 3.28)

project(graph_resolver LANGUAGES CXX)

# Cpp standart
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Fetch content
include(FetchContent)

FetchContent_Declare(
    nameof
    GIT_REPOSITORY https://github.com/Neargye/nameof.git
    GIT_TAG        v0.10.4 
)
FetchContent_MakeAvailable(nameof)

# Target
add_library(graph_resolver)

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
            include/graph_resolver/headers/graph_resolver.h
            include/graph_resolver/headers/core.h

            include/graph_resolver/utils/macro.h
            include/graph_resolver/utils/my_span.h
            include/graph_resolver/utils/hash_string.h
)

target_include_directories(graph_resolver
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Linking externals
target_link_libraries(graph_resolver
    PUBLIC
        nameof::nameof
)

add_subdirectory(examples/first)
add_subdirectory(examples/second)