cmake_minimum_required(VERSION 3.23)
project(nes)

set(CMAKE_CXX_STANDARD 20)

add_executable(nes

        src/cpu.cpp
        src/cpu.h
        src/main.cpp
        src/mapper.cpp
        src/mapper.h
        src/memory.cpp
        src/memory.h
        src/nes.h
        src/rom.cpp
        src/rom.h)


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

        src/cpu.cpp
        src/cpu.h
        src/mapper.cpp
        src/mapper.h
        src/memory.cpp
        src/memory.h
        src/nes.h
        src/rom.cpp
        src/rom.h

        tests/cpu.cpp)

target_link_libraries(
        nes_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(nes_test)