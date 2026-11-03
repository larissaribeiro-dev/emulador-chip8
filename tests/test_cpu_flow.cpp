#include "errors.hpp"
#include "test_helpers.hpp"

using chip8::Chip8;
using namespace testutil;

TEST(CpuFlow, JumpSetsPC) {
    Chip8 m;
    load_and_run(m, {0x1234}, 1);
    EXPECT_EQ(m.cpu().state().pc, 0x234);
}

TEST(CpuFlow, PCAdvancesByTwoPerInstruction) {
    Chip8 m;
    load_and_run(m, {0x6001, 0x6002, 0x6003}, 3);
    EXPECT_EQ(m.cpu().state().pc, 0x206);
}

TEST(CpuFlow, CallPushesReturnAddressAndRetPopsIt) {
    Chip8 m;
    m.load_rom(assemble({0x2204,    // 200: CALL 204
                         0x1202,    // 202: JP 202
                         0x00EE})); // 204: RET (volta a 202)
    m.step();
    EXPECT_EQ(m.cpu().state().pc, 0x204);
    EXPECT_EQ(m.cpu().state().sp, 1);
    EXPECT_EQ(m.cpu().state().stack[0], 0x202);

    m.step();
    EXPECT_EQ(m.cpu().state().pc, 0x202);
    EXPECT_EQ(m.cpu().state().sp, 0);
}

TEST(CpuFlow, StackHoldsSixteenLevels) {
    Chip8 m;
    m.load_rom(assemble({0x2200}));  // CALL 200 chama a si mesmo
    EXPECT_NO_THROW(run(m, 16));
    EXPECT_EQ(m.cpu().state().sp, 16);
    EXPECT_THROW(m.step(), chip8::StackError);
}

TEST(CpuFlow, RetWithEmptyStackThrows) {
    Chip8 m;
    m.load_rom(assemble({0x00EE}));
    EXPECT_THROW(m.step(), chip8::StackError);
}

TEST(CpuFlow, SkipIfEqualImmediate) {
    Chip8 m;
    load_and_run(m, {0x6005, 0x3005}, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x206);  // pulou
    load_and_run(m, {0x6005, 0x3006}, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x204);  // não pulou
}

TEST(CpuFlow, SkipIfNotEqualImmediate) {
    Chip8 m;
    load_and_run(m, {0x6005, 0x4006}, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x206);
    load_and_run(m, {0x6005, 0x4005}, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x204);
}

TEST(CpuFlow, SkipIfRegistersEqual) {
    Chip8 m;
    load_and_run(m, {0x6007, 0x6107, 0x5010}, 3);
    EXPECT_EQ(m.cpu().state().pc, 0x208);
    load_and_run(m, {0x6007, 0x6108, 0x5010}, 3);
    EXPECT_EQ(m.cpu().state().pc, 0x206);
}

TEST(CpuFlow, SkipIfRegistersNotEqual) {
    Chip8 m;
    load_and_run(m, {0x6007, 0x6108, 0x9010}, 3);
    EXPECT_EQ(m.cpu().state().pc, 0x208);
    load_and_run(m, {0x6007, 0x6107, 0x9010}, 3);
    EXPECT_EQ(m.cpu().state().pc, 0x206);
}

TEST(CpuFlow, MalformedFiveAndNineOpcodesThrow) {
    Chip8 m;
    m.load_rom(assemble({0x5011}));
    EXPECT_THROW(m.step(), chip8::UnknownOpcodeError);
    m.load_rom(assemble({0x9011}));
    EXPECT_THROW(m.step(), chip8::UnknownOpcodeError);
}

TEST(CpuFlow, JumpWithOffsetAddsV0) {
    Chip8 m;
    load_and_run(m, {0x6010, 0xB300}, 2);
    EXPECT_EQ(m.cpu().state().pc, 0x310);
}

TEST(CpuFlow, JumpWithOffsetBeyondMemoryFailsOnNextFetch) {
    Chip8 m;
    load_and_run(m, {0x60FF, 0xBFFF}, 2);
    EXPECT_THROW(m.step(), chip8::MemoryAccessError);
}

TEST(CpuFlow, UnknownOpcodeReportsOpcodeAndAddress) {
    Chip8 m;
    m.load_rom(assemble({0x6001, 0xFFFF}));
    m.step();
    try {
        m.step();
        FAIL() << "era esperada UnknownOpcodeError";
    } catch (const chip8::UnknownOpcodeError& e) {
        EXPECT_EQ(e.opcode(), 0xFFFF);
        EXPECT_EQ(e.pc(), 0x202);
    }
}

TEST(CpuFlow, SysCallOpcodeIsRejected) {
    Chip8 m;
    m.load_rom(assemble({0x0123}));
    EXPECT_THROW(m.step(), chip8::UnknownOpcodeError);
}

TEST(CpuFlow, FetchOutsideMemoryThrows) {
    Chip8 m;
    m.cpu().state().pc = 0xFFF;  // o segundo byte do opcode cairia em 0x1000
    EXPECT_THROW(m.step(), chip8::MemoryAccessError);
    m.cpu().state().pc = 0x1000;
    EXPECT_THROW(m.step(), chip8::MemoryAccessError);
}
