#include "rom.hpp"
#include "test_helpers.hpp"

using chip8::Chip8;
using namespace testutil;

namespace {

Chip8& boot(Chip8& m) {
    m.load_rom(chip8::read_rom_file(std::string(CHIP8_ROMS_DIR) + "/demo.ch8"));
    run(m, 400);  // desenha os 16 dígitos e para esperando uma tecla
    return m;
}

}  // namespace

TEST(DemoRom, DrawsTheSixteenHexDigits) {
    Chip8 m;
    boot(m);
    EXPECT_EQ(m.cpu().state().pc, 0x21C);  // preso em LD V0, K
    EXPECT_TRUE(m.display().pixel(2, 2));   // topo do "0" (x=2, y=2)
    EXPECT_TRUE(m.display().pixel(2, 10));  // topo do "8" na segunda linha
    EXPECT_TRUE(m.display().pixel(58, 10)); // topo do "F" (x=2+7*8)
}

TEST(DemoRom, ShowsThePressedKeyAndBeeps) {
    Chip8 m;
    boot(m);

    m.keypad().set_key(0xA, true);
    run(m, 20);
    EXPECT_FALSE(m.timers().beeping());  // só reage depois de soltar

    m.keypad().set_key(0xA, false);
    run(m, 20);

    EXPECT_TRUE(m.timers().beeping());
    EXPECT_FALSE(m.display().pixel(2, 2));   // tela foi limpa
    EXPECT_TRUE(m.display().pixel(30, 13));  // topo do glifo "A" no centro
    EXPECT_EQ(V(m, 0), 0xA);
}

TEST(DemoRom, RunsOnBothQuirkPresets) {
    Chip8 vip(chip8::Quirks::vip());
    boot(vip);
    EXPECT_EQ(vip.cpu().state().pc, 0x21C);
}
