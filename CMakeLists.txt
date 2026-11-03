cmake_minimum_required(VERSION 3.16)

project(chip8
    VERSION 0.1.0
    DESCRIPTION "Emulador de CHIP-8 em C++17"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CHIP8_BUILD_FRONTEND "Compila o executável com janela SDL2" ON)
option(CHIP8_BUILD_TESTS "Compila os testes unitários" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()

# Núcleo do emulador. Não depende de SDL.
add_library(chip8_core STATIC
    src/core/chip8.cpp
    src/core/cpu.cpp
    src/core/debug.cpp
    src/core/disassembler.cpp
    src/core/display.cpp
    src/core/errors.cpp
    src/core/keypad.cpp
    src/core/memory.cpp
    src/core/rom.cpp)
target_include_directories(chip8_core PUBLIC src src/core)

if(MSVC)
    target_compile_options(chip8_core PRIVATE /W4)
else()
    target_compile_options(chip8_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

if(CHIP8_BUILD_FRONTEND)
    find_package(SDL2 REQUIRED)
    # Versões antigas do SDL2 não exportam o alvo SDL2::SDL2.
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 INTERFACE IMPORTED)
        target_include_directories(SDL2::SDL2 INTERFACE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(SDL2::SDL2 INTERFACE ${SDL2_LIBRARIES})
    endif()

    add_executable(chip8
        src/frontend/main.cpp
        src/frontend/platform.cpp)
    target_link_libraries(chip8 PRIVATE chip8_core SDL2::SDL2)
    # O main() é nosso, sem a indireção do SDL_main.
    target_compile_definitions(chip8 PRIVATE SDL_MAIN_HANDLED)
endif()

if(CHIP8_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
