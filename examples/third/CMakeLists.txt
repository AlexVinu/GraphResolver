cmake_minimum_required(VERSION 3.28)
project(ExampleThird CXX)

add_executable(example_third 
                core.h
                header.h
                main.cpp)
add_library(example_third_lib SHARED
            core.h
            header.h
            plugin.cpp)

target_compile_definitions(example_third PRIVATE EXEC)
target_compile_definitions(example_third_lib PRIVATE PLUGIN)

set_target_properties(example_third PROPERTIES ENABLE_EXPORTS ON)

target_compile_features(example_third PRIVATE cxx_std_20)
target_compile_features(example_third_lib PRIVATE cxx_std_20)

foreach(target example_third example_third_lib)
    if(WIN32)
        target_compile_definitions(${target} PRIVATE WIN)
    elseif(APPLE)
        target_compile_definitions(${target} PRIVATE MAC)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(${target} PRIVATE LIN)
    endif()
endforeach()

target_link_libraries(example_third PRIVATE graph_resolver)
target_link_libraries(example_third_lib PRIVATE graph_resolver)

add_custom_command(TARGET example_third POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:example_third_lib>
        $<TARGET_FILE_DIR:example_third>
)