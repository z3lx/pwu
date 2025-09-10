module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>

module pwu;

import :Dialogue;
import :ErrorHandling;

import std;

namespace {
template <typename StringView>
pwu::MessageBoxResult ShowMessageBox(
    const StringView title,
    const StringView message,
    const pwu::MessageBoxIcon icon,
    const pwu::MessageBoxButton button,
    const pwu::MessageBoxDefaultButton defaultButton) {
    constexpr auto func = [] {
        using CharT = typename StringView::value_type;
        if constexpr (std::is_same_v<CharT, char>) {
            return MessageBoxA;
        } else if constexpr (std::is_same_v<CharT, wchar_t>) {
            return MessageBoxW;
        }
    }();
    int result = func(
        nullptr,
        message.data(),
        title.data(),
        static_cast<UINT>(icon) |
        static_cast<UINT>(button) |
        static_cast<UINT>(defaultButton)
    );
    pwu::ThrowLastWin32ErrorIf(result == 0);
    return static_cast<pwu::MessageBoxResult>(result);
}
} // namespace

namespace pwu {
MessageBoxResult ShowMessageBox(
    const std::string_view title,
    const std::string_view message,
    const MessageBoxIcon icon,
    const MessageBoxButton button,
    const MessageBoxDefaultButton defaultButton) {
    return ::ShowMessageBox(
        title,
        message,
        icon,
        button,
        defaultButton
    );
}

MessageBoxResult ShowMessageBox(
    const std::wstring_view title,
    const std::wstring_view message,
    const MessageBoxIcon icon,
    const MessageBoxButton button,
    const MessageBoxDefaultButton defaultButton) {
    return ::ShowMessageBox(
        title,
        message,
        icon,
        button,
        defaultButton
    );
}

std::filesystem::path OpenFileDialogue(
    const std::optional<std::span<const Filter>> filters,
    const std::optional<std::wstring_view> initialPath,
    const std::optional<std::wstring_view> dialogueTitle) {
    std::wstring stringFilter {};
    if (filters) {
        size_t size = 0;
        for (const auto& [name, spec] : *filters) {
            size += name.size() + spec.size() + 2;
        }
        stringFilter.reserve(size);
        for (const auto& [name, spec] : *filters) {
            stringFilter.append(name).push_back('\0');
            stringFilter.append(spec).push_back('\0');
        }
    }

    std::array<wchar_t, MAX_PATH> buffer {};
    OPENFILENAMEW ofn {
        .lStructSize = sizeof(ofn),
        .lpstrFilter = filters ? stringFilter.data() : nullptr,
        .nFilterIndex = filters ? 1UL : 0UL,
        .lpstrFile = buffer.data(),
        .nMaxFile = static_cast<DWORD>(buffer.size()),
        .lpstrInitialDir = initialPath ? initialPath->data() : nullptr,
        .lpstrTitle = dialogueTitle ? dialogueTitle->data() : nullptr,
        .Flags =
            OFN_PATHMUSTEXIST |
            OFN_FILEMUSTEXIST |
            OFN_EXPLORER |
            OFN_NOCHANGEDIR
    };
    ThrowIfWin32BoolFalse(GetOpenFileNameW(&ofn));
    return std::filesystem::path { buffer.data() };
}
} // namespace pwu
