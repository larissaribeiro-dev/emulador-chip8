#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "display.hpp"
#include "keypad.hpp"
#include "memory.hpp"
#include "quirks.hpp"
#include "timers.hpp"

namespace chip8 {

// Registradores e pilha da CPU. Exposto para depuração e testes.
struct CpuState {
    std::array<std::uint8_t, 16> v{};
    std::uint16_t i = 0;
    std::uint16_t pc = Memory::kProgramStart;
    std::uint8_t sp = 0;
    std::array<std::uint16_t, 16> stack{};
};

class Cpu {
public:
    using RandomSource = std::function<std::uint8_t()>;

    Cpu(Memory& memory, Display& display, Keypad& keypad, Timers& timers, Quirks quirks);

    // Volta aos valores iniciais, com PC em 0x200.
    void reset();

    // Executa uma instrução completa (busca, decodificação e execução) e
    // devolve o opcode executado. Lança Chip8Error em caso de falha.
    std::uint16_t step();

    // Permite trocar o gerador de CXNN, o que torna os testes determinísticos.
    void set_random_source(RandomSource source) { random_ = std::move(source); }

    CpuState& state() { return state_; }
    const CpuState& state() const { return state_; }
    const Quirks& quirks() const { return quirks_; }

private:
    std::uint16_t fetch() const;
    void execute(std::uint16_t opcode, std::uint16_t opcode_pc);
    void execute_arithmetic(std::uint16_t opcode, std::uint16_t opcode_pc);
    void execute_misc(std::uint16_t opcode, std::uint16_t opcode_pc);
    void wait_for_key(std::uint8_t x);

    Memory& memory_;
    Display& display_;
    Keypad& keypad_;
    Timers& timers_;
    Quirks quirks_;
    CpuState state_;
    RandomSource random_;
    bool waiting_for_key_ = false;
};

}  // namespace chip8
