cmake_minimum_required(VERSION 3.1)

project(ptime VERSION 0.1.0
              LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
add_compile_options(-D_FORTIFY_SOURCE=2 -fstack-protector -g3 -pedantic -W -Wall -Wbad-function-cast -Wcast-align -Wcast-qual -Wdisabled-optimization -Wendif-labels -Wfloat-equal -Wformat=2 -Wformat-nonliteral -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wno-unused-parameter -Wpointer-arith -Wshadow -Wstrict-prototypes -Wstack-protector -Wswitch -Wundef -Wwrite-strings)

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
target_link_libraries(ptime_test PRIVATE ${LIBRT})
add_test(ptime_test ${EXECUTABLE_OUTPUT_PATH}/ptime_test)
