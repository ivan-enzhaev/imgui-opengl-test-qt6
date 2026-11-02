cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

project(imgui-opengl-test-qt6 CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -----------------------------------------------------------------------------
# Paths
# -----------------------------------------------------------------------------
if(WIN32)
    set(IMGUI_DIR "C:/libs/imgui-1.93.0")
    set(GLAD_DIR  "C:/libs/glad-0.1.36/opengl-3.3")
else()
    set(IMGUI_DIR "$ENV{HOME}/libs/imgui-1.93.0")
    set(GLAD_DIR  "$ENV{HOME}/libs/glad-0.1.36/opengl-3.3")
endif()

# -----------------------------------------------------------------------------
# Locate Qt6 Packages
# -----------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

# -----------------------------------------------------------------------------
# ImGui Sources
# -----------------------------------------------------------------------------
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# -----------------------------------------------------------------------------
# Executable Target
# -----------------------------------------------------------------------------
add_executable(app
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    ${IMGUI_SOURCES}
    ${GLAD_DIR}/src/glad.c
)

# -----------------------------------------------------------------------------
# Include Directories & Definitions
# -----------------------------------------------------------------------------
target_include_directories(app PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLAD_DIR}/include
)

target_compile_definitions(app PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)

# -----------------------------------------------------------------------------
# Link Libraries
# -----------------------------------------------------------------------------
target_link_libraries(app PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    opengl32
)

target_link_options(app PRIVATE -static-libgcc -static-libstdc++)

if(WIN32)
    target_link_options(app PRIVATE -static)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(app PRIVATE -mconsole)
    else()
        target_link_options(app PRIVATE -mwindows)
    endif()
endif()
