cmake_minimum_required(VERSION 3.5.1)
project(stl_perf C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    add_library(
      stl_perf SHARED
      stl_perf.c
      stl_perf.h)

    target_include_directories(stl_perf PUBLIC ${CMAKE_CURRENT_LIST_DIR})

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pedantic -Werror -Wextra -Wall")
endif ()
