#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// ========== 设备配置 ==========
#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE
#define CFG_TUSB_OS             OPT_OS_NONE

// ========== 关键：只启用 HID，禁用其他 ==========
#define CFG_TUD_HID             1
#define CFG_TUD_CDC             0
#define CFG_TUD_MSC             0
#define CFG_TUD_MIDI            0
#define CFG_TUD_VENDOR          0

// ========== HID 配置 ==========
#define CFG_TUD_HID_EP_BUFSIZE  8

// ========== 内存配置 ==========
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN      __attribute__((aligned(4)))

#ifdef __cplusplus
}
#endif

#endif
