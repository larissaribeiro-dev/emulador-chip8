#include "errors.hpp"
#include "test_helpers.hpp"

using chip8::Chip8;
using chip8::Memory;
using chip8::Quirks;
using namespace testutil;

TEST(CpuMemory, LoadIndexRegister) {
    Chip8 m;
    load_and_run(m, {0xA123}, 1);
    EXPECT_EQ(m.cpu().state().i, 0x123);
}

TEST(CpuMemory, AddToIndexRegister) {
    Chip8 m;
    load_and_run(m, {0xA300, 0x6010, 0xF01E}, 3);
    EXPECT_EQ(m.cpu().state().i, 0x310);
}

TEST(CpuMemory, FontGlyphAddress) {
    Chip8 m;
    load_and_run(m, {0x600A, 0xF029}, 2);
    EXPECT_EQ(m.cpu().state().i, Memory::kFontStart + 0xA * 5);
}

TEST(CpuMemory, BcdStoresHundredsTensAndUnits) {
    Chip8 m;
    load_and_run(m, {0x60FE, 0xA300, 0xF033}, 3);  // 254
    EXPECT_EQ(m.memory().read(0x300), 2);
    EXPECT_EQ(m.memory().read(0x301), 5);
    EXPECT_EQ(m.memory().read(0x302), 4);

    load_and_run(m, {0x6007, 0xA300, 0xF033}, 3);
    EXPECT_EQ(m.memory().read(0x300), 0);
    EXPECT_EQ(m.memory().read(0x301), 0);
    EXPECT_EQ(m.memory().read(0x302), 7);
}

TEST(CpuMemory, BcdNearTheEndOfMemoryThrows) {
    Chip8 m;
    m.load_rom(assemble({0x6064, 0xAFFE, 0xF033}));
    run(m, 2);
    EXPECT_THROW(m.step(), chip8::MemoryAccessError);
}

TEST(CpuMemory, StoreRegistersLeavesIUntouchedOnModernQuirks) {
    Chip8 m(Quirks::modern());
    load_and_run(m, {0x6001, 0x6102, 0x6203, 0xA300, 0xF255}, 5);
    EXPECT_EQ(m.memory().read(0x300), 1);
    EXPECT_EQ(m.memory().read(0x301), 2);
    EXPECT_EQ(m.memory().read(0x302), 3);
    EXPECT_EQ(m.cpu().state().i, 0x300);
}

TEST(CpuMemory, StoreRegistersAdvancesIOnVipQuirks) {
    Chip8 m(Quirks::vip());
    load_and_run(m, {0x6001, 0x6102, 0x6203, 0xA300, 0xF255}, 5);
    EXPECT_EQ(m.cpu().state().i, 0x303);
}

TEST(CpuMemory, StoreOnlyUpToVX) {
    Chip8 m;
    load_and_run(m, {0x6001, 0x6102, 0xA300, 0xF055}, 4);  // só V0
    EXPECT_EQ(m.memory().read(0x300), 1);
    EXPECT_EQ(m.memory().read(0x301), 0);
}

TEST(CpuMemory, LoadRegistersFillsV0ToVX) {
    Chip8 m(Quirks::modern());
    m.load_rom(assemble({0xA300, 0xF265}));
    m.memory().write(0x300, 9);
    m.memory().write(0x301, 8);
    m.memory().write(0x302, 7);
    m.memory().write(0x303, 6);
    run(m, 2);
    EXPECT_EQ(V(m, 0), 9);
    EXPECT_EQ(V(m, 1), 8);
    EXPECT_EQ(V(m, 2), 7);
    EXPECT_EQ(V(m, 3), 0);  // além de X não é alterado
    EXPECT_EQ(m.cpu().state().i, 0x300);
}

TEST(CpuMemory, LoadRegistersAdvancesIOnVipQuirks) {
    Chip8 m(Quirks::vip());
    load_and_run(m, {0xA300, 0xF265}, 2);
    EXPECT_EQ(m.cpu().state().i, 0x303);
}

TEST(CpuMemory, LoadAndStoreOutsideMemoryThrow) {
    Chip8 m;
    m.load_rom(assemble({0xAFFF, 0xF165, 0xF155}));
    m.step();
    EXPECT_THROW(m.step(), chip8::MemoryAccessError);  // lê 0xFFF e 0x1000
    m.cpu().state().pc = 0x204;
    EXPECT_THROW(m.step(), chip8::MemoryAccessError);
}

TEST(CpuMemory, UnknownFFamilyOpcodeThrows) {
    Chip8 m;
    m.load_rom(assemble({0xF0FF}));
    EXPECT_THROW(m.step(), chip8::UnknownOpcodeError);
}
