#include "disassembler.hpp"

#include <cstdio>

namespace chip8 {

namespace {

std::string fmt(const char* format, unsigned a = 0, unsigned b = 0, unsigned c = 0) {
    char buf[48];
    std::snprintf(buf, sizeof buf, format, a, b, c);
    return buf;
}

}  // namespace

std::string disassemble(std::uint16_t opcode) {
    const unsigned x = (opcode >> 8) & 0xF;
    const unsigned y = (opcode >> 4) & 0xF;
    const unsigned n = opcode & 0xF;
    const unsigned nn = opcode & 0xFF;
    const unsigned nnn = opcode & 0xFFF;

    switch (opcode >> 12) {
        case 0x0:
            if (opcode == 0x00E0) return "CLS";
            if (opcode == 0x00EE) return "RET";
            break;
        case 0x1: return fmt("JP 0x%03X", nnn);
        case 0x2: return fmt("CALL 0x%03X", nnn);
        case 0x3: return fmt("SE V%X, 0x%02X", x, nn);
        case 0x4: return fmt("SNE V%X, 0x%02X", x, nn);
        case 0x5:
            if (n == 0) return fmt("SE V%X, V%X", x, y);
            break;
        case 0x6: return fmt("LD V%X, 0x%02X", x, nn);
        case 0x7: return fmt("ADD V%X, 0x%02X", x, nn);
        case 0x8:
            switch (n) {
                case 0x0: return fmt("LD V%X, V%X", x, y);
                case 0x1: return fmt("OR V%X, V%X", x, y);
                case 0x2: return fmt("AND V%X, V%X", x, y);
                case 0x3: return fmt("XOR V%X, V%X", x, y);
                case 0x4: return fmt("ADD V%X, V%X", x, y);
                case 0x5: return fmt("SUB V%X, V%X", x, y);
                case 0x6: return fmt("SHR V%X, V%X", x, y);
                case 0x7: return fmt("SUBN V%X, V%X", x, y);
                case 0xE: return fmt("SHL V%X, V%X", x, y);
                default: break;
            }
            break;
        case 0x9:
            if (n == 0) return fmt("SNE V%X, V%X", x, y);
            break;
        case 0xA: return fmt("LD I, 0x%03X", nnn);
        case 0xB: return fmt("JP V0, 0x%03X", nnn);
        case 0xC: return fmt("RND V%X, 0x%02X", x, nn);
        case 0xD: return fmt("DRW V%X, V%X, %u", x, y, n);
        case 0xE:
            if (nn == 0x9E) return fmt("SKP V%X", x);
            if (nn == 0xA1) return fmt("SKNP V%X", x);
            break;
        case 0xF:
            switch (nn) {
                case 0x07: return fmt("LD V%X, DT", x);
                case 0x0A: return fmt("LD V%X, K", x);
                case 0x15: return fmt("LD DT, V%X", x);
                case 0x18: return fmt("LD ST, V%X", x);
                case 0x1E: return fmt("ADD I, V%X", x);
                case 0x29: return fmt("LD F, V%X", x);
                case 0x33: return fmt("LD B, V%X", x);
                case 0x55: return fmt("LD [I], V%X", x);
                case 0x65: return fmt("LD V%X, [I]", x);
                default: break;
            }
            break;
    }
    return fmt("DW 0x%04X", opcode);
}

}  // namespace chip8
