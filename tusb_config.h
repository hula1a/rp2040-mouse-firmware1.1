#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// Board config
#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE
#define CFG_TUSB_OS             OPT_OS_NONE

// 禁用 CDC
#define CFG_TUD_CDC             0

// 禁用 MSC
#define CFG_TUD_MSC             0

// 启用 HID
#define CFG_TUD_HID             1

// 端点配置
#define CFG_TUD_ENDPOINT0_SIZE  64
#define CFG_TUD_HID_EP_BUFSIZE  64

#ifdef __cplusplus
}
#endif

#endif
