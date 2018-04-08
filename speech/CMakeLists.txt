project (speech)

find_package(Boost 1.55 REQUIRED COMPONENTS program_options system thread)
include_directories(${Boost_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

find_package(PocketSphinx REQUIRED)
find_package(SphinxBase REQUIRED)
include_directories(${POCKETSPHINX_INCLUDE_DIRS} ${SPHINXBASE_INCLUDE_DIRS})
set(LIBS ${LIBS} ${POCKETSPHINX_LIBRARIES} ${SPHINXBASE_LIBRARIES} ${SPHINXAD_LIBRARIES})
include_directories(/usr/local/include/sphinxbase /usr/local/include/pocketsphinx)
target_link_libraries(${PROJECT_NAME} ${LIBS} ${Boost_LIBRARIES} -lpthread)
