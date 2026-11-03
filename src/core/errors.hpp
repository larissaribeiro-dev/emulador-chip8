#pragma once

#include <cstdint>
#include <stdexcept>

namespace chip8 {

// Base de todas as falhas controladas do emulador.
class Chip8Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Opcode que a CPU não sabe executar.
class UnknownOpcodeError : public Chip8Error {
public:
    UnknownOpcodeError(std::uint16_t opcode, std::uint16_t pc);

    std::uint16_t opcode() const { return opcode_; }
    std::uint16_t pc() const { return pc_; }

private:
    std::uint16_t opcode_;
    std::uint16_t pc_;
};

// Leitura ou escrita fora dos 4 KiB de memória.
class MemoryAccessError : public Chip8Error {
public:
    using Chip8Error::Chip8Error;
};

// CALL com a pilha cheia ou RET com a pilha vazia.
class StackError : public Chip8Error {
public:
    using Chip8Error::Chip8Error;
};

// ROM inexistente, vazia ou grande demais.
class RomError : public Chip8Error {
public:
    using Chip8Error::Chip8Error;
};

}  // namespace chip8
