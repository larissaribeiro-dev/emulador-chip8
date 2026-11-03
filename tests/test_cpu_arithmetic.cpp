#include "errors.hpp"
#include "test_helpers.hpp"

using chip8::Chip8;
using chip8::Quirks;
using namespace testutil;

TEST(CpuArithmetic, LoadImmediate) {
    Chip8 m;
    load_and_run(m, {0x6A42}, 1);
    EXPECT_EQ(V(m, 0xA), 0x42);
}

TEST(CpuArithmetic, AddImmediateWrapsWithoutTouchingVF) {
    Chip8 m;
    load_and_run(m, {0x6FAA, 0x60FF, 0x7002}, 3);
    EXPECT_EQ(V(m, 0), 0x01);
    EXPECT_EQ(V(m, 0xF), 0xAA);
}

TEST(CpuArithmetic, RegisterCopy) {
    Chip8 m;
    load_and_run(m, {0x6107, 0x8010}, 2);
    EXPECT_EQ(V(m, 0), 0x07);
}

TEST(CpuArithmetic, OrAndXor) {
    Chip8 m;
    load_and_run(m, {0x60CC, 0x61AA, 0x8011}, 3);
    EXPECT_EQ(V(m, 0), 0xEE);
    load_and_run(m, {0x60CC, 0x61AA, 0x8012}, 3);
    EXPECT_EQ(V(m, 0), 0x88);
    load_and_run(m, {0x60CC, 0x61AA, 0x8013}, 3);
    EXPECT_EQ(V(m, 0), 0x66);
}

TEST(CpuArithmetic, LogicOpsKeepVFOnModernQuirks) {
    Chip8 m(Quirks::modern());
    load_and_run(m, {0x6F01, 0x60CC, 0x61AA, 0x8011}, 4);
    EXPECT_EQ(V(m, 0xF), 0x01);
}

TEST(CpuArithmetic, LogicOpsResetVFOnVipQuirks) {
    Chip8 m(Quirks::vip());
    load_and_run(m, {0x6F01, 0x60CC, 0x61AA, 0x8011}, 4);
    EXPECT_EQ(V(m, 0xF), 0x00);
}

TEST(CpuArithmetic, AddRegistersSetsCarry) {
    Chip8 m;
    load_and_run(m, {0x60C8, 0x6164, 0x8014}, 3);  // 200 + 100
    EXPECT_EQ(V(m, 0), 0x2C);
    EXPECT_EQ(V(m, 0xF), 1);
}

TEST(CpuArithmetic, AddRegistersWithoutCarryClearsVF) {
    Chip8 m;
    load_and_run(m, {0x6F01, 0x6005, 0x6106, 0x8014}, 4);
    EXPECT_EQ(V(m, 0), 0x0B);
    EXPECT_EQ(V(m, 0xF), 0);
}

TEST(CpuArithmetic, SubSetsVFWhenThereIsNoBorrow) {
    Chip8 m;
    load_and_run(m, {0x6005, 0x6103, 0x8015}, 3);
    EXPECT_EQ(V(m, 0), 0x02);
    EXPECT_EQ(V(m, 0xF), 1);
}

TEST(CpuArithmetic, SubBorrowWrapsAndClearsVF) {
    Chip8 m;
    load_and_run(m, {0x6003, 0x6105, 0x8015}, 3);
    EXPECT_EQ(V(m, 0), 0xFE);
    EXPECT_EQ(V(m, 0xF), 0);
}

TEST(CpuArithmetic, SubOfEqualValuesCountsAsNoBorrow) {
    Chip8 m;
    load_and_run(m, {0x6007, 0x6107, 0x8015}, 3);
    EXPECT_EQ(V(m, 0), 0);
    EXPECT_EQ(V(m, 0xF), 1);
}

TEST(CpuArithmetic, SubnIsReversedSubtraction) {
    Chip8 m;
    load_and_run(m, {0x6003, 0x6105, 0x8017}, 3);  // V0 = V1 - V0
    EXPECT_EQ(V(m, 0), 0x02);
    EXPECT_EQ(V(m, 0xF), 1);

    load_and_run(m, {0x6005, 0x6103, 0x8017}, 3);
    EXPECT_EQ(V(m, 0), 0xFE);
    EXPECT_EQ(V(m, 0xF), 0);
}

TEST(CpuArithmetic, ShiftRightShiftsVXOnModernQuirks) {
    Chip8 m(Quirks::modern());
    load_and_run(m, {0x6005, 0x61F0, 0x8016}, 3);
    EXPECT_EQ(V(m, 0), 0x02);
    EXPECT_EQ(V(m, 0xF), 1);
}

TEST(CpuArithmetic, ShiftRightReadsVYOnVipQuirks) {
    Chip8 m(Quirks::vip());
    load_and_run(m, {0x6005, 0x61F0, 0x8016}, 3);
    EXPECT_EQ(V(m, 0), 0x78);
    EXPECT_EQ(V(m, 0xF), 0);
}

TEST(CpuArithmetic, ShiftLeftShiftsVXOnModernQuirks) {
    Chip8 m(Quirks::modern());
    load_and_run(m, {0x60C1, 0x800E}, 2);
    EXPECT_EQ(V(m, 0), 0x82);
    EXPECT_EQ(V(m, 0xF), 1);
}

TEST(CpuArithmetic, ShiftLeftReadsVYOnVipQuirks) {
    Chip8 m(Quirks::vip());
    load_and_run(m, {0x60C1, 0x6103, 0x801E}, 3);
    EXPECT_EQ(V(m, 0), 0x06);
    EXPECT_EQ(V(m, 0xF), 0);
}

TEST(CpuArithmetic, VFIsWrittenLastWhenItIsTheDestination) {
    Chip8 m;
    // VF = 0xFF + 0x01 = 0x00 com carry. O flag (1) sobrescreve o resultado.
    load_and_run(m, {0x6FFF, 0x6101, 0x8F14}, 3);
    EXPECT_EQ(V(m, 0xF), 1);
}

TEST(CpuArithmetic, UnknownEightFamilyOpcodeThrows) {
    Chip8 m;
    m.load_rom(assemble({0x8008}));
    EXPECT_THROW(m.step(), chip8::UnknownOpcodeError);
}
