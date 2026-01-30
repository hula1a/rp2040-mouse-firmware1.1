#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"
#include <math.h>

int main() {
    board_init();
    tusb_init();

    uint32_t counter = 0;

    while (1) {
        tud_task();

        // 每 10ms 发送一次
        static uint32_t last = 0;
        if (board_millis() - last >= 10) {
            last = board_millis();

            if (tud_hid_ready()) {
                counter++;

                // 画圆：X 和 Y 交替变化
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
