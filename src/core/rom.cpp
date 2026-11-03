#include "rom.hpp"

#include <fstream>
#include <iterator>

#include "errors.hpp"
#include "memory.hpp"

namespace chip8 {

std::vector<std::uint8_t> read_rom_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw RomError("não foi possível abrir a ROM '" + path + "'");

    std::vector<std::uint8_t> rom((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());

    if (rom.empty()) throw RomError("a ROM '" + path + "' está vazia");
    if (rom.size() > Memory::kMaxRomSize) {
        throw RomError("a ROM '" + path + "' tem " + std::to_string(rom.size()) +
                       " bytes e o máximo é " + std::to_string(Memory::kMaxRomSize));
    }
    return rom;
}

}  // namespace chip8
