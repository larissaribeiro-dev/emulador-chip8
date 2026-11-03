#include "errors.hpp"

#include <cstdio>
#include <string>

namespace chip8 {

namespace {

std::string format_unknown(std::uint16_t opcode, std::uint16_t pc) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "instrução desconhecida 0x%04X em PC=0x%03X",
                  static_cast<unsigned>(opcode), static_cast<unsigned>(pc));
    return buf;
}

}  // namespace

UnknownOpcodeError::UnknownOpcodeError(std::uint16_t opcode, std::uint16_t pc)
    : Chip8Error(format_unknown(opcode, pc)), opcode_(opcode), pc_(pc) {}

}  // namespace chip8
