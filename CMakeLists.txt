cmake_minimum_required (VERSION 3.26.4)
project (Compiler)
set(CMAKE_CXX_STANDARD 23)
 
set(TESTS_ROOT ${PROJECT_SOURCE_DIR}/tests)
set(SRC_ROOT ${PROJECT_SOURCE_DIR}/src)
set(SRC
)

add_executable (Compiler ${TESTS_ROOT}/Source.cpp ${SRC})
target_include_directories(Compiler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(Compiler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_link_directories(Compiler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/")