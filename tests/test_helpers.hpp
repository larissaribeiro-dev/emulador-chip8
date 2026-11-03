#pragma once

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "chip8.hpp"

namespace testutil {

// Converte uma lista de opcodes em bytes big-endian, prontos para load_rom().
inline std::vector<std::uint8_t> assemble(std::initializer_list<std::uint16_t> opcodes) {
    std::vector<std::uint8_t> bytes;
    for (std::uint16_t op : opcodes) {
        bytes.push_back(static_cast<std::uint8_t>(op >> 8));
        bytes.push_back(static_cast<std::uint8_t>(op & 0xFF));
    }
    return bytes;
}

inline void run(chip8::Chip8& machine, int steps) {
    for (int i = 0; i < steps; ++i) machine.step();
}

// Carrega o programa e executa `steps` instruções.
inline void load_and_run(chip8::Chip8& machine, std::initializer_list<std::uint16_t> opcodes, int steps) {
    machine.load_rom(assemble(opcodes));
    run(machine, steps);
}

inline std::uint8_t& V(chip8::Chip8& machine, std::size_t index) {
    return machine.cpu().state().v[index];
}

}  // namespace testutil
