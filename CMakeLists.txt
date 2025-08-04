cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

project(pwu)

include(FetchContent)

add_compile_definitions(WIL_EXCEPTION_MODE=1)
set(WIL_BUILD_PACKAGING OFF)
set(WIL_BUILD_TESTS OFF)
FetchContent_Declare(
    wil
    GIT_REPOSITORY https://github.com/microsoft/wil.git
    GIT_TAG v1.0.250325.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(wil)

file(GLOB_RECURSE SOURCES src/*)
add_library(pwu STATIC)
target_include_directories(pwu PRIVATE include)
target_sources(pwu PRIVATE ${SOURCES})
target_link_libraries(pwu PRIVATE WIL::WIL)
