#pragma once

#include "pwu/ErrorHandling.hpp"
#include "pwu/Loader.hpp"
#include "pwu/Resource.hpp"
#include "pwu/ScopeExit.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>

#include <Windows.h>

namespace pwu {
inline std::filesystem::path GetCurrentModuleFilePath() {
    const void* address = reinterpret_cast<void*>(GetCurrentModuleFilePath);
    return GetModuleFilePath(address);
}

template <typename Container>
void LoadRemoteLibrary(
    const HANDLE processHandle,
    const Container& libraryPaths) {
    if (libraryPaths.empty()) {
        return;
    }

    // Adjust privileges
    LUID luid {};
    ThrowIfWin32BoolFalse(LookupPrivilegeValueA(
        nullptr,
        "SeDebugPrivilege", // SE_DEBUG_NAME 20L
        &luid
    ));

    HANDLE rawToken = nullptr;
    ThrowIfWin32BoolFalse(OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &rawToken
    ));
    const UniqueHandle token = MakeUniqueHandle(rawToken);

    TOKEN_PRIVILEGES newPrivileges {
        .PrivilegeCount = 1,
        .Privileges = {{
            .Luid = luid,
            .Attributes = SE_PRIVILEGE_ENABLED
        }}
    };
    TOKEN_PRIVILEGES oldPrivileges {};
    DWORD returnLength = 0;
    ThrowIfWin32BoolFalse(AdjustTokenPrivileges(
        token.Get(),
        FALSE,
        &newPrivileges,
        sizeof(newPrivileges),
        &oldPrivileges,
        &returnLength
    ));
    const ScopeExit privilegeCleanup { [&]() noexcept {
        AdjustTokenPrivileges(
            token.Get(),
            FALSE,
            &oldPrivileges,
            returnLength,
            nullptr,
            nullptr
        );
    } };
    ThrowWin32ErrorIf(
        ERROR_PRIVILEGE_NOT_HELD,
        GetLastError() == ERROR_NOT_ALL_ASSIGNED
    );

    // Get LoadLibraryW
    const HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    ThrowLastWin32ErrorIfNull(kernel32);
    const FARPROC loadLibraryW = GetProcAddress(kernel32, "LoadLibraryW");
    ThrowLastWin32ErrorIfNull(loadLibraryW);

    // Calculate buffer size
    const std::filesystem::path& longestFilePath = *std::ranges::max_element(
        libraryPaths,
        [](const std::filesystem::path& a, const std::filesystem::path& b) {
            return a.native().size() < b.native().size();
        }
    );
    const size_t bufferSize =
        (longestFilePath.native().size() + 1) * sizeof(wchar_t);

    // Allocate buffer
    const LPVOID buffer = VirtualAllocEx(
        processHandle,
        nullptr,
        bufferSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );
    ThrowLastWin32ErrorIfNull(buffer);
    ScopeExit bufferCleanup { [=]() noexcept {
        VirtualFreeEx(
            processHandle,
            buffer,
            0,
            MEM_RELEASE
        );
    } };

    for (const std::filesystem::path& dllPath : libraryPaths) {
        // Write dll path to process
        ThrowIfWin32BoolFalse(WriteProcessMemory(
            processHandle,
            buffer,
            dllPath.c_str(),
            (dllPath.native().size() + 1) * sizeof(wchar_t),
            nullptr
        ));

        // Create thread to load dll
        const UniqueHandle thread = MakeUniqueHandle(CreateRemoteThread(
            processHandle,
            nullptr,
            0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryW),
            buffer,
            0,
            nullptr
        ));
        ThrowLastWin32ErrorIfNull(thread.Get());
        WaitForSingleObject(thread.Get(), INFINITE);
    }
}
} // namespace pwu
