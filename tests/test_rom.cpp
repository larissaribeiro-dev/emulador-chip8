#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "errors.hpp"
#include "memory.hpp"
#include "rom.hpp"
#include "test_helpers.hpp"

namespace fs = std::filesystem;

namespace {

// Arquivo temporário que se apaga sozinho.
class TempFile {
public:
    explicit TempFile(const std::vector<std::uint8_t>& content) {
        static int counter = 0;
        path_ = (fs::temp_directory_path() / ("chip8_test_rom_" + std::to_string(counter++) + ".ch8")).string();
        std::ofstream out(path_, std::ios::binary);
        out.write(reinterpret_cast<const char*>(content.data()), static_cast<std::streamsize>(content.size()));
    }
    ~TempFile() { std::remove(path_.c_str()); }
    const std::string& path() const { return path_; }

private:
    std::string path_;
};

}  // namespace

TEST(Rom, ReadsFileContentsByteForByte) {
    const std::vector<std::uint8_t> bytes = {0x00, 0xE0, 0xA2, 0x2A, 0xFF};
    TempFile file(bytes);
    EXPECT_EQ(chip8::read_rom_file(file.path()), bytes);
}

TEST(Rom, MissingFileThrows) {
    EXPECT_THROW(chip8::read_rom_file("nao_existe.ch8"), chip8::RomError);
}

TEST(Rom, EmptyFileThrows) {
    TempFile file({});
    EXPECT_THROW(chip8::read_rom_file(file.path()), chip8::RomError);
}

TEST(Rom, FileLargerThanAvailableMemoryThrows) {
    TempFile file(std::vector<std::uint8_t>(chip8::Memory::kMaxRomSize + 1, 0x11));
    EXPECT_THROW(chip8::read_rom_file(file.path()), chip8::RomError);
}

TEST(Rom, FileOfMaximumSizeIsAccepted) {
    TempFile file(std::vector<std::uint8_t>(chip8::Memory::kMaxRomSize, 0x11));
    EXPECT_EQ(chip8::read_rom_file(file.path()).size(), chip8::Memory::kMaxRomSize);
}
