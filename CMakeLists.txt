cmake_minimum_required (VERSION 3.26.4)
project (Compiler)
set(CMAKE_CXX_STANDARD 23)
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
 
set(TESTS_ROOT ${PROJECT_SOURCE_DIR}/tests)
set(SRC_ROOT ${PROJECT_SOURCE_DIR}/src)
set(SRC
    ${SRC_ROOT}/SourceInput.cpp
    ${SRC_ROOT}/Tokenizer.cpp
    ${SRC_ROOT}/Parser.cpp
    ${SRC_ROOT}/Type.cpp
    ${TESTS_ROOT}/Source.cpp
)

add_executable (Compiler ${SRC})
target_include_directories(Compiler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(Compiler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")
target_link_directories(Compiler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/")