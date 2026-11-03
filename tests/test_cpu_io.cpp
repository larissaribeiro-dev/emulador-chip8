#include "errors.hpp"
#include "test_helpers.hpp"

using chip8::Chip8;
using namespace testutil;

TEST(CpuIo, ClearScreen) {
    Chip8 m;
    load_and_run(m, {0xF029, 0xD005, 0x00E0}, 3);
    EXPECT_EQ(m.display().to_ascii(), chip8::Display{}.to_ascii());
}

TEST(CpuIo, DrawFontGlyphAndDetectCollisionOnSecondDraw) {
    Chip8 m;
    // I aponta para o glifo "0", desenha em (0,0) duas vezes.
    m.load_rom(assemble({0xF029, 0xD005, 0xD005}));
    run(m, 2);
    EXPECT_EQ(V(m, 0xF), 0);
    EXPECT_EQ(m.display().to_ascii().substr(0, 6), "####..");  // linha 0 do glifo 0 (F0)
    EXPECT_TRUE(m.display().pixel(0, 1));                       // 0x90
    EXPECT_FALSE(m.display().pixel(1, 1));

    m.step();
    EXPECT_EQ(V(m, 0xF), 1);
    EXPECT_FALSE(m.display().pixel(0, 0));
}

TEST(CpuIo, DrawWrapsStartCoordinate) {
    Chip8 m;
    load_and_run(m, {0x6046, 0x6121, 0xF029, 0xD011}, 4);  // x=70, y=33 -> (6,1)
    EXPECT_TRUE(m.display().pixel(6, 1));
}

TEST(CpuIo, DrawWithZeroRowsDrawsNothing) {
    Chip8 m;
    load_and_run(m, {0x6F01, 0xF029, 0xD000}, 3);
    EXPECT_EQ(V(m, 0xF), 0);
    EXPECT_FALSE(m.display().pixel(0, 0));
}

TEST(CpuIo, DrawReadingPastTheEndOfMemoryThrows) {
    Chip8 m;
    m.load_rom(assemble({0xAFFE, 0xD005}));
    m.step();
    EXPECT_THROW(m.step(), chip8::MemoryAccessError);
}

TEST(CpuIo, SkipIfKeyPressed) {
    Chip8 m;
    m.load_rom(assemble({0x6005, 0xE09E}));
    run(m, 1);
    m.keypad().set_key(5, true);
    m.step();
    EXPECT_EQ(m.cpu().state().pc, 0x206);

    m.load_rom(assemble({0x6005, 0xE09E}));
    run(m, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x204);
}

TEST(CpuIo, SkipIfKeyNotPressed) {
    Chip8 m;
    load_and_run(m, {0x6005, 0xE0A1}, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x206);

    m.load_rom(assemble({0x6005, 0xE0A1}));
    m.keypad().set_key(5, true);
    run(m, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x204);
}

TEST(CpuIo, KeyValueIsMaskedToLowNibble) {
    Chip8 m;
    m.load_rom(assemble({0x6013, 0xE09E}));  // 0x13 -> tecla 3
    m.keypad().set_key(3, true);
    run(m, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x206);
}

TEST(CpuIo, UnknownEFamilyOpcodeThrows) {
    Chip8 m;
    m.load_rom(assemble({0xE000}));
    EXPECT_THROW(m.step(), chip8::UnknownOpcodeError);
}

TEST(CpuIo, TimerInstructions) {
    Chip8 m;
    load_and_run(m, {0x603C, 0xF015, 0xF118, 0xF207}, 4);
    EXPECT_EQ(m.timers().delay, 0x3C);
    EXPECT_EQ(m.timers().sound, 0x00);  // V1 ainda vale 0

    m.tick_timers();
    m.cpu().state().pc = 0x206;
    m.step();
    EXPECT_EQ(V(m, 2), 0x3B);
}

TEST(CpuIo, SoundTimerStartsBeep) {
    Chip8 m;
    load_and_run(m, {0x6002, 0xF018}, 2);
    EXPECT_TRUE(m.timers().beeping());
    m.tick_timers();
    m.tick_timers();
    EXPECT_FALSE(m.timers().beeping());
}

TEST(CpuIo, WaitForKeyBlocksUntilPressAndRelease) {
    Chip8 m;
    m.load_rom(assemble({0xF00A}));

    m.step();
    EXPECT_EQ(m.cpu().state().pc, 0x200);  // repete a mesma instrução

    m.keypad().set_key(5, true);
    m.step();
    EXPECT_EQ(m.cpu().state().pc, 0x200);  // pressionada ainda não basta

    m.keypad().set_key(5, false);
    m.step();
    EXPECT_EQ(m.cpu().state().pc, 0x202);
    EXPECT_EQ(V(m, 0), 5);
}

TEST(CpuIo, WaitForKeyIgnoresReleasesFromBeforeTheInstruction) {
    Chip8 m;
    m.load_rom(assemble({0xF00A}));
    m.keypad().set_key(3, true);
    m.keypad().set_key(3, false);
    m.step();
    EXPECT_EQ(m.cpu().state().pc, 0x200);
}

TEST(CpuIo, RandomIsMaskedWithTheImmediate) {
    Chip8 m;
    m.cpu().set_random_source([] { return std::uint8_t{0xAB}; });
    load_and_run(m, {0xC00F}, 1);
    EXPECT_EQ(V(m, 0), 0x0B);

    m.cpu().set_random_source([] { return std::uint8_t{0xFF}; });
    load_and_run(m, {0xC1FF}, 1);
    EXPECT_EQ(V(m, 1), 0xFF);
}
