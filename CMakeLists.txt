cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

project(read_print_bme688 C CXX ASM)

pico_sdk_init()

add_executable(read_print_bme688 read_print_bme688.c api/bme68x.c)

pico_set_program_name(read_print_bme688 "read_print_bme688")
pico_set_program_version(read_print_bme688 "0.1")

pico_enable_stdio_uart(read_print_bme688 1)
pico_enable_stdio_usb(read_print_bme688 1)

target_link_libraries(read_print_bme688 pico_stdlib hardware_i2c)

pico_add_extra_outputs(read_print_bme688)

