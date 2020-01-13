cmake_minimum_required(VERSION 3.10)

project(eseed_engine)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(dependencies/math)
add_subdirectory(dependencies/graphics)
add_subdirectory(dependencies/logging)

add_executable(eseed_engine src/main.cpp)
target_link_libraries(eseed_engine eseed_logging eseed_math eseed_graphics)

enable_testing()