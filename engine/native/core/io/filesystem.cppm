module;

#include <vector>
#include <string>
#include <fstream>
#include <cstdint>

export module core.io.filesystem;

export namespace draco::core::io::filesystem
{
    // Returns a buffer of the file data
    std::vector<uint8_t> load_binary(const std::string& path);
}
