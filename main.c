#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "tusb.h"

#define UART_ID     uart0
#define BAUD_RATE   921600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// 环形缓冲区
#define RX_BUF_SIZE 256
static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile uint32_t rx_head = 0;
static volatile uint32_t rx_tail = 0;

// UART 中断处理
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        uint32_t next = (rx_head + 1) & (RX_BUF_SIZE - 1);
        if (next != rx_tail) {
            rx_buffer[rx_head] = ch;
            rx_head = next;
        }
    }
}

static inline uint32_t ring_available() {
    return (rx_head - rx_tail) & (RX_BUF_SIZE - 1);
}

static inline uint8_t ring_read() {
    uint8_t ch = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) & (RX_BUF_SIZE - 1);
    return ch;
}

int main() {
    // UART 初始化
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // UART 中断
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    // USB 初始化
    tusb_init();

    uint8_t packet[5];
    uint8_t idx = 0;

    while (1) {
        tud_task();

        // 从缓冲区读取
        while (ring_available() > 0) {
            packet[idx++] = ring_read();

            if (idx >= 5) {
                // 解包：[dx, dy, buttons, wheel, reserved]
                int8_t dx       = (int8_t)packet[0];
                int8_t dy       = (int8_t)packet[1];
                uint8_t buttons = packet[2];
                int8_t wheel    = (int8_t)packet[3];

                // 发送 HID 报告
                if (tud_hid_ready()) {
                    // 报告格式：[buttons, x, y, wheel]
                    uint8_t report[4] = {
                        buttons,
                        (uint8_t)dx,
                        (uint8_t)dy,
                        (uint8_t)wheel
                    };
                    tud_hid_report(0, report, sizeof(report));
                }

                idx = 0;
            }
        }
    }

    return 0;
}
