#pragma once

#include "pwu/UniqueResource.hpp"

#include <Windows.h>

namespace pwu {
namespace detail {
const auto handleDeleter = [](const HANDLE handle) noexcept {
    CloseHandle(handle);
};
} // namespace detail

using UniqueHandle = UniqueResource<HANDLE, decltype(detail::handleDeleter)>;

inline UniqueHandle MakeUniqueHandle(const HANDLE handle) noexcept {
    return UniqueHandle { handle, detail::handleDeleter };
}
} // namespace pwu
