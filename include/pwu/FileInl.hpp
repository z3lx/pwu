#pragma once

#include "pwu/ErrorHandling.hpp"
#include "pwu/File.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

#include <Windows.h>

namespace pwu {
namespace detail {
constexpr size_t maxBufferChunkSize = (std::numeric_limits<DWORD>::max)();
} // namespace detail

template <typename ContiguousContainer>
ContiguousContainer ReadFile(const HANDLE fileHandle) {
    ContiguousContainer buffer {};
    ReadFile(fileHandle, buffer);
    return buffer;
}

template <typename ContiguousContainer>
void ReadFile(const HANDLE fileHandle, ContiguousContainer& buffer) {
    ThrowIfWin32BoolFalse(SetFilePointerEx(
        fileHandle, {}, nullptr, FILE_BEGIN
    ));

    // Calculate buffer size
    LARGE_INTEGER fileSize {};
    ThrowIfWin32BoolFalse(GetFileSizeEx(fileHandle, &fileSize));
    const auto bufferSize = static_cast<size_t>(fileSize.QuadPart);
    using BufferElementT = typename ContiguousContainer::value_type;
    const size_t bufferElementCount =
        (bufferSize + sizeof(BufferElementT) - 1) / sizeof(BufferElementT);
    buffer.resize(bufferElementCount);

    // Read file to buffer
    for (size_t bufferSizeOffset = 0; bufferSizeOffset < bufferSize;) {
        void* bufferChunkAddress =
            reinterpret_cast<uint8_t*>(buffer.data()) + bufferSizeOffset;
        const DWORD bufferChunkSize = static_cast<DWORD>((std::min)(
            detail::maxBufferChunkSize,
            bufferSize - bufferSizeOffset
        ));
        DWORD bytesRead = 0;
        ThrowIfWin32BoolFalse(::ReadFile(
            fileHandle,
            bufferChunkAddress,
            bufferChunkSize,
            &bytesRead,
            nullptr
        ));
        bufferSizeOffset += bytesRead;
    }
}

template <typename ContiguousContainer>
void WriteFile(const HANDLE fileHandle, const ContiguousContainer& buffer) {
    ThrowIfWin32BoolFalse(SetFilePointerEx(
        fileHandle, {}, nullptr, FILE_BEGIN
    ));
    ThrowIfWin32BoolFalse(SetEndOfFile(fileHandle));
    AppendFile(fileHandle, buffer);
}

template <typename ContiguousContainer>
void AppendFile(const HANDLE fileHandle, const ContiguousContainer& buffer) {
    using BufferElementT = typename ContiguousContainer::value_type;
    const size_t bufferSize = buffer.size() * sizeof(BufferElementT);

    for (size_t bufferSizeOffset = 0; bufferSizeOffset < bufferSize;) {
        const void* bufferChunkAddress =
            reinterpret_cast<const uint8_t*>(buffer.data()) + bufferSizeOffset;
        const DWORD bufferChunkSize = static_cast<DWORD>((std::min)(
            detail::maxBufferChunkSize,
            bufferSize - bufferSizeOffset
        ));
        DWORD bytesTransferred = 0;
        ThrowIfWin32BoolFalse(::WriteFile(
            fileHandle,
            bufferChunkAddress,
            bufferChunkSize,
            &bytesTransferred,
            nullptr
        ));
        bufferSizeOffset += bytesTransferred;
    }
}
} // namespace pwu
