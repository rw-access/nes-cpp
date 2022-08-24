cmake_minimum_required(VERSION 3.23)
project(nes)

set(CMAKE_CXX_STANDARD 20)

add_executable(nes src/main.cpp src/cpu.cpp src/nes.h src/memory.cpp tests/cpu.cpp src/rom.cpp src/cpu.h)


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
        src/nes.h
        src/cpu.cpp
        src/memory.cpp
        tests/cpu.cpp
        src/rom.cpp)

target_link_libraries(
        nes_test
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(nes_test)