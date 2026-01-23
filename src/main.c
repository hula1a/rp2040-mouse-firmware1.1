#include "pico/stdlib.h"
#include "tusb.h"

int main() {
    tusb_init();

    while (1) {
        tud_task();

        if (tud_hid_ready()) {
            // 每次右移 5 像素
            tud_hid_mouse_report(0, 0, 5, 0, 0, 0);
        }

        sleep_ms(50);  // 每 50ms 发一次
    }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
}
