module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

module pwu;

import :Loader;
import :ErrorHandling;

import std;

namespace fs = std::filesystem;

namespace pwu {
fs::path GetModuleFilePath(const HMODULE module) {
    std::array<wchar_t, MAX_PATH> buffer {};
    const DWORD stringSize = GetModuleFileNameW(
        module,
        buffer.data(),
        buffer.size()
    );
    ThrowLastWin32ErrorIf(
        stringSize == buffer.size() ||
        stringSize == 0
    );
    return fs::path { buffer.data() };
}

fs::path GetModuleFilePath(const void* address) {
    HMODULE module {};
    ThrowIfWin32BoolFalse(GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        static_cast<LPCTSTR>(address), &module
    ));
    return GetModuleFilePath(module);
}

std::filesystem::path GetModuleFilePath(const std::string_view moduleName) {
    HMODULE module {};
    ThrowIfWin32BoolFalse(GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        moduleName.data(), &module
    ));
    return GetModuleFilePath(module);
}

std::filesystem::path GetModuleFilePath(const std::wstring_view moduleName) {
    HMODULE module {};
    ThrowIfWin32BoolFalse(GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        moduleName.data(), &module
    ));
    return GetModuleFilePath(module);
}

void LoadRemoteLibrary(
    const HANDLE processHandle,
    const std::filesystem::path& libraryPath) {
    const std::span libraryPaths { &libraryPath, 1 };
    LoadRemoteLibrary(processHandle, libraryPaths);
}

} // namespace pwu
