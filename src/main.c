#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"
#include "class/hid/hid_device.h"

#define UART_ID uart0
#define BAUD_RATE 921600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int main() {
    // 初始化 USB
    tusb_init();

    // 初始化 UART
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
                int8_t dx = (int8_t)buffer[0];
                int8_t dy = (int8_t)buffer[1];
                uint8_t buttons = buffer[2];
                int8_t wheel = (int8_t)buffer[3];

                if (tud_hid_ready()) {
                    tud_hid_mouse_report(0, buttons, dx, dy, wheel, 0);
                }

                idx = 0;
            }
        }
    }
}

// TinyUSB 回调
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type;
    (void)buffer; (void)bufsize;
}
