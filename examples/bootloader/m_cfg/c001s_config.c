/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "c001s_config.h"
#include "event_bus.h"

#if CONFIG_BUTTON_ENABLED

button_action_event_map_t common_button_action_event_maps[] =
{
#if CONFIG_BUTTON_ENTER_DFU_ENABLED
    {BUTTON_6H, DRV_BTN_ACT_EVT_6S_LONG,            BUTTON_EVT_BT_DISABLE},         // 进 DFU 的过程中，先关闭 BLE，否则可能出问题
    {BUTTON_6H, DRV_BTN_ACT_EVT_10S_LONG,           BUTTON_EVT_START_DFU},
#else    
    {BUTTON_6H, DRV_BTN_ACT_EVT_12S_LONG,           BUTTON_EVT_SYSTEM_RESET},
#endif
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED,     BUTTON_EVT_BTN0_SHORT_REL},
    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};

button_action_event_map_t normal_button_action_event_maps[] =
{
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED,     BUTTON_EVT_CHECK_BT_STATE},
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED,     BUTTON_EVT_INDICATE_STEPS_GOAL},
    {BUTTON_6H, DRV_BTN_ACT_EVT_LONG,               BUTTON_EVT_BT_EN_SWTICH},

    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};

button_action_event_map_t low_power_button_action_event_maps[] =
{
    #if defined(CONFIG_BOARD_P1014) || defined(CONFIG_BOARD_P1015) || defined(CONFIG_BOARD_P1016) || defined(CONFIG_BOARD_P1018)
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED,     BUTTON_EVT_CHECK_BATT_LEVEL}, 
    {BUTTON_6H, DRV_BTN_ACT_EVT_LONG,               BUTTON_EVT_BT_EN_SWTICH},
    #else
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED,     BUTTON_EVT_CHECK_BATT_LEVEL},    
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED,     BUTTON_EVT_INDICATE_STEPS_GOAL},
    #endif

    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};

//button_action_event_map_t green_mode_button_action_event_maps[] =
//{  
//    {BUTTON_6H, DRV_BTN_ACT_EVT_LONG,               BUTTON_EVT_SYSTEM_RESUME},
//    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
//};


button_action_event_map_t find_phone_button_action_event_maps[] =
{
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED,     BUTTON_EVT_FIND_PHONE},
    {BUTTON_6H, DRV_BTN_ACT_EVT_LONG,               BUTTON_EVT_BT_EN_SWTICH},

    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};

button_action_event_map_t photo_req_button_action_event_maps[] = 
{
    {BUTTON_3H, DRV_BTN_ACT_EVT_SHORT_RELEASED, BUTTON_EVT_PHOTO_REQ},
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED, BUTTON_EVT_PHOTO_REQ},
    {BUTTON_9H, DRV_BTN_ACT_EVT_SHORT_RELEASED, BUTTON_EVT_PHOTO_REQ},
    {BUTTON_6H, DRV_BTN_ACT_EVT_LONG,           BUTTON_EVT_BT_EN_SWTICH},

    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};

button_action_event_map_t auth_ok_button_action_event_maps[] = 
{
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED, BUTTON_EVT_AUTH_OK},
    {BUTTON_6H, DRV_BTN_ACT_EVT_LONG,           BUTTON_EVT_BT_EN_SWTICH},

    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};

button_action_event_map_t reject_call_button_action_event_maps[] = 
{
    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED, BUTTON_EVT_REJECT_CALLIN},
    {BUTTON_6H, DRV_BTN_ACT_EVT_LONG,           BUTTON_EVT_BT_EN_SWTICH},

    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};

button_action_event_map_t test_mode_button_action_event_maps[] = 
{
//    {BUTTON_6H, DRV_BTN_ACT_EVT_SHORT_RELEASED, BUTTON_EVT_BTN0_SHORT_REL},
    {0xFFFF, DRV_BTN_ACT_EVT_INVALID, BUTTON_EVT_INVALID},
};


#endif // #if CONFIG_BUTTON_ENABLED




