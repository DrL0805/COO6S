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

#ifndef _C001S_CONFIG_H
#define _C001S_CONFIG_H



// SoC configuration:
#define APP_TIMER_PRESCALER         0
#define IS_IO_VALID(io)             (((io) & ~0x1F) == 0)

// Fetch board configuration:
#if defined(CONFIG_BOARD_P1011)
#define CONFIG_BOARD ("C001S_P1011 "CONFIG_FIRMWARE_REVISION)
//#include "sr3_config_p1011.h"
#elif defined(CONFIG_BOARD_P1012)
#define CONFIG_BOARD ("C001S_P1012 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1012.h"
#elif defined(CONFIG_BOARD_P1013)
#define CONFIG_BOARD ("C001S_P1013 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1013.h"
#elif defined(CONFIG_BOARD_P1014)
#define CONFIG_BOARD ("C001S_P1014 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1014.h"
#elif defined(CONFIG_BOARD_P1015)
#define CONFIG_BOARD ("C001S_P1015 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1015.h"
#elif defined(CONFIG_BOARD_P1016)
#define CONFIG_BOARD ("C001S_P1016 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1016.h"
#elif defined(CONFIG_BOARD_P1116)
#define CONFIG_BOARD ("C001S_P1116 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1116.h"
#elif defined(CONFIG_BOARD_P1017)
#define CONFIG_BOARD ("C001S_P1017 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1017.h"
#elif defined(CONFIG_BOARD_P1018)
#define CONFIG_BOARD ("C001S_P1017 "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_p1018.h"
#elif defined(CONFIG_BOARD_TEST)
#define CONFIG_BOARD ("C001S_TEST "CONFIG_FIRMWARE_REVISION)
#include "sr3_config_test.h"

#elif defined(CONFIG_BOARD_BASEBOARD) || defined(CONFIG_BOARD_BASEBOARD_P1015)
#include "sr3_config_base_bootloader.h"

#else
#error "CONFIG_BOARD_xxx is not defined!"
#endif

#if CONFIG_STACK_GUARD_ENABLED
#define CONFIG_STACK_SIZE_GUARD     CONFIG_STACK_GUARD_SIZE
#else /* !CONFIG_STACK_GUARD_ENABLED */
#define CONFIG_STACK_SIZE_GUARD     0
#endif /* CONFIG_STACK_GUARD_ENABLED */

#define CONFIG_STACK_SIZE   (CONFIG_STACK_SIZE_BASE + CONFIG_STACK_SIZE_GUARD)

#if (!defined(__ASSEMBLER__) && !defined(__IAR_SYSTEMS_ASM__))
#include <stddef.h>
#include "app_util.h"

#if CONFIG_LED_ENABLED



#endif // CONFIG_LED_ENABLED

#if CONFIG_BUTTON_ENABLED

#include "m_drv_btn_action.h"



#define BUTTON_3H 1 // ??? CONFIG_IO_BUTTON_0
#define BUTTON_6H 0 // ??? CONFIG_IO_BUTTON_1
#define BUTTON_9H 2 // ??? CONFIG_IO_BUTTON_2

extern button_action_event_map_t common_button_action_event_maps[];
extern button_action_event_map_t normal_button_action_event_maps[];
extern button_action_event_map_t low_power_button_action_event_maps[];
extern button_action_event_map_t green_mode_button_action_event_maps[];
extern button_action_event_map_t photo_req_button_action_event_maps[];
extern button_action_event_map_t auth_ok_button_action_event_maps[];
extern button_action_event_map_t find_phone_button_action_event_maps[];
extern button_action_event_map_t reject_call_button_action_event_maps[];
extern button_action_event_map_t test_mode_button_action_event_maps[];


#endif // #if CONFIG_BUTTON_ENABLED


#endif /* !defined(__ASSEMBLER__) && !defined(__IAR_SYSTEMS_ASM__) */
#endif /* _C001S_CONFIG_H */
