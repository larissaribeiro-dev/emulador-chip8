#include "keypad.hpp"

namespace chip8 {

void Keypad::set_key(std::uint8_t key, bool pressed) {
    if (key >= keys_.size()) return;
    if (keys_[key] && !pressed) released_ = key;
    keys_[key] = pressed;
}

bool Keypad::is_pressed(std::uint8_t key) const {
    return key < keys_.size() && keys_[key];
}

std::optional<std::uint8_t> Keypad::take_release() {
    auto key = released_;
    released_.reset();
    return key;
}

void Keypad::reset() {
    keys_.fill(false);
    released_.reset();
}

}  // namespace chip8
