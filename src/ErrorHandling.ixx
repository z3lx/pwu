module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define CURRENT_LOCATION \
    std::source_location location = std::source_location::current()

export module pwu:ErrorHandling;

import std;

export namespace pwu {
void ThrowWin32Error(DWORD error, CURRENT_LOCATION);
void ThrowLastWin32Error(CURRENT_LOCATION);

void ThrowIfWin32Error(DWORD error, CURRENT_LOCATION);
void ThrowIfWin32BoolFalse(BOOL result, CURRENT_LOCATION);
void ThrowWin32ErrorIf(DWORD error, bool condition, CURRENT_LOCATION);
void ThrowLastWin32ErrorIf(bool condition, CURRENT_LOCATION);
void ThrowLastWin32ErrorIfNull(const void* pointer, CURRENT_LOCATION);
} // namespace pwu
