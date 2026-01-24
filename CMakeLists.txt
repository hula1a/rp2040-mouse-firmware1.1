cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(zowie_mouse C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(zowie_mouse
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(zowie_mouse PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# 关键：禁用 USB CDC
target_compile_definitions(zowie_mouse PRIVATE
    PICO_STDIO_USB=0
    PICO_STDIO_UART=0
)

target_link_libraries(zowie_mouse
    pico_stdlib
    hardware_uart
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(zowie_mouse 0)
pico_enable_stdio_uart(zowie_mouse 0)

pico_add_extra_outputs(zowie_mouse)
