#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

#include "core/chip8.hpp"
#include "core/debug.hpp"
#include "core/errors.hpp"
#include "core/rom.hpp"
#include "platform.hpp"

namespace {

constexpr int kFrameRate = 60;

struct Options {
    std::string rom_path;
    int scale = 12;
    int ips = 700;
    chip8::Quirks quirks = chip8::Quirks::modern();
    bool trace = false;
    long headless_cycles = -1;  // -1 = com janela
    bool dump_memory = false;
    std::uint16_t dump_start = 0;
    std::uint16_t dump_length = 0;
};

void print_usage(const char* program) {
    std::printf(
        "Uso: %s [opções] <rom.ch8>\n\n"
        "Opções:\n"
        "  --scale N            tamanho de cada pixel na janela (padrão 12)\n"
        "  --ips N              instruções por segundo (padrão 700)\n"
        "  --quirks vip|moderno comportamento das instruções ambíguas (padrão moderno)\n"
        "  --trace              imprime cada instrução executada\n"
        "  --headless N         executa N instruções sem janela e mostra o estado final\n"
        "  --dump-mem INI:TAM   com --headless, mostra TAM bytes da memória a partir de INI\n"
        "  -h, --help           mostra esta ajuda\n\n"
        "Teclas: 1234 / QWER / ASDF / ZXCV representam o teclado hexadecimal. Esc sai.\n",
        program);
}

bool parse_number(const std::string& text, long& out) {
    char* end = nullptr;
    out = std::strtol(text.c_str(), &end, 0);
    return end != text.c_str() && *end == '\0';
}

// Retorna false se os argumentos forem inválidos.
bool parse_args(int argc, char** argv, Options& opt) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto value = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "erro: %s precisa de um valor\n", name);
                return nullptr;
            }
            return argv[++i];
        };

        if (arg == "--scale" || arg == "--ips" || arg == "--headless") {
            const char* text = value(arg.c_str());
            long number = 0;
            if (!text) return false;
            if (!parse_number(text, number) || number <= 0) {
                std::fprintf(stderr, "erro: valor inválido para %s: '%s'\n", arg.c_str(), text);
                return false;
            }
            if (arg == "--scale") opt.scale = static_cast<int>(number);
            if (arg == "--ips") opt.ips = static_cast<int>(number);
            if (arg == "--headless") opt.headless_cycles = number;
        } else if (arg == "--quirks") {
            const char* text = value("--quirks");
            if (!text) return false;
            const std::string name = text;
            if (name == "vip") opt.quirks = chip8::Quirks::vip();
            else if (name == "moderno") opt.quirks = chip8::Quirks::modern();
            else {
                std::fprintf(stderr, "erro: --quirks aceita 'vip' ou 'moderno', recebeu '%s'\n", text);
                return false;
            }
        } else if (arg == "--dump-mem") {
            const char* text = value("--dump-mem");
            if (!text) return false;
            const std::string range = text;
            const auto colon = range.find(':');
            long start = 0, length = 0;
            if (colon == std::string::npos || !parse_number(range.substr(0, colon), start) ||
                !parse_number(range.substr(colon + 1), length) || start < 0 || length <= 0 ||
                start >= 4096) {
                std::fprintf(stderr, "erro: use --dump-mem INICIO:TAMANHO, por exemplo 0x200:32\n");
                return false;
            }
            opt.dump_memory = true;
            opt.dump_start = static_cast<std::uint16_t>(start);
            opt.dump_length = static_cast<std::uint16_t>(length);
        } else if (arg == "--trace") {
            opt.trace = true;
        } else if (!arg.empty() && arg[0] == '-') {
            std::fprintf(stderr, "erro: opção desconhecida '%s'\n", arg.c_str());
            return false;
        } else if (opt.rom_path.empty()) {
            opt.rom_path = arg;
        } else {
            std::fprintf(stderr, "erro: mais de uma ROM informada\n");
            return false;
        }
    }

    if (opt.rom_path.empty()) {
        std::fprintf(stderr, "erro: informe o caminho de uma ROM\n");
        return false;
    }
    if (opt.dump_memory && opt.headless_cycles < 0) {
        std::fprintf(stderr, "erro: --dump-mem só funciona junto com --headless\n");
        return false;
    }
    return true;
}

void step_once(chip8::Chip8& machine, bool trace) {
    if (trace) std::puts(chip8::format_trace_line(machine).c_str());
    machine.step();
}

int run_headless(chip8::Chip8& machine, const Options& opt) {
    const long per_tick = opt.ips / kFrameRate > 0 ? opt.ips / kFrameRate : 1;

    for (long cycle = 0; cycle < opt.headless_cycles; ++cycle) {
        step_once(machine, opt.trace);
        if ((cycle + 1) % per_tick == 0) machine.tick_timers();
    }

    std::printf("\nEstado após %ld instruções\n", opt.headless_cycles);
    std::printf("%s\n", chip8::format_registers(machine).c_str());
    std::printf("%s", machine.display().to_ascii().c_str());
    if (opt.dump_memory) {
        std::printf("\nMemória\n%s", chip8::hexdump(machine.memory(), opt.dump_start, opt.dump_length).c_str());
    }
    return 0;
}

int run_windowed(chip8::Chip8& machine, const Options& opt) {
    frontend::Platform platform("CHIP-8", opt.scale);

    using clock = std::chrono::steady_clock;
    const auto frame_time = std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(1.0 / kFrameRate));
    const int per_frame = opt.ips / kFrameRate > 0 ? opt.ips / kFrameRate : 1;
    auto next_frame = clock::now();

    while (platform.poll_events(machine.keypad())) {
        for (int k = 0; k < per_frame; ++k) step_once(machine, opt.trace);
        machine.tick_timers();

        platform.set_beep(machine.timers().beeping());
        if (machine.display().dirty()) {
            platform.render(machine.display());
            machine.display().clear_dirty();
        }

        next_frame += frame_time;
        std::this_thread::sleep_until(next_frame);
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc >= 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        print_usage(argv[0]);
        return 0;
    }

    Options opt;
    if (!parse_args(argc, argv, opt)) {
        std::fprintf(stderr, "Use --help para ver as opções.\n");
        return 2;
    }

    chip8::Chip8 machine(opt.quirks);
    try {
        machine.load_rom(chip8::read_rom_file(opt.rom_path));
        return opt.headless_cycles >= 0 ? run_headless(machine, opt) : run_windowed(machine, opt);
    } catch (const chip8::UnknownOpcodeError& e) {
        std::fprintf(stderr, "\nerro: %s\n%s", e.what(), chip8::format_registers(machine).c_str());
        return 1;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "\nerro: %s\n", e.what());
        return 1;
    }
}
