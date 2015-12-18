find_package(Boost REQUIRED COMPONENTS program_options system thread)
find_package(Jsoncpp)
include_directories(${Boost_INCLUDE_DIR} ${Jsoncpp_INCLUDE_DIR})

add_executable(server server.cpp client.cpp)

target_link_libraries(server ${Boost_LIBRARIES} ${Jsoncpp_LIBRARY})
