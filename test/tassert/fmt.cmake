include("${CMAKE_CURRENT_LIST_DIR}/get_cpm.cmake")
CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG main
    OPTIONS "FMT_TEST OFF" "FMT_DOC OFF" "FMT_INSTALL ON" "FMT_MODULE OFF"
)
set(fmt_DIR "${fmt_BINARY_DIR}" CACHE PATH "fmt package configuration" FORCE)
find_package(fmt 11.2.0 CONFIG REQUIRED)
