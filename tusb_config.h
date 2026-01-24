#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"

// 串口配置
#define UART_ID uart0
#define BAUD_RATE 921600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// LED 用于调试 (RP2040 Zero 是 25? 不同板子可能不同，暂定25)
// 如果你是 RP2040-Zero，LED 是 WS2812，比较复杂，这里先忽略 LED，只保证逻辑
// 如果是普通 Pico，LED 是 25

void hid_task(void);

int main(void) {
    // 1. 初始化系统时钟为 125MHz (USB 需要)
    // 这是必须的，否则 USB 时钟不对无法枚举
    set_sys_clock_khz(125000, true);

    // 2. 初始化 USB 协议栈
    tusb_init();

    // 3. 初始化 UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // 4. 主循环
    while (1) {
        //处理 USB 事务（必须频繁调用）
        tud_task();

        // 自己的逻辑
        hid_task();
    }

    return 0;
}

void hid_task(void) {
    // 检查是否收到完整的 5 字节
    if (uart_is_readable(UART_ID)) {
        // 为了防止数据错位，我们可以尝试先读取足够的数据
        // 但为了简单和速度，这里假设数据流是同步的
        // 实际上最好加个超时或者包头，但在极简模式下我们直接读

        static uint8_t buffer[5];
        static uint8_t idx = 0;

        buffer[idx] = uart_getc(UART_ID);
        idx++;

        if (idx >= 5) {
            // 解析数据
            // [dx, dy, buttons, wheel, rsv]

            // 只有当 USB 准备好时才发送
            if (tud_hid_ready()) {
                uint8_t report[4];
                report[0] = buffer[2];           // Byte 0: buttons
                report[1] = (uint8_t)buffer[0];  // Byte 1: x
                report[2] = (uint8_t)buffer[1];  // Byte 2: y
                report[3] = (uint8_t)buffer[3];  // Byte 3: wheel

                // 发送报告，Report ID = 0 (因为描述符里没有定义 ID)
                tud_hid_report(0, report, sizeof(report));
            }

            // 重置索引，准备读下一个包
            idx = 0;
        }
    }
}

// --------- USB 回调函数 (必须有，否则链接报错) ---------

// 收到 GET_REPORT 请求
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
    return 0;
}

// 收到 SET_REPORT 请求
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}
