#include "cpu.hpp"

#include <memory>
#include <random>

#include "errors.hpp"

namespace chip8 {

namespace {

constexpr std::size_t kStackDepth = 16;
constexpr std::size_t kVF = 0xF;

}  // namespace

Cpu::Cpu(Memory& memory, Display& display, Keypad& keypad, Timers& timers, Quirks quirks)
    : memory_(memory), display_(display), keypad_(keypad), timers_(timers), quirks_(quirks) {
    auto engine = std::make_shared<std::mt19937>(std::random_device{}());
    random_ = [engine]() {
        std::uniform_int_distribution<int> dist(0, 255);
        return static_cast<std::uint8_t>(dist(*engine));
    };
}

void Cpu::reset() {
    state_ = CpuState{};
    waiting_for_key_ = false;
}

std::uint16_t Cpu::fetch() const {
    const std::uint16_t pc = state_.pc;
    const std::uint16_t high = memory_.read(pc);
    const std::uint16_t low = memory_.read(static_cast<std::uint16_t>(pc + 1));
    return static_cast<std::uint16_t>((high << 8) | low);
}

std::uint16_t Cpu::step() {
    const std::uint16_t opcode_pc = state_.pc;
    const std::uint16_t opcode = fetch();
    state_.pc = static_cast<std::uint16_t>(state_.pc + 2);
    execute(opcode, opcode_pc);
    return opcode;
}

void Cpu::execute(std::uint16_t opcode, std::uint16_t opcode_pc) {
    const std::size_t x = (opcode >> 8) & 0xF;
    const std::size_t y = (opcode >> 4) & 0xF;
    const auto n = static_cast<std::uint8_t>(opcode & 0xF);
    const auto nn = static_cast<std::uint8_t>(opcode & 0xFF);
    const std::uint16_t nnn = opcode & 0xFFF;
    auto& v = state_.v;

    switch (opcode >> 12) {
        case 0x0:
            if (opcode == 0x00E0) {
                display_.clear();
            } else if (opcode == 0x00EE) {
                if (state_.sp == 0) throw StackError("RET com a pilha vazia");
                state_.pc = state_.stack[--state_.sp];
            } else {
                // 0NNN (SYS) chamaria código de máquina do computador original.
                throw UnknownOpcodeError(opcode, opcode_pc);
            }
            break;

        case 0x1:
            state_.pc = nnn;
            break;

        case 0x2:
            if (state_.sp >= kStackDepth) throw StackError("CALL com a pilha cheia");
            state_.stack[state_.sp++] = state_.pc;
            state_.pc = nnn;
            break;

        case 0x3:
            if (v[x] == nn) state_.pc += 2;
            break;

        case 0x4:
            if (v[x] != nn) state_.pc += 2;
            break;

        case 0x5:
            if (n != 0) throw UnknownOpcodeError(opcode, opcode_pc);
            if (v[x] == v[y]) state_.pc += 2;
            break;

        case 0x6:
            v[x] = nn;
            break;

        case 0x7:
            v[x] = static_cast<std::uint8_t>(v[x] + nn);
            break;

        case 0x8:
            execute_arithmetic(opcode, opcode_pc);
            break;

        case 0x9:
            if (n != 0) throw UnknownOpcodeError(opcode, opcode_pc);
            if (v[x] != v[y]) state_.pc += 2;
            break;

        case 0xA:
            state_.i = nnn;
            break;

        case 0xB:
            state_.pc = static_cast<std::uint16_t>(nnn + v[0]);
            break;

        case 0xC:
            v[x] = random_() & nn;
            break;

        case 0xD: {
            std::array<std::uint8_t, 15> sprite{};
            for (std::size_t row = 0; row < n; ++row) {
                sprite[row] = memory_.read(static_cast<std::uint16_t>(state_.i + row));
            }
            v[kVF] = display_.draw_sprite(v[x], v[y], sprite.data(), n) ? 1 : 0;
            break;
        }

        case 0xE:
            if (nn == 0x9E) {
                if (keypad_.is_pressed(v[x] & 0xF)) state_.pc += 2;
            } else if (nn == 0xA1) {
                if (!keypad_.is_pressed(v[x] & 0xF)) state_.pc += 2;
            } else {
                throw UnknownOpcodeError(opcode, opcode_pc);
            }
            break;

        case 0xF:
            execute_misc(opcode, opcode_pc);
            break;
    }
}

void Cpu::execute_arithmetic(std::uint16_t opcode, std::uint16_t opcode_pc) {
    const std::size_t x = (opcode >> 8) & 0xF;
    const std::size_t y = (opcode >> 4) & 0xF;
    auto& v = state_.v;

    // Os operandos são copiados antes de qualquer escrita, porque VF também
    // pode ser VX ou VY e o flag só deve ser gravado por último.
    const std::uint8_t vx = v[x];
    const std::uint8_t vy = v[y];
    std::uint8_t flag = 0;
    bool sets_flag = true;

    switch (opcode & 0xF) {
        case 0x0:
            v[x] = vy;
            sets_flag = false;
            break;
        case 0x1:
            v[x] = vx | vy;
            flag = 0;
            sets_flag = quirks_.logic_resets_vf;
            break;
        case 0x2:
            v[x] = vx & vy;
            flag = 0;
            sets_flag = quirks_.logic_resets_vf;
            break;
        case 0x3:
            v[x] = vx ^ vy;
            flag = 0;
            sets_flag = quirks_.logic_resets_vf;
            break;
        case 0x4: {
            const unsigned sum = vx + vy;
            v[x] = static_cast<std::uint8_t>(sum);
            flag = sum > 0xFF ? 1 : 0;
            break;
        }
        case 0x5:
            v[x] = static_cast<std::uint8_t>(vx - vy);
            flag = vx >= vy ? 1 : 0;
            break;
        case 0x6: {
            const std::uint8_t source = quirks_.shift_uses_vy ? vy : vx;
            v[x] = source >> 1;
            flag = source & 0x01;
            break;
        }
        case 0x7:
            v[x] = static_cast<std::uint8_t>(vy - vx);
            flag = vy >= vx ? 1 : 0;
            break;
        case 0xE: {
            const std::uint8_t source = quirks_.shift_uses_vy ? vy : vx;
            v[x] = static_cast<std::uint8_t>(source << 1);
            flag = (source >> 7) & 0x01;
            break;
        }
        default:
            throw UnknownOpcodeError(opcode, opcode_pc);
    }

    if (sets_flag) v[kVF] = flag;
}

void Cpu::execute_misc(std::uint16_t opcode, std::uint16_t opcode_pc) {
    const std::size_t x = (opcode >> 8) & 0xF;
    auto& v = state_.v;

    switch (opcode & 0xFF) {
        case 0x07:
            v[x] = timers_.delay;
            break;

        case 0x0A:
            wait_for_key(static_cast<std::uint8_t>(x));
            break;

        case 0x15:
            timers_.delay = v[x];
            break;

        case 0x18:
            timers_.sound = v[x];
            break;

        case 0x1E:
            state_.i = static_cast<std::uint16_t>(state_.i + v[x]);
            break;

        case 0x29:
            state_.i = Memory::glyph_address(v[x]);
            break;

        case 0x33:
            memory_.write(state_.i, v[x] / 100);
            memory_.write(static_cast<std::uint16_t>(state_.i + 1), (v[x] / 10) % 10);
            memory_.write(static_cast<std::uint16_t>(state_.i + 2), v[x] % 10);
            break;

        case 0x55:
            for (std::size_t k = 0; k <= x; ++k) {
                memory_.write(static_cast<std::uint16_t>(state_.i + k), v[k]);
            }
            if (quirks_.load_store_increments_i) state_.i = static_cast<std::uint16_t>(state_.i + x + 1);
            break;

        case 0x65:
            for (std::size_t k = 0; k <= x; ++k) {
                v[k] = memory_.read(static_cast<std::uint16_t>(state_.i + k));
            }
            if (quirks_.load_store_increments_i) state_.i = static_cast<std::uint16_t>(state_.i + x + 1);
            break;

        default:
            throw UnknownOpcodeError(opcode, opcode_pc);
    }
}

// FX0A bloqueia a CPU repetindo a própria instrução até uma tecla ser solta.
// Os temporizadores continuam correndo porque quem os decrementa é a máquina.
void Cpu::wait_for_key(std::uint8_t x) {
    if (!waiting_for_key_) {
        waiting_for_key_ = true;
        keypad_.discard_release();
    }

    if (auto key = keypad_.take_release()) {
        state_.v[x] = *key;
        waiting_for_key_ = false;
    } else {
        state_.pc = static_cast<std::uint16_t>(state_.pc - 2);
    }
}

}  // namespace chip8
