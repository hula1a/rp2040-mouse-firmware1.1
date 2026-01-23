#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

#define UART_ID uart0
#define BAUD_RATE 921600
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define LED_PIN 25  // RP2040 Zero 的板载 LED

int main() {
    // 初始化 LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // 初始化 USB
    tusb_init();

    // 初始化 UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uint8_t buffer[5];
    uint8_t idx = 0;

    // LED 快闪 3 次表示启动成功
    for(int i=0; i<3; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
    }

    while (1) {
        tud_task();

        while (uart_is_readable(UART_ID)) {
            // 收到数据，LED 亮
            gpio_put(LED_PIN, 1);

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
                gpio_put(LED_PIN, 0);  // 处理完，LED 灭
            }
        }
    }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
}
