#pragma once

#include <SDL.h>

#include <cstdint>
#include <string>

#include "core/display.hpp"
#include "core/keypad.hpp"

namespace frontend {

// Janela, teclado e som via SDL2. Toda a parte dependente de SDL fica aqui.
class Platform {
public:
    // Lança std::runtime_error se a janela não puder ser criada.
    // Se o áudio falhar o emulador segue em silêncio.
    Platform(const std::string& title, int scale);
    ~Platform();

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;

    // Processa eventos e atualiza o teclado. Retorna false quando o usuário
    // fecha a janela ou aperta Esc.
    bool poll_events(chip8::Keypad& keypad);

    void render(const chip8::Display& display);
    void set_beep(bool on);

private:
    static void audio_callback(void* userdata, std::uint8_t* stream, int length);

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    SDL_AudioDeviceID audio_ = 0;
    bool beeping_ = false;
    double phase_ = 0.0;
};

}  // namespace frontend
