#include "platform.hpp"

#include <array>
#include <stdexcept>

namespace frontend {

namespace {

constexpr std::uint32_t kPixelOn = 0xFFE8E8E8;
constexpr std::uint32_t kPixelOff = 0xFF101010;
constexpr int kSampleRate = 44100;
constexpr double kToneHz = 440.0;

// Layout 4x4 do teclado original mapeado nas teclas 1-4 / Q-R / A-F / Z-V.
//   1 2 3 C        1 2 3 4
//   4 5 6 D   ->   Q W E R
//   7 8 9 E        A S D F
//   A 0 B F        Z X C V
struct KeyBinding {
    SDL_Scancode scancode;
    std::uint8_t key;
};

constexpr std::array<KeyBinding, 16> kBindings = {{
    {SDL_SCANCODE_1, 0x1}, {SDL_SCANCODE_2, 0x2}, {SDL_SCANCODE_3, 0x3}, {SDL_SCANCODE_4, 0xC},
    {SDL_SCANCODE_Q, 0x4}, {SDL_SCANCODE_W, 0x5}, {SDL_SCANCODE_E, 0x6}, {SDL_SCANCODE_R, 0xD},
    {SDL_SCANCODE_A, 0x7}, {SDL_SCANCODE_S, 0x8}, {SDL_SCANCODE_D, 0x9}, {SDL_SCANCODE_F, 0xE},
    {SDL_SCANCODE_Z, 0xA}, {SDL_SCANCODE_X, 0x0}, {SDL_SCANCODE_C, 0xB}, {SDL_SCANCODE_V, 0xF},
}};

}  // namespace

Platform::Platform(const std::string& title, int scale) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        throw std::runtime_error(std::string("falha ao iniciar o SDL: ") + SDL_GetError());
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               chip8::Display::kWidth * scale, chip8::Display::kHeight * scale,
                               SDL_WINDOW_RESIZABLE);
    if (!window_) {
        std::string msg = std::string("falha ao criar a janela: ") + SDL_GetError();
        SDL_Quit();
        throw std::runtime_error(msg);
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) {
        std::string msg = std::string("falha ao criar o renderer: ") + SDL_GetError();
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error(msg);
    }
    SDL_RenderSetLogicalSize(renderer_, chip8::Display::kWidth, chip8::Display::kHeight);
    SDL_RenderSetIntegerScale(renderer_, SDL_TRUE);

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                 chip8::Display::kWidth, chip8::Display::kHeight);
    if (!texture_) {
        std::string msg = std::string("falha ao criar a textura: ") + SDL_GetError();
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error(msg);
    }

    SDL_AudioSpec want{};
    want.freq = kSampleRate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = &Platform::audio_callback;
    want.userdata = this;
    SDL_AudioSpec have{};
    audio_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audio_ == 0) SDL_Log("áudio indisponível, seguindo sem som: %s", SDL_GetError());
}

Platform::~Platform() {
    if (audio_ != 0) SDL_CloseAudioDevice(audio_);
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool Platform::poll_events(chip8::Keypad& keypad) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                const bool pressed = event.type == SDL_KEYDOWN;
                if (pressed && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) return false;
                for (const auto& binding : kBindings) {
                    if (binding.scancode == event.key.keysym.scancode) {
                        keypad.set_key(binding.key, pressed);
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    return true;
}

void Platform::render(const chip8::Display& display) {
    std::array<std::uint32_t, chip8::Display::kWidth * chip8::Display::kHeight> frame{};
    const auto& pixels = display.pixels();
    for (std::size_t i = 0; i < frame.size(); ++i) frame[i] = pixels[i] ? kPixelOn : kPixelOff;

    SDL_UpdateTexture(texture_, nullptr, frame.data(), chip8::Display::kWidth * static_cast<int>(sizeof(std::uint32_t)));
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

void Platform::set_beep(bool on) {
    if (audio_ == 0 || on == beeping_) return;
    beeping_ = on;
    SDL_PauseAudioDevice(audio_, on ? 0 : 1);
}

void Platform::audio_callback(void* userdata, std::uint8_t* stream, int length) {
    auto* self = static_cast<Platform*>(userdata);
    auto* samples = reinterpret_cast<std::int16_t*>(stream);
    const int count = length / static_cast<int>(sizeof(std::int16_t));
    const double step = kToneHz / kSampleRate;

    for (int i = 0; i < count; ++i) {
        samples[i] = self->phase_ < 0.5 ? 3000 : -3000;
        self->phase_ += step;
        if (self->phase_ >= 1.0) self->phase_ -= 1.0;
    }
}

}  // namespace frontend
