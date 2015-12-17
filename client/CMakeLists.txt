find_package(Boost 1.56 REQUIRED COMPONENTS program_options system thread)
include_directories(${Boost_INCLUDE_DIR})

add_executable(client client.cpp)

target_link_libraries(client ${Boost_LIBRARIES})
