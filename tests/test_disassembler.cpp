#include <utility>

#include "disassembler.hpp"
#include "test_helpers.hpp"

using chip8::disassemble;

TEST(Disassembler, TranslatesEveryInstructionFamily) {
    const std::pair<std::uint16_t, const char*> cases[] = {
        {0x00E0, "CLS"},
        {0x00EE, "RET"},
        {0x1234, "JP 0x234"},
        {0x2ABC, "CALL 0xABC"},
        {0x3A12, "SE VA, 0x12"},
        {0x4B34, "SNE VB, 0x34"},
        {0x5120, "SE V1, V2"},
        {0x6A05, "LD VA, 0x05"},
        {0x7101, "ADD V1, 0x01"},
        {0x8120, "LD V1, V2"},
        {0x8121, "OR V1, V2"},
        {0x8122, "AND V1, V2"},
        {0x8123, "XOR V1, V2"},
        {0x8124, "ADD V1, V2"},
        {0x8125, "SUB V1, V2"},
        {0x8126, "SHR V1, V2"},
        {0x8127, "SUBN V1, V2"},
        {0x812E, "SHL V1, V2"},
        {0x9120, "SNE V1, V2"},
        {0xA2F0, "LD I, 0x2F0"},
        {0xB300, "JP V0, 0x300"},
        {0xC0FF, "RND V0, 0xFF"},
        {0xD015, "DRW V0, V1, 5"},
        {0xE19E, "SKP V1"},
        {0xE1A1, "SKNP V1"},
        {0xF107, "LD V1, DT"},
        {0xF10A, "LD V1, K"},
        {0xF115, "LD DT, V1"},
        {0xF118, "LD ST, V1"},
        {0xF11E, "ADD I, V1"},
        {0xF129, "LD F, V1"},
        {0xF133, "LD B, V1"},
        {0xF155, "LD [I], V1"},
        {0xF165, "LD V1, [I]"},
    };
    for (const auto& [opcode, text] : cases) {
        EXPECT_EQ(disassemble(opcode), text) << std::hex << opcode;
    }
}

TEST(Disassembler, InvalidOpcodesBecomeData) {
    EXPECT_EQ(disassemble(0x0123), "DW 0x0123");
    EXPECT_EQ(disassemble(0x5121), "DW 0x5121");
    EXPECT_EQ(disassemble(0x8128), "DW 0x8128");
    EXPECT_EQ(disassemble(0xE100), "DW 0xE100");
    EXPECT_EQ(disassemble(0xF1FF), "DW 0xF1FF");
}
