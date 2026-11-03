#pragma once

#include <cstdint>

namespace chip8 {

// Os dois temporizadores decrescem a 60 Hz, independentes da velocidade da CPU.
struct Timers {
    std::uint8_t delay = 0;
    std::uint8_t sound = 0;

    // Chamado uma vez a cada 1/60 s.
    void tick() {
        if (delay > 0) --delay;
        if (sound > 0) --sound;
    }

    bool beeping() const { return sound > 0; }
    void reset() { delay = 0; sound = 0; }
};

}  // namespace chip8
