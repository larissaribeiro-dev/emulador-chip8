#include "display.hpp"

namespace chip8 {

void Display::clear() {
    pixels_.fill(0);
    dirty_ = true;
}

bool Display::draw_sprite(int x, int y, const std::uint8_t* rows, std::size_t count) {
    const int x0 = x % kWidth;
    const int y0 = y % kHeight;
    bool collision = false;

    for (std::size_t row = 0; row < count; ++row) {
        const int py = y0 + static_cast<int>(row);
        if (py >= kHeight) break;

        for (int bit = 0; bit < 8; ++bit) {
            const int px = x0 + bit;
            if (px >= kWidth) break;
            if ((rows[row] & (0x80 >> bit)) == 0) continue;

            std::uint8_t& cell = pixels_[static_cast<std::size_t>(py * kWidth + px)];
            if (cell) collision = true;
            cell ^= 1;
        }
    }

    dirty_ = true;
    return collision;
}

bool Display::pixel(int x, int y) const {
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) return false;
    return pixels_[static_cast<std::size_t>(y * kWidth + x)] != 0;
}

std::string Display::to_ascii() const {
    std::string out;
    out.reserve((kWidth + 1) * kHeight);
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) out += pixel(x, y) ? '#' : '.';
        out += '\n';
    }
    return out;
}

}  // namespace chip8
