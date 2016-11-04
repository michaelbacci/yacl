cmake_minimum_required(VERSION 3.3)
project(yacl CXX)

set (CMAKE_CXX_FLAGS "-std=c++0x")

option(EXAMPLES "compile the examples ?" ON)
option(TESTS "run the test ?" ON)

if (${EXAMPLES})
    include_directories(.)
    add_executable(subgroup examples/subgroup.cpp yacl.hpp)
endif()

if (${TESTS})

    if (NOT EXISTS "${PROJECT_SOURCE_DIR}/lib/googletest/CMakeLists.txt")
        message("Gooogle test library does not exist: downloading...")

        execute_process(
                COMMAND git submodule init
                COMMAND git submodule update
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                RESULT_VARIABLE git_result
                OUTPUT_VARIABLE git_output
                ERROR_VARIABLE git_error
        )

        if (NOT "${git_error}" STREQUAL "")
            message(FATAL_ERROR "Please, run:\ngit submodule init\ngit submodule update\nIn order to download the googletest library.")
        else()
            message(STATUS "Git result -> [${git_result}]")
            message(STATUS "Git output -> [${git_output}]")
        endif()
    endif()

    enable_testing()
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/googletest)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    include_directories(.)
    file(GLOB test_files test/*.cpp test/*.hpp)
    add_executable(RunUnitTests ${test_files})
    target_link_libraries(RunUnitTests gtest gtest_main)
    add_test(run-all RunUnitTests)

endif()