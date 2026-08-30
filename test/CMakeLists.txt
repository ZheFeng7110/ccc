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

file(DOWNLOAD
    "https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp"
    "${CMAKE_SOURCE_DIR}/.deps/catch2/catch2/catch.hpp"
    EXPECTED_HASH SHA256=3725C0F0A75F376A5005DDE31EAD0FEB8F7DA7507644C201B814443DE8355170
)
add_library(Catch2 INTERFACE)
target_include_directories(Catch2 INTERFACE "${CMAKE_SOURCE_DIR}/.deps/catch2")
add_library(Catch2::Catch2 ALIAS Catch2)

message(STATUS "CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION=${CMAKE_CXX_COMPILER_VERSION}")

if (MSVC)
    add_compile_options(/W4 /Zc:__cplusplus)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

if ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND
    (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 22))
    # Catch2 v2 uses __COUNTER__ inside TEST_CASE, which clang 22 reports as
    # a C2y extension under -Wpedantic
    add_compile_options(-Wno-c2y-extensions)
endif ()

if ((${CMAKE_CXX_COMPILER_ID} MATCHES "GNU") AND
    (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16))
    add_compile_options(-fdiagnostics-all-candidates -fdiagnostics-add-output=experimental-html)
    message(TRACE "Added -fdiagnostics-all-candidates for GCC 16 or higher")
endif ()

set(CMAKE_CXX_STANDARD "${CCC_TEST_CPP_STANDARD}"
    CACHE INTERNAL "" FORCE
)

add_executable(${PROJECT_NAME})

function (globSources cmd variable cxxStdRequired)
    if (CMAKE_CXX_STANDARD GREATER_EQUAL ${cxxStdRequired})
        if (${cmd} STREQUAL "FILES")
            set(${variable} ${ARGN} PARENT_SCOPE)
        else ()
            file(${cmd} tmpSources ${ARGN})
            set(${variable} ${tmpSources} PARENT_SCOPE)
        endif ()
    else ()
        set(${variable} "" PARENT_SCOPE)
    endif ()
endfunction ()

globSources(GLOB_RECURSE cccPipeOperatorHelperTestSources 17 "pipe_operator_helper_test/*.cc")
globSources(GLOB expectedTestSources 17 "expected_test/*.cc")

globSources(GLOB_RECURSE hashTestSources 11 "hash_test/*.cc")
# Constructing std::shared_ptr<T[]> from a raw pointer is a C++17 feature;
# libc++ rejects it in earlier modes
list(REMOVE_ITEM hashTestSources "${CMAKE_CURRENT_SOURCE_DIR}/hash_test/smart_ptr.cc")
globSources(GLOB hashSmartPtrTestSources 17 "hash_test/smart_ptr.cc")
globSources(GLOB_RECURSE inplaceTestSources 11 "inplace/*.cc")
globSources(FILES whenTestSources 17 when_test.cc)
globSources(GLOB_RECURSE utilityTestSources 11 "utility_test/*.cc")
target_sources(${PROJECT_NAME} PRIVATE
    catch_main.cc

    ${cccPipeOperatorHelperTestSources}
    ${expectedTestSources}
    ${hashTestSources}
    ${hashSmartPtrTestSources}
    ${inplaceTestSources}
    ${utilityTestSources}
    ${whenTestSources}
    resource_guard_test.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE ccc Catch2::Catch2)

if (NOT MSVC AND (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif ()

if (NOT APPLE)
    target_precompile_headers(${PROJECT_NAME} PRIVATE test_pch.hh)
    set_source_files_properties(catch_main.cc PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif ()

get_target_property(cccCppStandard ${PROJECT_NAME} CXX_STANDARD)
if ((${cccCppStandard} GREATER_EQUAL 20) AND (CCC_USE_CPP_MODULES))
    set(CMAKE_CXX_SCAN_FOR_MODULES ON CACHE INTERNAL "" FORCE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_USE_MODULE)
endif ()

add_test(
    NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
)
