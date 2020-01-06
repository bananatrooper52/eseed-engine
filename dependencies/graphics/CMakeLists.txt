cmake_minimum_required(VERSION 3.10)

project(eseed_graphics)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_library(eseed_graphics
    src/window.cpp
    src/windowwindows.cpp
    src/rendercontext.cpp
)
target_include_directories(eseed_graphics PUBLIC include/)
target_link_libraries(eseed_graphics eseed_math Vulkan::Vulkan)