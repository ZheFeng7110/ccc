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
set(CCC_TASSERT_FORMAT_BACKEND "" CACHE STRING "tassert backend: empty=auto, 0=none, 1=std, 2=fmt")
set_property(CACHE CCC_TASSERT_FORMAT_BACKEND PROPERTY STRINGS "" 0 1 2)
set(CCC_TASSERT_CONFIG_HEADER "" CACHE FILEPATH "tassert inline constexpr ignore_tags configuration header")

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
        modules/ccc.hash.ccm
        modules/ccc.pipe_operator_helper.ccm
        modules/ccc.inplace.ccm
        modules/ccc.inplace.inplace_vector.ccm
        modules/ccc.inplace.inplace_map.ccm
        modules/ccc.when.ccm
        modules/ccc.tassert.ccm
    )

    # Must be PUBLIC: MSVC reports __cplusplus as 199711L without this flag,
    # so consumers compiling the installed module units fail the C++11 check
    # in include/ccc/detail/config.hh.
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PUBLIC /Zc:__cplusplus)
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

if (CCC_USE_CPP_MODULES)
    set(cccVisibility PUBLIC)
else ()
    set(cccVisibility INTERFACE)
endif ()

if (NOT CCC_TASSERT_FORMAT_BACKEND STREQUAL "")
    if (NOT CCC_TASSERT_FORMAT_BACKEND MATCHES "^[012]$")
        message(FATAL_ERROR "CCC_TASSERT_FORMAT_BACKEND must be 0, 1 or 2")
    endif ()
    target_compile_definitions(ccc ${cccVisibility}
        CCC_TASSERT_FORMAT_BACKEND=${CCC_TASSERT_FORMAT_BACKEND})
endif ()

if (CCC_TASSERT_FORMAT_BACKEND STREQUAL "1")
    target_compile_features(ccc ${cccVisibility} cxx_std_23)
    function (cccCheckStdFormat)
        set(CMAKE_CXX_STANDARD 23)
        include(CheckCXXSourceCompiles)
        check_cxx_source_compiles("#include <format>
            #if !defined(__cpp_lib_format) || __cpp_lib_format < 202110L
            #error unavailable std::format
            #endif
            int main() { return std::format(\"{}\", 1).size() != 1; }" CCC_HAS_STD_FORMAT)
        if (NOT CCC_HAS_STD_FORMAT)
            message(FATAL_ERROR "tassert standard backend requires usable C++23 std::format")
        endif ()
    endfunction ()
    cccCheckStdFormat()
elseif (CCC_TASSERT_FORMAT_BACKEND STREQUAL "2")
    find_package(fmt 11.2.0 CONFIG REQUIRED)
    target_link_libraries(ccc ${cccVisibility} fmt::fmt)
endif ()

if (CCC_TASSERT_CONFIG_HEADER)
    get_filename_component(cccTassertConfig "${CCC_TASSERT_CONFIG_HEADER}" ABSOLUTE)
    if (NOT EXISTS "${cccTassertConfig}")
        message(FATAL_ERROR "tassert configuration header does not exist: ${cccTassertConfig}")
    endif ()
    get_filename_component(cccTassertConfigName "${cccTassertConfig}" NAME)
    target_compile_definitions(ccc ${cccVisibility}
        "$<BUILD_INTERFACE:CCC_TASSERT_CONFIG_HEADER=\"${cccTassertConfig}\">"
        "$<INSTALL_INTERFACE:CCC_TASSERT_CONFIG_HEADER=<${cccTassertConfigName}$<ANGLE-R>>")
endif ()

if (CCC_INSTALL)
    if (CCC_USE_CPP_MODULES)
        include(cmake/InstallModulesTargets.cmake)
    else ()
        include(cmake/InstallInterfaceTargets.cmake)
    endif ()
endif ()
