#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "tusb.h"
// 必须包含这个，否则报错 unknown type name 'hid_report_type_t'
#include "class/hid/hid_device.h"

#define UART_ID uart0
#define BAUD_RATE 921600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

void hid_task(void);

int main(void) {
    // 1. 设置时钟 (必须!)
    set_sys_clock_khz(125000, true);

    // 2. 初始化 USB
    tusb_init();

    // 3. 初始化 UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    while (1) {
        // USB 任务 (必须频繁调用)
        tud_task();

        // 自己的逻辑
        hid_task();
    }
}

void hid_task(void) {
    if (uart_is_readable(UART_ID)) {
        static uint8_t buffer[5];
        static uint8_t idx = 0;

        buffer[idx] = uart_getc(UART_ID);
        idx++;

        if (idx >= 5) {
            // 只有当 USB 准备好时才处理
            if (tud_hid_ready()) {
                uint8_t report[4];
                // 协议: [dx, dy, buttons, wheel, rsv]
                report[0] = buffer[2];           // buttons
                report[1] = (uint8_t)buffer[0];  // x
                report[2] = (uint8_t)buffer[1];  // y
                report[3] = (uint8_t)buffer[3];  // wheel

                // 发送
                tud_hid_report(0, report, sizeof(report));
            }
            idx = 0;
        }
    }
}

// 回调函数
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}
