cmake_minimum_required(VERSION 3.20)
project(AnotherEngine VERSION 0.1.0 LANGUAGES CXX)

# imgui
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
)

# implot
add_library(implot STATIC
    external/implot/implot.cpp
    external/implot/implot_items.cpp
)
target_include_directories(implot PUBLIC external/implot)
target_link_libraries(implot PUBLIC imgui)

# AnotherEngine
file(GLOB_RECURSE AOE_SOURCES CONFIGURE_DEPENDS
    include/vob/aoe/*.*
    src/vob/aoe/*.cpp
)

add_library(AnotherEngine STATIC ${AOE_SOURCES})
add_library(vob::aoe ALIAS AnotherEngine)

target_include_directories(AnotherEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/stb>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(AnotherEngine PUBLIC cxx_std_23)
target_compile_options(AnotherEngine PRIVATE /Zc:preprocessor)

target_compile_definitions(AnotherEngine PUBLIC
    GLM_ENABLE_EXPERIMENTAL
    GLM_FORCE_CXX17
    GLM_FORCE_SIZE_T_LENGTH
)

target_link_libraries(AnotherEngine PUBLIC
    assimp::assimp
    vob::misc
    imgui
    implot
    Tracy::TracyClient
    EnTT::EnTT
    glew_s
    glfw
    glm::glm
)

add_executable(ShaderExporter tools/ShaderExporter/main.cpp)
target_compile_features(ShaderExporter PRIVATE cxx_std_23)

if(MSVC)
    target_compile_options(AnotherEngine PRIVATE /W4 /wd4324)
endif()