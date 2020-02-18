cmake_minimum_required(VERSION 3.10)

project(eseed_gpu)

find_package(Vulkan REQUIRED)

add_library(eseed_gpu
    src/rendercontext.cpp
)
target_include_directories(eseed_gpu PUBLIC include/)
target_link_libraries(eseed_gpu Vulkan::Vulkan eseed_logging eseed_window)