#pragma once

#include <cstdint>
#include <string>

#include "chip8.hpp"

namespace chip8 {

// Linha de rastreio: PC, opcode, mnemônico e registradores.
// Deve ser chamada antes de step(), com o opcode que está para ser executado.
std::string format_trace_line(const Chip8& machine);

// Registradores, pilha e temporizadores em várias linhas.
std::string format_registers(const Chip8& machine);

// Despejo hexadecimal de `length` bytes a partir de `start`, 16 por linha.
// Endereços fora da memória são ignorados.
std::string hexdump(const Memory& memory, std::uint16_t start, std::uint16_t length);

}  // namespace chip8
