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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

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
    PUBLIC
        FILE_SET CXX_MODULES FILES
            src/graph_resolver.cppm src/core.cppm
        FILE_SET public_headers TYPE HEADERS FILES
            src/macro.h
    
    PRIVATE
        src/graph_resolver.cpp
)

# Linking externals
target_link_libraries(graph_resolver
    PRIVATE
        nameof::nameof
)

add_subdirectory(examples/first)
add_subdirectory(examples/second)