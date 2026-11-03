#include "chip8.hpp"

namespace chip8 {

Chip8::Chip8(Quirks quirks) : cpu_(memory_, display_, keypad_, timers_, quirks) {}

void Chip8::load_rom(const std::vector<std::uint8_t>& rom) {
    memory_.reset();
    memory_.load_program(rom);
    display_.clear();
    keypad_.reset();
    timers_.reset();
    cpu_.reset();
}

void Chip8::reset() {
    display_.clear();
    keypad_.reset();
    timers_.reset();
    cpu_.reset();
}

}  // namespace chip8
