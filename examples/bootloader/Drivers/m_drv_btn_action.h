
#ifndef DRV_BTN_ACT_H__
#define DRV_BTN_ACT_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DRV_BTN_NUM_INVALID                 0xFFFF
#define DRV_BTN_NUMS                        (CONFIG_IO_BUTTON_0 != 0xFFFFFFFF)  \
                                            +(CONFIG_IO_BUTTON_1 != 0xFFFFFFFF) \
                                            +(CONFIG_IO_BUTTON_2 != 0xFFFFFFFF) \
                                            +(CONFIG_IO_BUTTON_3 != 0xFFFFFFFF)

typedef enum
{
    DRV_BTN_ACT_EVT_FIRST               = 0,
    DRV_BTN_ACT_EVT_IDLE                = DRV_BTN_ACT_EVT_FIRST,
    DRV_BTN_ACT_EVT_SHORT,
    DRV_BTN_ACT_EVT_DOUBLE,
    DRV_BTN_ACT_EVT_LONG,
    DRV_BTN_ACT_EVT_6S_LONG,
    DRV_BTN_ACT_EVT_12S_LONG,
    DRV_BTN_ACT_EVT_SHORT_RELEASED,
    DRV_BTN_ACT_EVT_DOUBLE_RELEASED,
    DRV_BTN_ACT_EVT_LONG_RELEASED,
    DRV_BTN_ACT_EVT_6S_LONG_RELEASED,
    DRV_BTN_ACT_EVT_12S_LONG_RELEASED,
    DRV_BTN_ACT_EVT_INVALID,
}drv_btn_act_evt_type_t;

typedef void(*drv_btn_act_event_handler_t)(uint16_t button, drv_btn_act_evt_type_t evt);

uint32_t drv_btn_act_init(drv_btn_act_event_handler_t handler);

uint32_t drv_btn_act_uninit(void);

uint32_t drv_btn_act_enable(void);

uint32_t drv_btn_act_disable(void);

bool drv_btn_action_wakeup_prepare(bool wakeup);

bool drv_btn_action_check_enter_normal_state_button(void);

#ifdef __cplusplus
}
#endif

#endif // DRV_BTN_ACT_H__

