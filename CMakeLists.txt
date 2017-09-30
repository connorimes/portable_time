cmake_minimum_required(VERSION 2.8)

project(ptime)
set(PROJECT_VERSION 0.1.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2 -fstack-protector -g3 -pedantic -W -Wall -Wbad-function-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Wformat=2 -Wformat-nonliteral -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wno-unused-parameter -Wpointer-arith -Wshadow -Wstrict-prototypes -Wstack-protector -Wswitch -Wundef -Wwrite-strings -std=c99")

# Dependencies

if(UNIX AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Determine if we should link with librt for libraries that use "clock_gettime"
  include(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
  if(NOT HAVE_CLOCK_GETTIME)
    find_library(LIBRT NAMES rt)
  endif()
endif()

# Tests

enable_testing()

add_executable(ptime_test ptime_test.c ptime.c)
target_link_libraries(ptime_test ${LIBRT})
add_test(ptime_test ${EXECUTABLE_OUTPUT_PATH}/ptime_test)
