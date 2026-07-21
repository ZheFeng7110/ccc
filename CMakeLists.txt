cmake_minimum_required(VERSION 3.28...4.0)

file(READ "version.txt" cccVersion)
string(STRIP "${cccVersion}" cccVersion)
project(ccc LANGUAGES CXX VERSION ${cccVersion})

option(CCC_BUILD_TESTS "Build tests for library `ccc`" OFF)
option(CCC_USE_CPP_MODULES "Use C++ modules for library `ccc`" OFF)
option(CCC_BUILD_SHARED "Build shared library `ccc`" OFF)
option(CCC_INSTALL "Install library `ccc`" OFF)

message(STATUS "Configuring library ${PROJECT_NAME}@${PROJECT_VERSION}")
list(APPEND CMAKE_MESSAGE_INDENT "    ")

message(STATUS "CCC_BUILD_TESTS: ${CCC_BUILD_TESTS}")
message(STATUS "CCC_USE_CPP_MODULES: ${CCC_USE_CPP_MODULES}")
message(STATUS "CCC_BUILD_SHARED: ${CCC_BUILD_SHARED}")

if (CCC_BUILD_TESTS)
    enable_testing()

    if (MSVC)
        add_compile_options(/WX)
    else ()
        add_compile_options(-Werror)
    endif ()

    add_subdirectory(test)
endif ()

if (CCC_USE_CPP_MODULES)
    if (CCC_BUILD_SHARED)
        add_library(${PROJECT_NAME} SHARED)
    else ()
        add_library(${PROJECT_NAME} STATIC)
    endif ()

    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_sources(${PROJECT_NAME} PUBLIC FILE_SET modules TYPE CXX_MODULES FILES
        modules/ccc.ccm
        modules/ccc.utility.ccm
        modules/ccc.resource_guard.ccm
        modules/ccc.expected.ccm
        modules/ccc.pipe_operator_helper.ccm
        modules/ccc.inplace.ccm
        modules/ccc.inplace.inplace_vector.ccm
        modules/ccc.inplace.inplace_map.ccm
        modules/ccc.when.ccm
    )

    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus)
    endif ()
else ()
    add_library(${PROJECT_NAME} INTERFACE)

    target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif ()
