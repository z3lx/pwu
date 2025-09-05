module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

module pwu:ErrorHandling;

import :ErrorHandling;
import :TracedException;

import std;

namespace pwu {
void ThrowWin32Error(
    const DWORD error,
    const std::source_location location) {
    const std::system_error exception {
        static_cast<int>(error),
        std::system_category()
    };
    ThrowTraced(exception, location);
}

void ThrowLastWin32Error(
    const std::source_location location) {
    const DWORD error = GetLastError();
    ThrowWin32Error(error, location);
}

void ThrowIfWin32Error(
    const DWORD error,
    const std::source_location location) {
    if (error != ERROR_SUCCESS) {
        ThrowWin32Error(error, location);
    }
}

void ThrowIfWin32BoolFalse(
    const BOOL result,
    const std::source_location location) {
    if (result == FALSE) {
        ThrowLastWin32Error(location);
    }
}

void ThrowWin32ErrorIf(
    const DWORD error,
    const bool condition,
    const std::source_location location) {
    if (condition) {
        ThrowWin32Error(error, location);
    }
}

void ThrowLastWin32ErrorIf(
    const bool condition,
    const std::source_location location) {
    if (condition) {
        ThrowLastWin32Error(location);
    }
}

void ThrowLastWin32ErrorIfNull(
    const void* pointer,
    const std::source_location location) {
    if (pointer == nullptr) {
        ThrowLastWin32Error(location);
    }
}
} // namespace pwu
