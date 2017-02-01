cmake_minimum_required(VERSION 2.8.9)

project(ptime)
set(PROJECT_VERSION 0.1.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Dependencies

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Determine if we should link with librt for libraries that use "clock_gettime"
  include(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
  if(NOT HAVE_CLOCK_GETTIME)
    find_library(LIBRT NAMES rt)
  endif()
endif()
if(NOT LIBRT)
  set(LIBRT "")
endif()

include_directories(${PROJECT_SOURCE_DIR}/inc)

# Tests

enable_testing()

add_executable(ptime_test src/ptime_test.c src/ptime.c)
target_link_libraries(ptime_test ${LIBRT})
add_test(ptime_test ${EXECUTABLE_OUTPUT_PATH}/ptime_test)
