cmake_minimum_required(VERSION 3.10)

project(eseed_math)

set(CMAKE_CXX_STANDARD 17)

add_library(eseed_math INTERFACE)
target_include_directories(eseed_math INTERFACE include/)