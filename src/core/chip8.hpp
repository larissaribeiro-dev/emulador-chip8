#pragma once

#include <cstdint>
#include <vector>

#include "cpu.hpp"
#include "display.hpp"
#include "keypad.hpp"
#include "memory.hpp"
#include "quirks.hpp"
#include "timers.hpp"

namespace chip8 {

// A máquina completa: CPU ligada à memória, ao vídeo, ao teclado e aos
// temporizadores. Não depende de SDL, então roda em testes e sem janela.
class Chip8 {
public:
    explicit Chip8(Quirks quirks = Quirks::modern());

    Chip8(const Chip8&) = delete;
    Chip8& operator=(const Chip8&) = delete;

    // Reinicia a máquina e carrega o programa em 0x200.
    void load_rom(const std::vector<std::uint8_t>& rom);

    // Reinicia sem apagar a ROM carregada na memória.
    void reset();

    // Executa uma instrução e devolve o opcode.
    std::uint16_t step() { return cpu_.step(); }

    // Avança os temporizadores um passo de 60 Hz.
    void tick_timers() { timers_.tick(); }

    Cpu& cpu() { return cpu_; }
    const Cpu& cpu() const { return cpu_; }
    Memory& memory() { return memory_; }
    const Memory& memory() const { return memory_; }
    Display& display() { return display_; }
    const Display& display() const { return display_; }
    Keypad& keypad() { return keypad_; }
    const Keypad& keypad() const { return keypad_; }
    Timers& timers() { return timers_; }
    const Timers& timers() const { return timers_; }

private:
    Memory memory_;
    Display display_;
    Keypad keypad_;
    Timers timers_;
    Cpu cpu_;
};

}  // namespace chip8
