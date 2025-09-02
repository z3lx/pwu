#pragma once

#include "pwu/TracedException.hpp"

#include <source_location>

#include <Windows.h>

#define CURRENT_LOCATION \
    std::source_location location = std::source_location::current()

namespace pwu {
void ThrowWin32Error(DWORD error, CURRENT_LOCATION);
void ThrowLastWin32Error(CURRENT_LOCATION);

void ThrowIfWin32Error(DWORD error, CURRENT_LOCATION);
void ThrowIfWin32BoolFalse(BOOL result, CURRENT_LOCATION);
void ThrowWin32ErrorIf(DWORD error, bool condition, CURRENT_LOCATION);
void ThrowLastWin32ErrorIf(bool condition, CURRENT_LOCATION);
void ThrowLastWin32ErrorIfNull(const void* pointer, CURRENT_LOCATION);
} // namespace pwu
