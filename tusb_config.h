#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

// USB 模式：设备模式
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE

// 禁用 CDC（关键！）
#define CFG_TUD_CDC                 0

// 只启用 HID
#define CFG_TUD_HID                 1
#define CFG_TUD_MSC                 0
#define CFG_TUD_MIDI                0
#define CFG_TUD_VENDOR              0

// HID 配置
#define CFG_TUD_HID_EP_BUFSIZE      8
#define CFG_TUD_ENDPOINT0_SIZE      64

// 内存配置（关键！之前漏了）
#define CFG_TUSB_MEM_SECTION        __attribute__((section(".usb_ram")))
#define CFG_TUSB_MEM_ALIGN          __attribute__((aligned(4)))

#endif
