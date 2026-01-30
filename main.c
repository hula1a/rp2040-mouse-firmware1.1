#include "pico/stdlib.h"
#include "tusb.h"

// 获取毫秒数（替代 board_millis）
static inline uint32_t get_millis(void) {
    return to_ms_since_boot(get_absolute_time());
}

int main() {
    // Pico SDK 初始化
    stdio_init_all();
    tusb_init();

    uint32_t counter = 0;
    uint32_t last = 0;

    while (1) {
        tud_task();

        // 每 10ms 发送一次
        uint32_t now = get_millis();
        if (now - last >= 10) {
            last = now;

            if (tud_hid_ready()) {
                counter++;

                // 画方形：右→下→左→上
                int8_t dx = 0;
                int8_t dy = 0;

                int phase = (counter / 25) % 4;
                switch (phase) {
                    case 0: dx = 5;  dy = 0;  break;  // 右
                    case 1: dx = 0;  dy = 5;  break;  // 下
                    case 2: dx = -5; dy = 0;  break;  // 左
                    case 3: dx = 0;  dy = -5; break;  // 上
                }

                uint8_t report[4] = {0, (uint8_t)dx, (uint8_t)dy, 0};
                tud_hid_n_report(0, 0, report, 4);
            }
        }
    }

    return 0;
}
