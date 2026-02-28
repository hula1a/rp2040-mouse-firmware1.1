#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "tusb.h"

#define UART_ID      uart0
#define BAUD_RATE    115200
#define UART_TX_PIN  0
#define UART_RX_PIN  1

#define HEADER_BYTE  0xAA
#define PKT_LEN      6

#define RX_BUF_SIZE  64
static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

static volatile uint32_t packet_count = 0;
static volatile uint32_t error_count = 0;
static volatile uint32_t byte_count = 0;

void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        byte_count++;
        
        uint8_t next = (rx_head + 1) % RX_BUF_SIZE;
        if (next != rx_tail) {
            rx_buffer[rx_head] = ch;
            rx_head = next;
        } else {
            error_count++;
        }
        
        // 回传收到的字节（用于调试）
        uart_putc_raw(UART_ID, ch);
    }
}

static inline uint8_t ring_available() {
    return (rx_head - rx_tail + RX_BUF_SIZE) % RX_BUF_SIZE;
}

static inline uint8_t ring_read() {
    uint8_t ch = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    return ch;
}

static inline uint8_t ring_peek() {
    return rx_buffer[rx_tail];
}

int main() {
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
        
        // 处理数据包
        while (ring_available() >= PKT_LEN) {
            if (ring_peek() != HEADER_BYTE) {
                ring_read();
                continue;
            }
            
            uint8_t pkt[PKT_LEN];
            for (int i = 0; i < PKT_LEN; i++) {
                pkt[i] = ring_read();
            }
            
            int8_t dx = (int8_t)pkt[1];
            int8_t dy = (int8_t)pkt[2];
            uint8_t buttons = pkt[3];
            int8_t wheel = (int8_t)pkt[4];
            uint8_t checksum = pkt[5];
            
            uint8_t calc_checksum = (pkt[1] + pkt[2] + pkt[3] + pkt[4]) & 0xFF;
            if (checksum != calc_checksum) {
                error_count++;
                continue;
            }
            
            // 发送 USB HID 报告
            if (tud_hid_ready() && tud_mounted()) {
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
