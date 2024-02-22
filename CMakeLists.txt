cmake_minimum_required(VERSION 3.20.0)
project(squirrel VERSION 0.1.0 LANGUAGES C)

set(C_HASHMAP_INCLUDE ${PROJECT_SOURCE_DIR}/include)
# lib must be files, instead of dirs, the same as src
set(C_HASHMAP_LIB ${PROJECT_SOURCE_DIR}/lib/libc_hashmap.so)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/common/*.c ${PROJECT_SOURCE_DIR}/src/front/*.c)

set(SRC_INCLUDE ${PROJECT_SOURCE_DIR}/src/common ${PROJECT_SOURCE_DIR}/src/front)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${C_HASHMAP_INCLUDE} ${SRC_INCLUDE})

add_executable(${PROJECT_NAME} main.c ${SRC})
target_link_libraries(${PROJECT_NAME} ${C_HASHMAP_LIB})

# test
option(NEED_TEST OFF)
if(NEED_TEST)
    file(GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/test/*.c)
    add_executable(squirrel_test ${TEST_SRC})
endif(NEED_TEST)

