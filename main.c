#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "bsp/board.h"
#include "tusb.h"

#define UART_ID      uart0
#define BAUD_RATE    921600
#define UART_TX_PIN  0
#define UART_RX_PIN  1

// 环形缓冲区
#define RX_BUF_SIZE  256
static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile uint32_t rx_head = 0;
static volatile uint32_t rx_tail = 0;

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

static inline int ring_read(uint8_t* out) {
    if (rx_head == rx_tail) return 0;
    *out = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) & (RX_BUF_SIZE - 1);
    return 1;
}

static inline uint32_t ring_available() {
    return (rx_head - rx_tail) & (RX_BUF_SIZE - 1);
}

int main() {
    // 板级初始化
    board_init();

    // 过钟
    set_sys_clock_khz(133000, true);

    // UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    // TinyUSB
    tusb_init();

    uint8_t packet[5];
    uint8_t idx = 0;

    while (1) {
        tud_task();

        while (ring_available() > 0) {
            uint8_t b;
            if (ring_read(&b)) {
                packet[idx++] = b;

                if (idx >= 5) {
                    int8_t dx       = (int8_t)packet[0];
                    int8_t dy       = (int8_t)packet[1];
                    uint8_t buttons = packet[2];
                    int8_t wheel    = (int8_t)packet[3];

                    if (tud_hid_ready()) {
                        uint8_t report[4] = {
                            buttons,
                            (uint8_t)dx,
                            (uint8_t)dy,
                            (uint8_t)wheel
                        };
                        // 修复：使用正确的函数签名
                        tud_hid_n_report(0, 0, report, sizeof(report));
                    }

                    idx = 0;
                }
            }
        }
    }

    return 0;
}
