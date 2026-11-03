#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace chip8 {

// 4 KiB de RAM. A fonte hexadecimal fica em 0x050 e os programas começam em 0x200.
class Memory {
public:
    static constexpr std::size_t kSize = 4096;
    static constexpr std::uint16_t kFontStart = 0x050;
    static constexpr std::uint16_t kFontGlyphSize = 5;
    static constexpr std::uint16_t kProgramStart = 0x200;
    static constexpr std::size_t kMaxRomSize = kSize - kProgramStart;

    Memory();

    // Zera a RAM e recarrega a fonte.
    void reset();

    // Lança MemoryAccessError se o endereço estiver fora de 0x000-0xFFF.
    std::uint8_t read(std::uint16_t address) const;
    void write(std::uint16_t address, std::uint8_t value);

    // Copia a ROM para 0x200. Lança MemoryAccessError se ela não couber.
    void load_program(const std::vector<std::uint8_t>& rom);

    // Endereço do glifo do dígito hexadecimal (0x0 a 0xF).
    static std::uint16_t glyph_address(std::uint8_t digit);

    const std::array<std::uint8_t, kSize>& bytes() const { return data_; }

private:
    std::array<std::uint8_t, kSize> data_{};
};

}  // namespace chip8
