#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "tusb.h"

#define UART_ID     uart0
#define BAUD_RATE   921600

typedef struct __attribute__((packed)) {
    uint8_t buttons;
    int8_t  x;
    int8_t  y;
    int8_t  wheel;
} mouse_report_t;

int main(void) {
    // 先初始化 GPIO
    gpio_init(0);
    gpio_init(1);

    // 设置为 UART 功能
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // 初始化 UART
    uart_init(UART_ID, BAUD_RATE);

    // 初始化 USB
    tusb_init();

    while (true) {
        tud_task();

        if (uart_is_readable(UART_ID)) {
            uart_getc(UART_ID);

            if (tud_hid_ready()) {
                mouse_report_t report = {0, 10, 0, 0};
                tud_hid_report(0, &report, sizeof(report));
            }
        }
    }

    return 0;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t *buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const *buffer, uint16_t bufsize) {
}
