#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "tusb.h"

// UART 配置
#define UART_ID      uart0
#define BAUD_RATE    921600
#define UART_TX_PIN  0
#define UART_RX_PIN  1

// 鼠标报告结构
typedef struct __attribute__((packed)) {
    uint8_t buttons;
    int8_t  x;
    int8_t  y;
    int8_t  wheel;
} mouse_report_t;

// 接收缓冲区
static volatile uint8_t rx_buffer[5];
static volatile uint8_t rx_index = 0;
static volatile bool packet_ready = false;

// UART 中断处理函数
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);

        rx_buffer[rx_index++] = ch;

        if (rx_index >= 5) {
            packet_ready = true;
            rx_index = 0;
        }
    }
}

int main(void) {
    // 初始化 USB
    tusb_init();

    // 初始化 UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // 设置 UART 中断
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    while (true) {
        tud_task();

        if (packet_ready) {
            packet_ready = false;

            mouse_report_t report;
            report.x       = (int8_t)rx_buffer[0];
            report.y       = (int8_t)rx_buffer[1];
            report.buttons = rx_buffer[2];
            report.wheel   = (int8_t)rx_buffer[3];

            if (tud_hid_ready()) {
                tud_hid_report(0, &report, sizeof(report));
            }
        }
    }

    return 0;
}

// TinyUSB 回调
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t *buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const *buffer, uint16_t bufsize) {
}
