cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(zowie_mouse C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(zowie_mouse
    main.c
    usb_descriptors.c
)

pico_enable_stdio_usb(zowie_mouse 0)
pico_enable_stdio_uart(zowie_mouse 0)

target_include_directories(zowie_mouse PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(zowie_mouse
    pico_stdlib
    pico_unique_id
    hardware_uart
    hardware_irq
    hardware_clocks
    tinyusb_device
    tinyusb_board
)

pico_add_extra_outputs(zowie_mouse)
