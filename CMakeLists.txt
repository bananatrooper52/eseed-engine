cmake_minimum_required(VERSION 3.10)

project(eseed_engine)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)

find_package(Vulkan REQUIRED)

add_subdirectory(dependencies/math)
add_subdirectory(dependencies/logging)
add_subdirectory(dependencies/window)

add_definitions(-DNOMINMAX)

add_executable(eseed_engine 
    src/main.cpp
    src/gpu/rendercontext.cpp
    src/gpu/presentmanager.cpp
    src/gpu/resourcemanager.cpp
    src/gpu/renderpipeline.cpp
)
target_link_libraries(eseed_engine eseed_logging eseed_math eseed_window)

enable_testing()