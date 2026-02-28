#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "tusb.h"

// 高速串口传输，支持 1000Hz+ 轮询率
#define UART_ID      uart0
#define BAUD_RATE    921600  // 高速模式
#define UART_TX_PIN  0
#define UART_RX_PIN  1

// 协议：[0xAA, dx, dy, btn, wheel, checksum] = 6 字节
// checksum = (dx + dy + btn + wheel) & 0xFF
#define HEADER_BYTE  0xAA
#define PKT_LEN      6

// 环形缓冲区（使用 uint8_t 保证原子性）
#define RX_BUF_SIZE  64
static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile uint8_t rx_head = 0;  // 改为 uint8_t
static volatile uint8_t rx_tail = 0;  // 改为 uint8_t

// 统计信息（可选，用于调试）
static volatile uint32_t packet_count = 0;
static volatile uint32_t error_count = 0;

// UART 中断处理
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        uint8_t next = (rx_head + 1) % RX_BUF_SIZE;  // 改为 uint8_t
        if (next != rx_tail) {
            rx_buffer[rx_head] = ch;
            rx_head = next;
        } else {
            error_count++;  // 缓冲区溢出计数
        }
    }
}

// 缓冲区可用字节数
static inline uint8_t ring_available() {  // 返回值改为 uint8_t
    return (rx_head - rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE;
}

// 读取一个字节
static inline uint8_t ring_read() {
    uint8_t ch = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    return ch;
}

// 查看下一个字节（不移除）
static inline uint8_t ring_peek() {
    return rx_buffer[rx_tail];
}

int main() {
    stdio_init_all();

    // UART 初始化
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);

    // UART 中断
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    // TinyUSB
    tusb_init();

    while (1) {
        tud_task();

        // 寻找帧头并解析
        while (ring_available() >= PKT_LEN) {
            // 如果第一个字节不是帧头，丢弃它
            if (ring_peek() != HEADER_BYTE) {
                ring_read();  // 丢弃
                continue;
            }

            // 读取完整的 6 字节包
            uint8_t pkt[PKT_LEN];
            for (int i = 0; i < PKT_LEN; i++) {
                pkt[i] = ring_read();
            }

            // 解析：[0xAA, dx, dy, btn, wheel, checksum]
            int8_t dx       = (int8_t)pkt[1];
            int8_t dy       = (int8_t)pkt[2];
            uint8_t buttons = pkt[3];
            int8_t wheel    = (int8_t)pkt[4];
            uint8_t checksum = pkt[5];

            // 校验和验证
            uint8_t calc_checksum = (pkt[1] + pkt[2] + pkt[3] + pkt[4]) & 0xFF;
            if (checksum != calc_checksum) {
                error_count++;
                continue;  // 校验失败，继续寻找下一个帧头
            }

            // 发送 USB HID 报告（保持 Zowie 鼠标格式）
            if (tud_hid_ready()) {
                uint8_t report[4] = {
                    buttons,
                    (uint8_t)dx,
                    (uint8_t)dy,
                    (uint8_t)wheel
                };
                tud_hid_n_report(0, 0, report, 4);
                packet_count++;
            }
        }
    }

    return 0;
}
