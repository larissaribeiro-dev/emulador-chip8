#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace chip8 {

// Tela monocromática de 64x32. Cada posição guarda 0 (apagado) ou 1 (aceso).
class Display {
public:
    static constexpr int kWidth = 64;
    static constexpr int kHeight = 32;

    void clear();

    // Desenha um sprite de `rows` linhas (8 pixels de largura) com XOR.
    // A coordenada inicial dá a volta na tela; o restante do sprite é cortado
    // nas bordas. Retorna true se algum pixel aceso foi apagado (colisão).
    bool draw_sprite(int x, int y, const std::uint8_t* rows, std::size_t count);

    bool pixel(int x, int y) const;
    const std::array<std::uint8_t, kWidth * kHeight>& pixels() const { return pixels_; }

    // true se a tela mudou desde a última chamada a clear_dirty().
    bool dirty() const { return dirty_; }
    void clear_dirty() { dirty_ = false; }

    // Uma linha de texto por linha da tela, '#' para aceso e '.' para apagado.
    std::string to_ascii() const;

private:
    std::array<std::uint8_t, kWidth * kHeight> pixels_{};
    bool dirty_ = true;
};

}  // namespace chip8
