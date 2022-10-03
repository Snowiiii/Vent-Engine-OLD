cmake_minimum_required(VERSION 3.21)
project(vent VERSION 1.0.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(VENT_COMMON_DIR "${CMAKE_SOURCE_DIR}/Common")
set(VENT_RUNTIME_DIR "${CMAKE_SOURCE_DIR}/Runtime")
set(VENT_EDITOR_DIR "${CMAKE_SOURCE_DIR}/Editor")
set(VENT_DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")

# Dependencies
find_package(Vulkan REQUIRED)

if (NOT Vulkan_FOUND)
    message(ERROR "Could not find Vulkan")
endif ()

add_subdirectory("${VENT_DEPS_DIR}/volk")
add_subdirectory("${VENT_DEPS_DIR}/vma")
add_subdirectory("${VENT_DEPS_DIR}/glm")
add_subdirectory("${VENT_DEPS_DIR}/SDL2")
add_subdirectory("${VENT_DEPS_DIR}/stb")
add_subdirectory("${VENT_DEPS_DIR}/assimp")
add_subdirectory("${VENT_DEPS_DIR}/meshoptimizer")

# add_subdirectory("${VENT_DEPS_DIR}/imgui")


add_library(vulkan INTERFACE)
target_link_libraries(vulkan INTERFACE Vulkan::Vulkan)

target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

target_link_libraries(VulkanMemoryAllocator INTERFACE vulkan)
target_link_libraries(volk INTERFACE vulkan)

if (VKB_WSI_SELECTION STREQUAL XCB)
    target_include_directories(volk PUBLIC ${XCB_INCLUDE_DIRS})
elseif (VKB_WSI_SELECTION STREQUAL XLIB)
    target_include_directories(volk PUBLIC ${X11_INCLUDE_DIRS})
elseif (VKB_WSI_SELECTION STREQUAL WAYLAND)
    target_include_directories(volk PUBLIC ${WAYLAND_INCLUDE_DIRS})
endif ()



target_compile_definitions(glm INTERFACE
        GLM_FORCE_SWIZZLE
        GLM_FORCE_RADIANS
        GLM_FORCE_CTOR_INIT
        GLM_ENABLE_EXPERIMENTAL)

# ------------------------------------------

list(APPEND VENT_STATIC_LIBS
        SDL2-static
        glm::glm
        stb
        assimp
        meshoptimizer
        vulkan
        volk
        VulkanMemoryAllocator)

# Runtime
file(GLOB_RECURSE VENT_RUNTIME_HEADERS "${VENT_RUNTIME_DIR}/src/*.h*")
file(GLOB_RECURSE VENT_RUNTIME_SOURCES "${VENT_RUNTIME_DIR}/src/*.c*")

add_executable("${CMAKE_PROJECT_NAME}_runtime"
        ${VENT_RUNTIME_HEADERS}
        ${VENT_RUNTIME_SOURCES})

# target_precompile_headers("${CMAKE_PROJECT_NAME}_runtime" PRIVATE <vulkan/vulkan.hpp>)
target_link_libraries("${CMAKE_PROJECT_NAME}_runtime" ${VENT_STATIC_LIBS})

# Runtime as static library to link against
add_library("${CMAKE_PROJECT_NAME}_runtime_static"
        STATIC
        ${VENT_RUNTIME_HEADERS}
        ${VENT_RUNTIME_SOURCES})

# target_precompile_headers("${CMAKE_PROJECT_NAME}_runtime_static" PRIVATE <vulkan/vulkan.hpp>)
target_link_libraries("${CMAKE_PROJECT_NAME}_runtime_static" ${VENT_STATIC_LIBS})

# Editor
file(GLOB_RECURSE VENT_EDITOR_HEADERS "${VENT_EDITOR_DIR}/src/*.h*")
file(GLOB_RECURSE VENT_EDITOR_SOURCES "${VENT_EDITOR_DIR}/src/*.c*")

set(IMGUI_DIR ${VENT_DEPS_DIR}/imgui)
set(IMGUI_SOURCE_SPECIAL ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp)
file(GLOB IMGUI_SOURCES_HEADERS "${IMGUI_DIR}/*.h*") 
file(GLOB IMGUI_SOURCES_SOURCES "${IMGUI_DIR}/*.c*") 

add_library(ImGui STATIC ${IMGUI_SOURCES_HEADERS} ${IMGUI_SOURCES_SOURCES} ${IMGUI_SOURCE_SPECIAL})

target_include_directories(ImGui PUBLIC ${IMGUI_DIR})
target_link_libraries(ImGui SDL2-static)

add_executable("${CMAKE_PROJECT_NAME}_editor"
        ${VENT_EDITOR_HEADERS}
        ${VENT_EDITOR_SOURCES})

target_include_directories("${CMAKE_PROJECT_NAME}_editor" PUBLIC ${VENT_RUNTIME_DIR}/src/)
target_link_libraries("${CMAKE_PROJECT_NAME}_editor" PUBLIC "${CMAKE_PROJECT_NAME}_runtime_static")

target_include_directories("${CMAKE_PROJECT_NAME}_editor" PUBLIC ${IMGUI_DIR})
target_link_libraries("${CMAKE_PROJECT_NAME}_editor" PUBLIC ImGui)

# Shaders
set(VENT_RUNTIME_ASSET_DIR "${VENT_RUNTIME_DIR}/assets")
set(VENT_RUNTIME_TEXTURE_DIR "${VENT_RUNTIME_ASSET_DIR}/textures")
set(VENT_RUNTIME_SHADER_DIR "${VENT_RUNTIME_ASSET_DIR}/shaders")
set(VENT_RUNTIME_MESH_DIR "${VENT_RUNTIME_ASSET_DIR}/meshes")

set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets/shaders)
set(SHADER_SOURCE_DIR ${VENT_RUNTIME_SHADER_DIR})

file(GLOB SHADERS
        ${SHADER_SOURCE_DIR}/*.vert.glsl
        ${SHADER_SOURCE_DIR}/*.frag.glsl
        ${SHADER_SOURCE_DIR}/*.comp.glsl
        ${SHADER_SOURCE_DIR}/*.geom.glsl
        ${SHADER_SOURCE_DIR}/*.tesc.glsl
        ${SHADER_SOURCE_DIR}/*.tese.glsl
        ${SHADER_SOURCE_DIR}/*.mesh.glsl
        ${SHADER_SOURCE_DIR}/*.task.glsl
        ${SHADER_SOURCE_DIR}/*.rgen.glsl
        ${SHADER_SOURCE_DIR}/*.rchit.glsl
        ${SHADER_SOURCE_DIR}/*.rmiss)

message(DEBUG "Compiling shaders...")

add_custom_command(
COMMAND
  ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
OUTPUT ${SHADER_BINARY_DIR}
COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach (source ${SHADERS})
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
            OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
            COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V ${source} -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
            DEPENDS ${source} ${SHADER_BINARY_DIR}
            COMMENT "[SHADERS] Compiling ${FILENAME}"
    )
    list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach ()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_dependencies("${CMAKE_PROJECT_NAME}_runtime" shaders)
add_dependencies("${CMAKE_PROJECT_NAME}_runtime_static" shaders)

# Textures
# set(TEXTURES_SOURCE_DIR ${ASSETS_SOURCE_DIR}/textures)
# set(TEXTURES_SOURCE_DIR_OUT ${ASSETS_SOURCE_DIR_OUTPUT}/textures)

# message("Copying Textures")
# add_custom_command(
#     COMMAND
#       ${CMAKE_COMMAND} -E make_directory ${TEXTURES_SOURCE_DIR}
#     OUTPUT ${TEXTURES_SOURCE_DIR}
#     COMMENT "Creating ${TEXTURES_SOURCE_DIR}"
#   )
# 
# file(COPY ${TEXTURES_SOURCE_DIR} DESTINATION ${TEXTURES_SOURCE_DIR_OUT})


file(GLOB_RECURSE VENT_RUNTIME_SHADERS "${VENT_RUNTIME_SHADER_DIR}/*.glsl")
file(GLOB_RECURSE VENT_RUNTIME_TEXTURES "${VENT_RUNTIME_TEXTURE_DIR}/*.png")
file(GLOB_RECURSE VENT_RUNTIME_MESHES "${VENT_RUNTIME_MESH_DIR}/*.obj")

# test change zip to pak
# add_custom_target("${CMAKE_PROJECT_NAME}_assets"
#         COMMAND ${CMAKE_COMMAND} -E tar "cfv" "assets.zip" --format=zip
#         "${VENT_RUNTIME_SHADERS}"
#         "${VENT_RUNTIME_TEXTURES}"
#         "${VENT_RUNTIME_MESHES}"
#         OUTPUT "assets.zip"
#         )

# add_dependencies("${CMAKE_PROJECT_NAME}_runtime" "${CMAKE_PROJECT_NAME}_assets")
# add_dependencies("${CMAKE_PROJECT_NAME}_runtime_static" "${CMAKE_PROJECT_NAME}_assets")