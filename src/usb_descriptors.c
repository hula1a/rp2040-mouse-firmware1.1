#include "tusb.h"
#include "device/usbd.h"
#include "class/hid/hid_device.h"

#define USB_VID   0x04A5
#define USB_PID   0x8002

// ====== 设备描述符 ======
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&desc_device;
}

// ====== HID 报告描述符 ======
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(0))
};

uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    return desc_hid_report;
}

// ====== 配置描述符 ======
enum { ITF_NUM_HID, ITF_NUM_TOTAL };

#define EPNUM_HID   0x81

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0,
        TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN,
        0x00, 100),

    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_MOUSE,
        sizeof(desc_hid_report), EPNUM_HID,
        CFG_TUD_HID_EP_BUFSIZE, 1)
};

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

// ====== 字符串描述符 ======
char const* string_desc_arr[] = {
    (const char[]){ 0x09, 0x04 },
    "Zowie Gear",
    "Zowie Gaming Mouse",
    "GBA5H0045004A"
};

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t _desc_str[32];
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr)/sizeof(string_desc_arr[0]))
            return NULL;

        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        for(uint8_t i=0; i<chr_count; i++) {
            _desc_str[1+i] = str[i];
        }
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2*chr_count + 2);
    return _desc_str;
}
