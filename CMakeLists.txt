cmake_minimum_required(VERSION 3.5...3.28.3)
project(squirrel VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

add_executable(squirrel main.c src/common/hashmap.c test/hashmap_test.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
