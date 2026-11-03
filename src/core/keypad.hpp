#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace chip8 {

// Teclado hexadecimal de 16 teclas (0x0 a 0xF).
class Keypad {
public:
    // Teclas fora de 0x0-0xF são ignoradas.
    void set_key(std::uint8_t key, bool pressed);
    bool is_pressed(std::uint8_t key) const;

    // A instrução FX0A espera uma tecla ser pressionada e solta.
    // Cada soltura é registrada aqui até alguém consumi-la.
    std::optional<std::uint8_t> take_release();
    void discard_release() { released_.reset(); }

    void reset();

private:
    std::array<bool, 16> keys_{};
    std::optional<std::uint8_t> released_;
};

}  // namespace chip8
