cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(TCP)
add_definitions("-std=c++14")

add_subdirectory(server)
add_subdirectory(client)

