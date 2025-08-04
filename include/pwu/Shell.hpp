#pragma once

#include <string_view>

namespace pwu {
void OpenUrl(std::string_view url);
void OpenUrl(std::wstring_view url);
} // namespace pwu
