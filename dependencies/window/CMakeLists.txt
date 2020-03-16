cmake_minimum_required(VERSION 3.10)

project(eseed_window)

find_package(Vulkan REQUIRED)

add_library(eseed_window
    src/window.cpp
)
if(WIN32)
    target_sources(eseed_window PRIVATE src/windowwin32.cpp)
elseif(UNIX AND NOT APPLE)
    message(FATAL_ERROR "Linux windows are not implemented yet!")
else()
    message(FATAL_ERROR "MacOS windows are not implemented yet!")
endif()
target_include_directories(eseed_window PUBLIC include/)
target_link_libraries(eseed_window Vulkan::Vulkan eseed_logging eseed_math)