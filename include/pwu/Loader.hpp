#pragma once

#include <filesystem>
#include <string_view>

#include <Windows.h>

namespace pwu {
std::filesystem::path GetCurrentModuleFilePath();

std::filesystem::path GetModuleFilePath(HMODULE module);
std::filesystem::path GetModuleFilePath(const void* address);
std::filesystem::path GetModuleFilePath(std::string_view moduleName);
std::filesystem::path GetModuleFilePath(std::wstring_view moduleName);

void LoadRemoteLibrary(
    HANDLE processHandle,
    const std::filesystem::path& libraryPath
);
template <typename Container>
void LoadRemoteLibrary(
    HANDLE processHandle,
    const Container& libraryPaths
);
} // namespace pwu

#include "pwu/LoaderInl.hpp"
