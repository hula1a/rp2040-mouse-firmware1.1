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

# 包含 tusb_config.h 所在的根目录
target_include_directories(zowie_mouse PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# 链接库：注意加入了 hardware_uart
target_link_libraries(zowie_mouse
    pico_stdlib
    hardware_uart
    tinyusb_device
    tinyusb_board
)

# 禁用 stdio 的 USB/UART 输出，防止干扰
pico_enable_stdio_usb(zowie_mouse 0)
pico_enable_stdio_uart(zowie_mouse 0)

pico_add_extra_outputs(zowie_mouse)
