cmake_minimum_required(VERSION 3.28...4.0)

project(ccc_test CXX)

message(STATUS "Configuring tests for library ccc (target: ${PROJECT_NAME})")
list(APPEND CMAKE_MESSAGE_INDENT "    ")

set(CCC_TEST_CPP_STANDARD 17 CACHE STRING
    "Set the C++ standard for testing."
)
set_property(CACHE CCC_TEST_CPP_STANDARD
    PROPERTY STRINGS "11;14;17;20;23"
)

if ((${CCC_TEST_CPP_STANDARD} LESS 20) AND (CCC_USE_CPP_MODULES))
    message(FATAL_ERROR "Use modules requires C++20 or higher!")
endif ()

include("get_cpm.cmake")
block (SCOPE_FOR POLICIES)
    cmake_policy(VERSION 3.28...4.0)

    CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG "v1.17.0"
        VERSION 1.17.0
        OPTIONS "BUILD_GMOCK OFF" "INSTALL_GTEST OFF"
    )

    if (NOT googletest_ADDED)
        message(FATAL_ERROR "Failed to add googletest")
    endif ()
endblock ()

if (MSVC)
    add_compile_options(/W4 /Zc:__cplusplus)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

set(CMAKE_CXX_STANDARD "${CCC_TEST_CPP_STANDARD}"
    CACHE INTERNAL "" FORCE
)

add_executable(${PROJECT_NAME})

function (globSources cmd variable cxxStdRequired)
    if (${cxxStdRequired} LESS_EQUAL CMAKE_CXX_STANDARD)
        file(${cmd} tmpSources ${ARGN})
        set(${variable} ${tmpSources} PARENT_SCOPE)
    else ()
        set(${variable} "" PARENT_SCOPE)
    endif ()
endfunction ()

globSources(GLOB_RECURSE cccPipeOperatorHelperTestSources 11 "pipe_operator_helper_test/*.cc")
target_sources(${PROJECT_NAME} PRIVATE
    ${cccPipeOperatorHelperTestSources}
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE ccc GTest::gtest_main)

if (NOT MSVC AND (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif ()

target_precompile_headers(${PROJECT_NAME} PRIVATE test_pch.hh)

get_target_property(cccCppStandard ${PROJECT_NAME} CXX_STANDARD)
if ((${cccCppStandard} GREATER_EQUAL 20) AND (CCC_USE_CPP_MODULES))
    set(CMAKE_CXX_SCAN_FOR_MODULES ON CACHE INTERNAL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_USE_MODULE)
endif ()

add_test(
    NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
)
