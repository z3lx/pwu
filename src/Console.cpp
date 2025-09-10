module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <conio.h>

module pwu;

import :Console;

import std;

namespace pwu {
void Pause() noexcept {
    std::cout << "Press any key to continue..." << std::endl;
    _getch();
}
} // namespace pwu
