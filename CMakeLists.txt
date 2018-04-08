cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(TCP)
add_definitions("-std=c++14")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(speech)
