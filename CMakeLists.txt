cmake_minimum_required(VERSION 3.23)
project(nes)

include(FetchContent)
set(
        CMAKE_CXX_STANDARD 20
        CMAKE_CXX_STANDARD_REQUIRED ON
        CMAKE_REQUIRED_FLAGS -std=c++20
)


find_package(SDL2 REQUIRED)
find_library(SDL2_LIBRARY NAME SDL2)
#set(SDL_LIBRARIES ${SDL_LIBRARIES} SDL2main SDL2-static SDL2_image)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

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
        src/controller.cpp src/controller.h src/apu.cpp src/apu.h src/dsp.cpp src/dsp.h)

include_directories(nes ${SDL2_INCLUDE_DIRS})
target_link_libraries(nes ${SDL2_LIBRARIES})

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
        src/controller.cpp
        src/controller.h
        src/cpu.cpp
        src/cpu.h
        src/nes.h
        src/ppu.cpp
        src/ppu.h
        src/rom.cpp
        src/rom.h

        tests/cpu.cpp tests/audio.cpp src/apu.cpp src/apu.h)

include_directories(nes_test ${SDL2_INCLUDE_DIRS})
target_link_libraries(nes_test ${SDL2_LIBRARIES})
target_link_libraries(
        nes_test
        GTest::gtest_main
        ${SDL_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(nes_test)