cmake_minimum_required(VERSION 3.28...4.0)

project(ccc
    LANGUAGES CXX
    VERSION "0.0.0"
    DESCRIPTION "A C++ utility library that provides modern C++ tools"
    HOMEPAGE_URL "https://github.com/ZheFeng7110/ccc"
)

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
    # Exported targets with CXX_MODULES file sets need cxx_std_20 among their
    # compile features, otherwise consumers fail generation with
    # "no C++ standard found" for the module collation target.
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

    target_sources(${PROJECT_NAME} PUBLIC FILE_SET modules TYPE CXX_MODULES FILES
        modules/ccc.ccm
        modules/ccc.utility.ccm
        modules/ccc.resource_guard.ccm
        modules/ccc.expected.ccm
        modules/ccc.hash.ccm
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

    if (MSVC)
        target_compile_options(${PROJECT_NAME} INTERFACE /Zc:__cplusplus)
    endif ()
endif ()

add_library(ccc::ccc ALIAS ${PROJECT_NAME})

if (CCC_INSTALL)
    if (CCC_USE_CPP_MODULES)
        include(cmake/InstallModulesTargets.cmake)
    else ()
        include(cmake/InstallInterfaceTargets.cmake)
    endif ()
endif ()
