#pragma once

#include <cstdint>
#include <filesystem>

namespace pwu {
struct Version {
    uint16_t major;
    uint16_t minor;
    uint16_t build;
    uint16_t revision;
};

Version GetFileVersion(const std::filesystem::path& filePath);
} // namespace pwu
