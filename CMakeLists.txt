cmake_minimum_required(VERSION 3.24)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/config.json" JSON_CONTENT)
string(JSON PARSED_PROJECT_NAME GET "${JSON_CONTENT}" "project_name")
string(JSON PARSED_VERSION GET "${JSON_CONTENT}" "version")

project(
    ${PARSED_PROJECT_NAME}
    VERSION ${PARSED_VERSION}
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default Build Type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Dynamic output directory
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE_LOWER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE_LOWER}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_TYPE_LOWER}")

add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")

file(GLOB_RECURSE PROJECT_SRC_FILES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
list(FILTER PROJECT_SRC_FILES EXCLUDE REGEX "/src/vendors/")

add_library(${PROJECT_NAME} MODULE
    ${PROJECT_SRC_FILES}
    src/vendors/ImGui/imgui.cpp
    src/vendors/ImGui/imgui_draw.cpp
    src/vendors/ImGui/imgui_tables.cpp
    src/vendors/ImGui/imgui_widgets.cpp
    src/vendors/ImGui/backends/imgui_impl_win32.cpp
    src/vendors/ImGui/backends/imgui_impl_dx11.cpp
    src/vendors/ImGui/backends/imgui_impl_dx12.cpp
    src/vendors/minhook/src/buffer.c
    src/vendors/minhook/src/hde/hde64.c
    src/vendors/minhook/src/hook.c
    src/vendors/minhook/src/trampoline.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/vendors
    src/vendors/ImGui
    src/vendors/ImGui/backends
    src/vendors/minhook/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        d3d11
        d3d12
        dxgi
        dwmapi
        d3dcompiler
        psapi
    )

    if(MINGW)
        # All
        target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)

        # Release
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
        target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-s>)
        target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:NDEBUG>)

        # Debug
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-O0 -g>)
        target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
    endif()
endif()
