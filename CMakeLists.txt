cmake_minimum_required(VERSION 3.22)
project(phy_sim)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(ext ext_build)

include_directories(include)
include_directories(ext/glm)
include_directories(ext/glfw/include)
include_directories(ext/tinyobjloader)
include_directories(ext/imgui/include)

add_executable(phy_sim src/main.cpp src/Application.cpp src/camera.cpp src/utility.cpp src/vk_context.cpp src/vk_resource_manager.cpp include/vulkan_memory_utils.h include/vulkan_image_utils.h include/vk_swap_chain.h include/vk_graphics_pipeline.h include/vk_command_manager.h include/vk_renderpass.h src/VkBootstrap.cpp src/spirv_reflect.c include/vk_pipeline.h include/vk_descriptor.h src/my_context.cpp include/vk_device.h include/mesh.h)

find_package(glfw3 REQUIRED)
target_link_libraries(phy_sim PRIVATE glfw)

find_package(Vulkan REQUIRED FATAL_ERROR)
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    #set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    #STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
    message(STATUS "Successfully located the Vulkan SDK at: ${Vulkan_INCLUDE_DIRS}")
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()

target_link_libraries (phy_sim PRIVATE Vulkan::Vulkan)

target_link_libraries(phy_sim PRIVATE imgui)