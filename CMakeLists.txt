cmake_minimum_required(VERSION 2.8.9)

project(ptime)
set(PROJECT_VERSION 0.1.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

include_directories(${PROJECT_SOURCE_DIR}/inc)

# Tests

enable_testing()

add_executable(ptime_test src/ptime_test.c src/ptime.c)
add_test(ptime_test ${EXECUTABLE_OUTPUT_PATH}/ptime_test)
