#include "debug.hpp"

#include <cstdio>

#include "disassembler.hpp"

namespace chip8 {

namespace {

template <typename... Args>
void append(std::string& out, const char* format, Args... args) {
    char buf[96];
    std::snprintf(buf, sizeof buf, format, args...);
    out += buf;
}

}  // namespace

std::string format_trace_line(const Chip8& machine) {
    const CpuState& s = machine.cpu().state();
    const Memory& mem = machine.memory();

    std::string out;
    if (s.pc + 1u >= Memory::kSize) {
        append(out, "PC=0x%03X  (fora da memória)", static_cast<unsigned>(s.pc));
        return out;
    }

    const auto opcode = static_cast<std::uint16_t>((mem.read(s.pc) << 8) | mem.read(s.pc + 1));
    append(out, "PC=0x%03X  %04X  %-16s", static_cast<unsigned>(s.pc),
           static_cast<unsigned>(opcode), disassemble(opcode).c_str());
    append(out, " I=0x%03X SP=%u V=", static_cast<unsigned>(s.i), static_cast<unsigned>(s.sp));
    for (std::uint8_t reg : s.v) append(out, "%02X ", static_cast<unsigned>(reg));
    return out;
}

std::string format_registers(const Chip8& machine) {
    const CpuState& s = machine.cpu().state();
    std::string out;

    append(out, "PC = 0x%03X   I = 0x%03X   SP = %u\n", static_cast<unsigned>(s.pc),
           static_cast<unsigned>(s.i), static_cast<unsigned>(s.sp));
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            const int idx = row * 4 + col;
            append(out, "V%X = 0x%02X   ", idx, static_cast<unsigned>(s.v[idx]));
        }
        out += '\n';
    }

    out += "Pilha:";
    if (s.sp == 0) out += " vazia";
    for (std::uint8_t k = 0; k < s.sp; ++k) append(out, " 0x%03X", static_cast<unsigned>(s.stack[k]));
    out += '\n';

    append(out, "Delay = %u   Sound = %u\n", static_cast<unsigned>(machine.timers().delay),
           static_cast<unsigned>(machine.timers().sound));
    return out;
}

std::string hexdump(const Memory& memory, std::uint16_t start, std::uint16_t length) {
    std::string out;
    const unsigned end = static_cast<unsigned>(start) + length;

    for (unsigned addr = start; addr < end && addr < Memory::kSize; addr += 16) {
        append(out, "%03X: ", addr);
        for (unsigned k = 0; k < 16 && addr + k < end && addr + k < Memory::kSize; ++k) {
            append(out, "%02X ", static_cast<unsigned>(memory.read(static_cast<std::uint16_t>(addr + k))));
        }
        out += '\n';
    }
    return out;
}

}  // namespace chip8
