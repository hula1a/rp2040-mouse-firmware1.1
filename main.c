#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "tusb.h"

// ========== UART 配置 ==========
#define UART_ID         uart0
#define BAUD_RATE       921600
#define UART_TX_PIN     0
#define UART_RX_PIN     1

// ========== 环形缓冲区（防止 FIFO 溢出）==========
#define RX_BUF_SIZE     256  // 必须是 2 的幂
static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile uint32_t rx_head = 0;  // 写入位置（ISR）
static volatile uint32_t rx_tail = 0;  // 读取位置（主循环）

// ========== UART 中断处理 ==========
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        uint32_t next_head = (rx_head + 1) & (RX_BUF_SIZE - 1);
        if (next_head != rx_tail) {  // 缓冲区未满
            rx_buffer[rx_head] = ch;
            rx_head = next_head;
        }
        // 满了就丢弃（或者可以设置溢出标志）
    }
}

// 从缓冲区读取一个字节
static inline int ring_read(uint8_t* out) {
    if (rx_head == rx_tail) return 0;  // 空
    *out = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) & (RX_BUF_SIZE - 1);
    return 1;
}

// 缓冲区可用字节数
static inline uint32_t ring_available() {
    return (rx_head - rx_tail) & (RX_BUF_SIZE - 1);
}

// ========== 主程序 ==========
int main() {
    // 初始化（不要用 stdio_init_all，它会启用 USB CDC）
    // set_sys_clock_khz(133000, true);  // 可选：过钟

    // UART 初始化
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // 启用 UART 接收中断
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);  // RX 中断开，TX 中断关

    // TinyUSB 初始化
    tusb_init();

    // 5 字节协议缓冲
    uint8_t packet[5];
    uint8_t packet_idx = 0;

    while (1) {
        // TinyUSB 任务（必须频繁调用）
        tud_task();

        // 从环形缓冲区读取数据，组装 5 字节包
        while (ring_available() > 0) {
            uint8_t byte;
            if (ring_read(&byte)) {
                packet[packet_idx++] = byte;

                if (packet_idx >= 5) {
                    // 解包
                    int8_t dx      = (int8_t)packet[0];
                    int8_t dy      = (int8_t)packet[1];
                    uint8_t buttons = packet[2];
                    int8_t wheel   = (int8_t)packet[3];
                    // packet[4] 保留

                    // 发送 HID 报告
                    if (tud_hid_ready()) {
                        uint8_t report[4] = {buttons, (uint8_t)dx, (uint8_t)dy, (uint8_t)wheel};
                        tud_hid_report(0, report, sizeof(report));
                    }

                    packet_idx = 0;
                }
            }
        }
    }

    return 0;
}
