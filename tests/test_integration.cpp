#include <algorithm>
#include <string>

#include "debug.hpp"
#include "errors.hpp"
#include "test_helpers.hpp"

using chip8::Chip8;
using namespace testutil;

namespace {

// Soma 1 + 2 + ... + 10, converte para BCD em 0x300 e recarrega nos registradores.
const std::initializer_list<std::uint16_t> kSumProgram = {
    0x6000,  // 200: LD V0, 0       soma
    0x6101,  // 202: LD V1, 1       contador
    0x8014,  // 204: ADD V0, V1
    0x7101,  // 206: ADD V1, 1
    0x310B,  // 208: SE V1, 11
    0x1204,  // 20A: JP 204
    0xA300,  // 20C: LD I, 0x300
    0xF033,  // 20E: LD B, V0
    0xF265,  // 210: LD V2, [I]
    0x1212,  // 212: JP 212         fim
};

}  // namespace

TEST(Integration, LoopSumsOneToTenAndStoresBcd) {
    Chip8 m;
    m.load_rom(assemble(kSumProgram));
    run(m, 100);

    EXPECT_EQ(m.cpu().state().pc, 0x212);
    EXPECT_EQ(m.memory().read(0x300), 0);
    EXPECT_EQ(m.memory().read(0x301), 5);
    EXPECT_EQ(m.memory().read(0x302), 5);
    EXPECT_EQ(V(m, 0), 0);
    EXPECT_EQ(V(m, 1), 5);
    EXPECT_EQ(V(m, 2), 5);
}

TEST(Integration, SameResultOnBothQuirkPresets) {
    Chip8 vip(chip8::Quirks::vip());
    vip.load_rom(assemble(kSumProgram));
    run(vip, 100);
    EXPECT_EQ(vip.memory().read(0x301), 5);
    EXPECT_EQ(vip.cpu().state().i, 0x303);  // FX65 avançou I
}

TEST(Integration, SubroutineDrawsDigitsSideBySide) {
    Chip8 m;
    m.load_rom(assemble({
        0x6000, 0x6101, 0x6201,  // 200-204
        0x2212,                  // 206: CALL 212
        0x7005, 0x7201,          // 208, 20A
        0x3204,                  // 20C: SE V2, 4
        0x1206,                  // 20E: JP 206
        0x1210,                  // 210: JP 210 (fim)
        0xF229, 0xD015, 0x00EE,  // 212: LD F,V2 / DRW V0,V1,5 / RET
    }));
    run(m, 80);

    // Dígito 1 em x=0, dígito 2 em x=5, dígito 3 em x=10, todos a partir de y=1.
    EXPECT_TRUE(m.display().pixel(2, 1));   // topo do "1" (0x20)
    EXPECT_TRUE(m.display().pixel(5, 1));   // topo do "2" (0xF0)
    EXPECT_TRUE(m.display().pixel(10, 1));  // topo do "3" (0xF0)
    EXPECT_EQ(m.cpu().state().sp, 0);
    EXPECT_EQ(m.cpu().state().pc, 0x210);
}

TEST(Integration, TimersKeepRunningWhileWaitingForKey) {
    Chip8 m;
    m.load_rom(assemble({0x6003, 0xF015, 0xF10A, 0xF207}));
    run(m, 2);
    EXPECT_EQ(m.timers().delay, 3);

    for (int frame = 0; frame < 3; ++frame) {
        run(m, 10);  // a CPU gira esperando a tecla
        m.tick_timers();
    }
    EXPECT_EQ(m.timers().delay, 0);
    EXPECT_EQ(m.cpu().state().pc, 0x204);

    m.keypad().set_key(0xB, true);
    m.keypad().set_key(0xB, false);
    run(m, 2);
    EXPECT_EQ(V(m, 1), 0xB);
    EXPECT_EQ(V(m, 2), 0);
}

TEST(Integration, LoadRomResetsTheWholeMachine) {
    Chip8 m;
    load_and_run(m, {0xF029, 0xD005, 0x6007, 0xF018, 0x1210}, 4);
    m.load_rom(assemble({0x00E0}));

    EXPECT_EQ(m.cpu().state().pc, 0x200);
    EXPECT_EQ(V(m, 0), 0);
    EXPECT_EQ(m.timers().sound, 0);
    EXPECT_FALSE(m.display().pixel(0, 0));
}

TEST(Integration, OversizedRomIsRejected) {
    Chip8 m;
    EXPECT_THROW(m.load_rom(std::vector<std::uint8_t>(4000, 0)), chip8::MemoryAccessError);
}

TEST(Integration, ProgramRunningIntoEmptyMemoryStopsWithAnError) {
    Chip8 m;
    m.load_rom(assemble({0x6001}));
    m.step();
    EXPECT_THROW(m.step(), chip8::UnknownOpcodeError);  // 0x0000 não é instrução válida
}

TEST(Integration, TraceLineShowsPcOpcodeAndMnemonic) {
    Chip8 m;
    m.load_rom(assemble({0x6A05}));
    const std::string line = chip8::format_trace_line(m);
    EXPECT_NE(line.find("PC=0x200"), std::string::npos);
    EXPECT_NE(line.find("6A05"), std::string::npos);
    EXPECT_NE(line.find("LD VA, 0x05"), std::string::npos);
}

TEST(Integration, HexdumpFormatsSixteenBytesPerLine) {
    Chip8 m;
    m.load_rom({0x12, 0x34});
    const std::string dump = chip8::hexdump(m.memory(), 0x200, 20);
    EXPECT_EQ(dump.substr(0, 9), "200: 12 3");
    EXPECT_EQ(std::count(dump.begin(), dump.end(), '\n'), 2);
}
