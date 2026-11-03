#pragma once

#include <cstdint>
#include <string>

namespace chip8 {

// Traduz um opcode para o mnemônico (ex.: 0x6A05 -> "LD VA, 0x05").
// Opcodes que a CPU não executa viram "DW 0xNNNN".
std::string disassemble(std::uint16_t opcode);

}  // namespace chip8
