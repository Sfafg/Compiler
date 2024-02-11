cmake_minimum_required (VERSION 3.26.4)
project (Parser)
set(CMAKE_CXX_STANDARD 23)
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
 
set(TESTS_ROOT ${PROJECT_SOURCE_DIR}/tests)
set(SRC_ROOT ${PROJECT_SOURCE_DIR}/src)
set(SRC
    ${TESTS_ROOT}/Source.cpp
)

add_executable (Parser ${SRC})
target_include_directories(Parser PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(Parser PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_link_directories(Parser PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/")