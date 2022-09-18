cmake_minimum_required(VERSION 3.5)

project(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE include/)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
