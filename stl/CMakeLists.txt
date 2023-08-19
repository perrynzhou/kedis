cmake_minimum_required(VERSION 3.5.1)
project(stl C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(
        stl SHARED
		stl_array.c
		stl_array.h
        stl_set.h
        stl_set.c
        stl_trie.h
        stl_trie.c
		stl_base64.c
		stl_base64.h
		stl_cond.c
		stl_cond.h
		stl_crc.c
		stl_crc.h
		stl_dict.c
		stl_dict.h
		stl_hash.c
		stl_hash.h
		stl_heap.c
		stl_heap.h
		stl_ini.c
		stl_ini.h
		stl_list.c
		stl_list.h
		stl_log.c
		stl_log.h
		stl_bloom_filter.c
		stl_bloom_filter.h
		stl_md5.c
		stl_md5.h
		stl_mmap.c
		stl_mmap.h
		stl_option.c
		stl_option.h
		stl_perf.c
		stl_perf.h
		stl_queue.c
		stl_queue.h
		stl_rand.c
		stl_rand.h
		stl_ringbuffer.c
		stl_ringbuffer.h
		stl_signal.c
		stl_signal.h
		stl_string.c
		stl_string.h
		stl_thread.c
		stl_thread.h
		stl_time.c
		stl_time.h
		stl_timer.c
		stl_timer.h
		stl_uri.c
		stl_uri.h)

target_include_directories(stl PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if (NOT CMAKE_C_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pedantic -Werror -Wall -Wextra")
endif ()


# --------------------------------------------------------------------------- #
# --------------------- Test Configuration Start ---------------------------- #
# --------------------------------------------------------------------------- #
if (SC_BUILD_TEST)

    include(CTest)
    include(CheckCCompilerFlag)

    if (SC_CLANG_TIDY)
        message(STATUS "Enabled CLANG_TIDY")

        set(CMAKE_C_CLANG_TIDY
                clang-tidy;
                -line-filter=[{"name":"${PROJECT_NAME}.h"},{"name":"${PROJECT_NAME}.c"}];
                -checks=clang-analyzer-*,misc-*,portability-*,bugprone-*;
                -warnings-as-errors=clang-analyzer-*,misc-*,portability-*,bugprone-*;)
    endif ()

    enable_testing()


    target_compile_options(${PROJECT_NAME}_test PRIVATE -DSC_SIZE_MAX=140000ul)

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
                "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
            target_compile_options(${PROJECT_NAME}_test PRIVATE -DSC_HAVE_WRAP)
            target_compile_options(${PROJECT_NAME}_test PRIVATE -fno-builtin)
            target_link_options(${PROJECT_NAME}_test PRIVATE -Wl,--wrap=malloc
                    -Wl,--wrap=snprintf -Wl,--wrap=sprintf -Wl,--wrap=strtoul)
        endif ()
    endif ()

    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang" OR
            "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME}_test PRIVATE -fno-omit-frame-pointer)

        if (SANITIZER)
            target_compile_options(${PROJECT_NAME}_test PRIVATE -fsanitize=${SANITIZER})
            target_link_options(${PROJECT_NAME}_test PRIVATE -fsanitize=${SANITIZER})
        endif ()
    endif ()

    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

    SET(MEMORYCHECK_COMMAND_OPTIONS
            "-q --log-fd=2 --trace-children=yes --track-origins=yes       \
           --leak-check=full --show-leak-kinds=all  \
           --error-exitcode=255")

    add_custom_target(valgrind_${PROJECT_NAME} ${CMAKE_COMMAND}
            -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
            --overwrite MemoryCheckCommandOptions=${MEMORYCHECK_COMMAND_OPTIONS}
            --verbose -T memcheck WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    add_custom_target(check_${PROJECT_NAME} ${CMAKE_COMMAND}
            -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} -C $<CONFIG> --verbose
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    # ----------------------- - Code Coverage Start ----------------------------- #

    if (${CMAKE_BUILD_TYPE} MATCHES "Coverage")
        if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
            target_compile_options(${PROJECT_NAME}_test PRIVATE --coverage)
            target_link_libraries(${PROJECT_NAME}_test gcov)
        else ()
            message(FATAL_ERROR "Only GCC is supported for coverage")
        endif ()
    endif ()

    add_custom_target(coverage_${PROJECT_NAME})
    add_custom_command(
            TARGET coverage_${PROJECT_NAME}
            COMMAND lcov --capture --directory .
            --output-file coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
            COMMAND lcov --remove coverage.info '/usr/*' '*example*' '*test*'
            --output-file coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
            COMMAND lcov --list coverage.info --rc lcov_branch_coverage=1 --rc lcov_excl_br_line='assert'
    )

    add_dependencies(coverage_${PROJECT_NAME} check_${PROJECT_NAME})

    # -------------------------- Code Coverage End ------------------------------ #
endif ()
# ----------------------- Test Configuration End ---------------------------- #

