#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

#define UART_ID      uart0
#define BAUD_RATE    921600
#define UART_TX_PIN  0
#define UART_RX_PIN  1

int main() {
    stdio_init_all();

    // UART 初始化
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // TinyUSB
    tusb_init();

    // 5 字节缓冲
    uint8_t buffer[5];
    uint8_t idx = 0;

    while (1) {
        tud_task();

        // 轮询读取串口（不用中断）
        while (uart_is_readable(UART_ID)) {
            buffer[idx++] = uart_getc(UART_ID);

            if (idx >= 5) {
                // 收满 5 字节，解析并发送
                int8_t dx       = (int8_t)buffer[0];
                int8_t dy       = (int8_t)buffer[1];
                uint8_t buttons = buffer[2];
                int8_t wheel    = (int8_t)buffer[3];

                if (tud_hid_ready()) {
                    uint8_t report[4] = {
                        buttons,
                        (uint8_t)dx,
                        (uint8_t)dy,
                        (uint8_t)wheel
                    };
                    tud_hid_n_report(0, 0, report, 4);
                }

                idx = 0;
            }
        }
    }

    return 0;
}
