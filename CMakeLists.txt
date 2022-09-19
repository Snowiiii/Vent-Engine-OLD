cmake_minimum_required(VERSION 3.13)

project(vent VERSION 1.0.0)

# Sets C++ standard to c++20
set(CMAKE_CXX_STANDARD 20)
# Sets C++ standard rquired to true
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SHADER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/shaders)

# find vulkan
find_package(Vulkan REQUIRED)

add_subdirectory(deps/glm)
add_subdirectory(deps/SDL2)
add_subdirectory(deps/stb)

if(Vulkan_FOUND)
    add_subdirectory(deps/volk)
    add_subdirectory(deps/vma)
endif(Vulkan_FOUND)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/src/*.h
        ${PROJECT_SOURCE_DIR}/src/*.hpp
        ${PROJECT_SOURCE_DIR}/src/*.c
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        )
add_executable(vent ${SRC_FILES})


target_link_libraries(vent PUBLIC glm::glm)

target_compile_definitions(glm INTERFACE
        GLM_FORCE_SWIZZLE
        GLM_FORCE_RADIANS
        GLM_FORCE_CTOR_INIT
        GLM_ENABLE_EXPERIMENTAL
        )
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_definitions(glm INTERFACE GLM_FORCE_CXX14)
endif ()


# stb
target_link_libraries(vent PUBLIC stb)

if (Vulkan_FOUND)
    add_library(vulkan INTERFACE)
    target_link_libraries(vulkan INTERFACE Vulkan::Vulkan)

    target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)
    target_compile_definitions(vulkan INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

    if (ANDROID)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
    elseif (WIN32)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
    elseif (APPLE)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_METAL_EXT)
    elseif (UNIX)
        # Choose WSI based on VKB_WSI_SELECTION
        if (VKB_WSI_SELECTION STREQUAL XCB OR VKB_WSI_SELECTION STREQUAL XLIB OR VKB_WSI_SELECTION STREQUAL WAYLAND)
            find_package(PkgConfig REQUIRED)
        endif ()
        if (VKB_WSI_SELECTION STREQUAL XCB)
            pkg_check_modules(XCB xcb REQUIRED)
            if (XCB_FOUND)
                target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
            endif ()
        elseif (VKB_WSI_SELECTION STREQUAL XLIB)
            pkg_check_modules(X11 x11 REQUIRED)
            if (X11_FOUND)
                target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XLIB_KHR)
            endif ()
        elseif (VKB_WSI_SELECTION STREQUAL WAYLAND)
            pkg_check_modules(WAYLAND wayland-client REQUIRED)
            if (WAYLAND_FOUND)
                target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
            endif ()
        elseif (VKB_WSI_SELECTION STREQUAL D2D)
            set(DIRECT_TO_DISPLAY TRUE)
            set(DIRECT_TO_DISPLAY TRUE PARENT_SCOPE)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_DISPLAY_KHR)
        endif ()
    endif ()

    # vma - 3.0.1
    add_library(vma INTERFACE)
    target_link_libraries(vma INTERFACE VulkanMemoryAllocator)
    target_link_libraries(vma INTERFACE vulkan)

    target_link_libraries(vent PUBLIC vma)

    # We use Volk
   # target_compile_definitions(vma INTERFACE VMA_STATIC_VULKAN_FUNCTIONS=0)
   # target_compile_definitions(vma INTERFACE VMA_DYNAMIC_VULKAN_FUNCTIONS=1)

    # volk - 1.3.215
    target_link_libraries(volk INTERFACE vulkan)
    target_link_libraries(vent PUBLIC volk)


    if (VKB_WSI_SELECTION STREQUAL XCB)
        target_include_directories(volk PUBLIC ${XCB_INCLUDE_DIRS})
    elseif (VKB_WSI_SELECTION STREQUAL XLIB)
        target_include_directories(volk PUBLIC ${X11_INCLUDE_DIRS})
    elseif (VKB_WSI_SELECTION STREQUAL WAYLAND)
        target_include_directories(volk PUBLIC ${WAYLAND_INCLUDE_DIRS})
    endif ()

endif (Vulkan_FOUND)

option(SDL_TESTS OFF)
option(SDL_INSTALL_TESTS OFF)

# SDL2 - 2.24.0
target_link_libraries(vent PUBLIC SDL2-static)

# target_precompile_headers(${PROJECT_NAME} PRIVATE <vulkan/vulkan.hpp>)

# Vulkan
if (Vulkan_FOUND)
    set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders/spv)

    file(GLOB SHADERS
            ${SHADER_SOURCE_DIR}/*.vert
            ${SHADER_SOURCE_DIR}/*.frag
            ${SHADER_SOURCE_DIR}/*.comp
            ${SHADER_SOURCE_DIR}/*.geom
            ${SHADER_SOURCE_DIR}/*.tesc
            ${SHADER_SOURCE_DIR}/*.tese
            ${SHADER_SOURCE_DIR}/*.mesh
            ${SHADER_SOURCE_DIR}/*.task
            ${SHADER_SOURCE_DIR}/*.rgen
            ${SHADER_SOURCE_DIR}/*.rchit
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
endif (Vulkan_FOUND)


# TODO: Add tests and install targets if needed.

