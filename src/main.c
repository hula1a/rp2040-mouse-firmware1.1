#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

#define UART_ID uart0
#define BAUD_RATE 921600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

typedef struct {
    uint8_t buttons;
    int8_t  x;
    int8_t  y;
    int8_t  wheel;
} __attribute__((packed)) mouse_report_t;

int main() {
    tusb_init();
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uint8_t buffer[5];
    uint8_t idx = 0;

    while (1) {
        tud_task();
        while (uart_is_readable(UART_ID)) {
            buffer[idx++] = uart_getc(UART_ID);
            if (idx == 5) {
                mouse_report_t report = {
                    .buttons = buffer[2],
                    .x       = (int8_t)buffer[0],
                    .y       = (int8_t)buffer[1],
                    .wheel   = (int8_t)buffer[3]
                };
                if (tud_hid_ready()) {
                    tud_hid_mouse_report(0, report.buttons,
                                         report.x, report.y,
                                         report.wheel, 0);
                }
                idx = 0;
            }
        }
    }
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
}
