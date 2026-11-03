#pragma once

namespace chip8 {

// Comportamentos em que as variantes do CHIP-8 divergem.
// As diferenças estão descritas no README.
struct Quirks {
    // 8XY6 e 8XYE: true lê VY (COSMAC VIP), false desloca o próprio VX.
    bool shift_uses_vy = false;

    // FX55 e FX65: true deixa I apontando para depois do último registrador copiado.
    bool load_store_increments_i = false;

    // 8XY1, 8XY2 e 8XY3: true zera VF depois da operação lógica.
    bool logic_resets_vf = false;

    // Comportamento do CHIP-8 original (COSMAC VIP).
    static constexpr Quirks vip() { return Quirks{true, true, true}; }

    // Comportamento de CHIP-48 e SUPER-CHIP, o mais comum nas ROMs distribuídas.
    static constexpr Quirks modern() { return Quirks{false, false, false}; }
};

}  // namespace chip8
