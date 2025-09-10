module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#undef ShellExecute

module pwu;

import :Shell;
import :ErrorHandling;

import std;

namespace {
template <typename StringView>
void ShellExecute(const StringView operation, const StringView file) {
    constexpr auto func = [] {
        using CharT = typename StringView::value_type;
        if constexpr (std::is_same_v<CharT, char>) {
            return ShellExecuteA;
        } else if constexpr (std::is_same_v<CharT, wchar_t>) {
            return ShellExecuteW;
        }
    }();
    const auto code = reinterpret_cast<INT_PTR>(func(
        nullptr,
        operation.data(),
        file.data(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    ));
    pwu::ThrowLastWin32ErrorIf(code <= 32);
}
} // namespace

namespace pwu {
void OpenUrl(const std::string_view url) {
    constexpr std::string_view operation { "open" };
    ShellExecute(operation, url);
}

void OpenUrl(const std::wstring_view url) {
    constexpr std::wstring_view operation { L"open" };
    ShellExecute(operation, url);
}
} // namespace pwu
