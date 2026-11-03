#include "errors.hpp"
#include "memory.hpp"
#include "test_helpers.hpp"

using chip8::Memory;
using chip8::MemoryAccessError;

TEST(Memory, FontIsLoadedAtFontStart) {
    Memory mem;
    EXPECT_EQ(mem.read(Memory::kFontStart), 0xF0);            // primeira linha do glifo 0
    EXPECT_EQ(mem.read(Memory::kFontStart + 79), 0x80);       // última linha do glifo F
}

TEST(Memory, GlyphAddressUsesFiveBytesPerDigit) {
    EXPECT_EQ(Memory::glyph_address(0x0), 0x050);
    EXPECT_EQ(Memory::glyph_address(0x1), 0x055);
    EXPECT_EQ(Memory::glyph_address(0xF), 0x050 + 15 * 5);
    EXPECT_EQ(Memory::glyph_address(0x1A), Memory::glyph_address(0xA));  // só o nibble baixo conta
}

TEST(Memory, WriteThenReadReturnsSameValue) {
    Memory mem;
    mem.write(0x300, 0xAB);
    EXPECT_EQ(mem.read(0x300), 0xAB);
}

TEST(Memory, AccessOutsideRangeThrows) {
    Memory mem;
    EXPECT_THROW(mem.read(Memory::kSize), MemoryAccessError);
    EXPECT_THROW(mem.write(Memory::kSize, 1), MemoryAccessError);
    EXPECT_NO_THROW(mem.read(Memory::kSize - 1));
}

TEST(Memory, LoadProgramStartsAt0x200) {
    Memory mem;
    mem.load_program({0x12, 0x34, 0x56});
    EXPECT_EQ(mem.read(0x200), 0x12);
    EXPECT_EQ(mem.read(0x201), 0x34);
    EXPECT_EQ(mem.read(0x202), 0x56);
    EXPECT_EQ(mem.read(0x203), 0x00);
}

TEST(Memory, LoadProgramAcceptsExactlyTheAvailableSpace) {
    Memory mem;
    std::vector<std::uint8_t> rom(Memory::kMaxRomSize, 0x77);
    EXPECT_NO_THROW(mem.load_program(rom));
    EXPECT_EQ(mem.read(Memory::kSize - 1), 0x77);
}

TEST(Memory, LoadProgramTooLargeThrows) {
    Memory mem;
    std::vector<std::uint8_t> rom(Memory::kMaxRomSize + 1, 0x77);
    EXPECT_THROW(mem.load_program(rom), MemoryAccessError);
}

TEST(Memory, ResetClearsRamButKeepsFont) {
    Memory mem;
    mem.write(0x300, 0xFF);
    mem.reset();
    EXPECT_EQ(mem.read(0x300), 0x00);
    EXPECT_EQ(mem.read(Memory::kFontStart), 0xF0);
}
