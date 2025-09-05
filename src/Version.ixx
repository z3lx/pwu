export module pwu:Version;

import std;

export namespace pwu {
struct Version {
    std::uint16_t major;
    std::uint16_t minor;
    std::uint16_t build;
    std::uint16_t revision;
};

Version GetFileVersion(const std::filesystem::path& filePath);
} // namespace pwu
