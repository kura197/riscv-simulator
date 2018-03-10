cmake_minimum_required(VERSION 2.8)

find_package (gflags REQUIRED)
include_directories (${gflags_INCLUDE_DIR})

add_executable (emu main.cpp instruction.cpp csr.cpp intr.cpp emulator.cpp gdb.cpp)
target_link_libraries (emu gflags)
