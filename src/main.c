#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

#define UART_ID     uart0
#define BAUD_RATE   921600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

typedef struct __attribute__((packed)) {
    uint8_t buttons;
    int8_t  x;
    int8_t  y;
    int8_t  wheel;
} mouse_report_t;

int main(void) {
    tusb_init();

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);
    uart_set_hw_flow(UART_ID, false, false);

    while (true) {
        tud_task();

        // 只要收到任何 1 字节，就移动鼠标
        if (uart_is_readable(UART_ID)) {
            uart_getc(UART_ID);  // 读掉

            if (tud_hid_ready()) {
                mouse_report_t report = {0, 10, 0, 0};  // 右移 10
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
