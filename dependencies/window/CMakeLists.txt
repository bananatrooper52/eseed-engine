cmake_minimum_required(VERSION 3.10)

project(eseed_window)

find_package(Vulkan REQUIRED)

add_library(eseed_window
    src/window.cpp
    src/windowwin32.cpp
)
target_include_directories(eseed_window PUBLIC include/)
target_link_libraries(eseed_window Vulkan::Vulkan eseed_logging eseed_math)