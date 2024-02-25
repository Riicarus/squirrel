cmake_minimum_required(VERSION 3.20.0)
project(squirrel VERSION 0.1.0 LANGUAGES C)

set(CMAKE_BUILD_TYPE "Release")
if(NEED_TEST)
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif(NEED_TEST)

set(C_HASHMAP_INCLUDE ${PROJECT_SOURCE_DIR}/include)
# lib must be files, instead of dirs, the same as src
set(C_HASHMAP_LIB ${PROJECT_SOURCE_DIR}/lib/libc_hashmap.so)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/common/*.c ${PROJECT_SOURCE_DIR}/src/lex/*.c)

set(SRC_INCLUDE ${PROJECT_SOURCE_DIR}/src/common ${PROJECT_SOURCE_DIR}/src/lex)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${C_HASHMAP_INCLUDE} ${SRC_INCLUDE})

add_executable(${PROJECT_NAME} main.c ${SRC})
target_link_libraries(${PROJECT_NAME} ${C_HASHMAP_LIB})

# test
option(NEED_TEST OFF)
if(NEED_TEST)
    file(GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/test/*.c)
    set(TEST_NAME squirrel_test)
    add_executable(${TEST_NAME} ${SRC} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} ${C_HASHMAP_LIB})
endif(NEED_TEST)

