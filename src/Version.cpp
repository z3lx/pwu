module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

module pwu:Version;

import :Version;
import :ErrorHandling;

import std;

namespace pwu {
Version GetFileVersion(const std::filesystem::path& filePath) {
    const DWORD infoSize = GetFileVersionInfoSizeW(
        filePath.c_str(),
        nullptr
    );
    ThrowLastWin32ErrorIf(infoSize == 0);

    std::vector<std::uint8_t> infoBuffer(infoSize, 0);
    ThrowIfWin32BoolFalse(GetFileVersionInfoW(
        filePath.c_str(),
        0,
        infoBuffer.size(),
        infoBuffer.data()
    ));

    PVOID queryBuffer = nullptr;
    UINT queryBufferSize = 0;
    ThrowIfWin32BoolFalse(VerQueryValueW(
        infoBuffer.data(),
        L"\\",
        &queryBuffer,
        &queryBufferSize
    ));

    const auto& info = *static_cast<VS_FIXEDFILEINFO*>(queryBuffer);
    ThrowWin32ErrorIf(ERROR_INVALID_DATA, info.dwSignature != 0xFEEF04BD);

    return Version {
        HIWORD(info.dwFileVersionMS),
        LOWORD(info.dwFileVersionMS),
        HIWORD(info.dwFileVersionLS),
        LOWORD(info.dwFileVersionLS)
    };
}
} // namespace pwu
