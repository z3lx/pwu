cmake_minimum_required(VERSION 3.28.2)
project(pwu LANGUAGES CXX)

set(MODULE_FILES
    src/Console.ixx
    src/Dialogue.ixx
    src/ErrorHandling.ixx
    src/File.ixx
    src/Loader.ixx
    src/pwu.ixx
    src/Resource.ixx
    src/ScopeExit.ixx
    src/Shell.ixx
    src/String.ixx
    src/TracedException.ixx
    src/UniqueResource.ixx
    src/Version.ixx
    src/VirtualKey.ixx
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
    CXX_SCAN_FOR_MODULES MMH_BUILD_MODULES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN TRUE
)

target_sources(pwu
    PRIVATE
        FILE_SET CXX_MODULES
        FILES ${MODULE_FILES}
    PRIVATE
        ${SOURCE_FILES}
)

target_link_libraries(pwu
    PRIVATE Version.lib
)
