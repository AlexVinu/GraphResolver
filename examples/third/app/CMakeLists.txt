

add_executable(example_third 
                "${CMAKE_CURRENT_LIST_DIR}/../core.h"
                "${CMAKE_CURRENT_LIST_DIR}/../header.h"
                main.cpp)

target_include_directories(example_third PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/.."
)
target_link_libraries(example_third PRIVATE
    graph_resolver::graph_resolver
    GraphResolverPluginAPI
)

if(WIN32)
    target_compile_definitions(example_third PRIVATE WIN)
elseif(APPLE)
    target_compile_definitions(example_third PRIVATE MAC)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(example_third PRIVATE LIN)
endif()

add_custom_command(TARGET example_third POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${EXAMPLE_THIRD_PLUGIN_INSTALL_DIR}/${EXAMPLE_THIRD_PLUGIN_FILENAME}"
        "$<TARGET_FILE_DIR:example_third>/${EXAMPLE_THIRD_PLUGIN_FILENAME}"
    VERBATIM
)