#include "pwu/Console.hpp"

#include <iostream>

#include <Windows.h>
#include <conio.h>

namespace pwu {
void Pause() noexcept {
    std::cout << "Press any key to continue..." << std::endl;
    _getch();
}
} // namespace pwu
