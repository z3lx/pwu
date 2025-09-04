cmake_minimum_required(VERSION 3.28.2)
project(pwu LANGUAGES CXX)

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

set(HEADER_DIRS
    include
)

set(HEADER_FILES
    include/pwu/Console.hpp
    include/pwu/Dialogue.hpp
    include/pwu/ErrorHandling.hpp
    include/pwu/File.hpp
    include/pwu/FileInl.hpp
    include/pwu/Loader.hpp
    include/pwu/LoaderInl.hpp
    include/pwu/Resource.hpp
    include/pwu/ScopeExit.hpp
    include/pwu/Shell.hpp
    include/pwu/String.hpp
    include/pwu/StringInl.hpp
    include/pwu/TracedException.hpp
    include/pwu/UniqueResource.hpp
    include/pwu/Version.hpp
    include/pwu/VirtualKey.hpp
)

set(SOURCE_FILES
    src/Console.cpp
    src/Dialogue.cpp
    src/ErrorHandling.cpp
    src/Loader.cpp
    src/Shell.cpp
    src/TracedException.cpp
    src/Version.cpp
)

add_library(pwu STATIC)
add_library(pwu::pwu ALIAS pwu)

target_compile_features(pwu PUBLIC cxx_std_23)
set_target_properties(pwu PROPERTIES
    CXX_STANDARD_REQUIRED TRUE
    CXX_EXTENSIONS FALSE
)

target_sources(pwu
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${HEADER_DIRS}
        FILES ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)

target_link_libraries(pwu
    PUBLIC WIL::WIL
    PRIVATE Version.lib
)
