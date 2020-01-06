cmake_minimum_required(VERSION 3.10)

project(eseed_logging)

set(CMAKE_CXX_STANDARD 17)

add_library(eseed_logging INTERFACE)
target_include_directories(eseed_logging INTERFACE include/)