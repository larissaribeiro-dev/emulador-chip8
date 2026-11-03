#include "memory.hpp"

#include <algorithm>
#include <cstdio>
#include <string>

#include "errors.hpp"

namespace chip8 {

namespace {

constexpr std::array<std::uint8_t, 80> kFont = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
};

std::string hex_address(std::uint16_t address) {
    char buf[16];
    std::snprintf(buf, sizeof buf, "0x%04X", static_cast<unsigned>(address));
    return buf;
}

}  // namespace

Memory::Memory() { reset(); }

void Memory::reset() {
    data_.fill(0);
    std::copy(kFont.begin(), kFont.end(), data_.begin() + kFontStart);
}

std::uint8_t Memory::read(std::uint16_t address) const {
    if (address >= kSize) {
        throw MemoryAccessError("leitura fora da memória em " + hex_address(address));
    }
    return data_[address];
}

void Memory::write(std::uint16_t address, std::uint8_t value) {
    if (address >= kSize) {
        throw MemoryAccessError("escrita fora da memória em " + hex_address(address));
    }
    data_[address] = value;
}

void Memory::load_program(const std::vector<std::uint8_t>& rom) {
    if (rom.size() > kMaxRomSize) {
        throw MemoryAccessError("programa de " + std::to_string(rom.size()) +
                                " bytes não cabe em " + std::to_string(kMaxRomSize));
    }
    std::copy(rom.begin(), rom.end(), data_.begin() + kProgramStart);
}

std::uint16_t Memory::glyph_address(std::uint8_t digit) {
    return static_cast<std::uint16_t>(kFontStart + (digit & 0x0F) * kFontGlyphSize);
}

}  // namespace chip8
