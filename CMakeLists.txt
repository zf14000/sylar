cmake_minimum_required(VERSION 4.2)
project(sylar)

# 指定C++标准 C++20，强制要求
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 关闭GNU扩展，严格标准C++20，可选

# C语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译选项
add_compile_options(
    -rdynamic
    -O3
    -fPIC
    -ggdb
    -Wall
    -Wno-deprecated
    -Werror
    -Wno-unused-function
    -Wno-builtin-macro-redefined
    -Wno-deprecated-declarations
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/utils
)

file(GLOB_RECURSE SYLAR_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(sylar SHARED ${SYLAR_SRC})

set(LIBS
    sylar
)

add_executable(test_log test/test_log.cpp)
add_dependencies(test_log sylar)
target_link_libraries(test_log ${LIBS})