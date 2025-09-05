module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module pwu:Resource;

import :UniqueResource;

const auto handleDeleter = [](const HANDLE handle) noexcept {
    CloseHandle(handle);
};

export namespace pwu {
using UniqueHandle = UniqueResource<HANDLE, decltype(handleDeleter)>;
inline UniqueHandle MakeUniqueHandle(const HANDLE handle) noexcept {
    return UniqueHandle { handle, handleDeleter };
}
} // namespace pwu
