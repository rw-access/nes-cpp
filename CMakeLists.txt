cmake_minimum_required(VERSION 3.23)
project(nes)

set(
        CMAKE_CXX_STANDARD 20
        CMAKE_CXX_STANDARD_REQUIRED ON
        CMAKE_REQUIRED_FLAGS -std=c++20
)

add_executable(nes

        src/cartridge.cpp
        src/cartridge.h
        src/console.cpp
        src/console.h
        src/cpu.cpp
        src/cpu.h
        src/main.cpp
        src/nes.h
        src/ppu.cpp
        src/ppu.h
        src/rom.cpp
        src/rom.h
        )


include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        nes_test

        src/cartridge.cpp
        src/cartridge.h
        src/console.cpp
        src/console.h
        src/cpu.cpp
        src/cpu.h
        src/nes.h
        src/ppu.cpp
        src/ppu.h
        src/rom.cpp
        src/rom.h

        tests/cpu.cpp)

target_link_libraries(
        nes_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(nes_test)