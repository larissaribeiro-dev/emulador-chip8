include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
include(GoogleTest)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(chip8_tests
    test_cpu_arithmetic.cpp
    test_cpu_flow.cpp
    test_cpu_io.cpp
    test_cpu_memory.cpp
    test_demo_rom.cpp
    test_disassembler.cpp
    test_display.cpp
    test_integration.cpp
    test_keypad.cpp
    test_memory.cpp
    test_rom.cpp
    test_timers.cpp)
target_link_libraries(chip8_tests PRIVATE chip8_core GTest::gtest_main)
target_compile_definitions(chip8_tests PRIVATE CHIP8_ROMS_DIR="${PROJECT_SOURCE_DIR}/roms")

gtest_discover_tests(chip8_tests)
