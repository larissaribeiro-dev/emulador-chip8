#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chip8 {

// Lê um arquivo ROM inteiro. Lança RomError se o arquivo não abrir,
// estiver vazio ou for maior que a memória disponível a partir de 0x200.
std::vector<std::uint8_t> read_rom_file(const std::string& path);

}  // namespace chip8
