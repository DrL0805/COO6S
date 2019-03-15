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

/**
 * @defgroup MOD_LEDS LED module
 * @ingroup output
 * @{
 * @brief LED module
 *
 * @details This module deals with the LEDs.
 */
#ifndef __M_LEDS_H__
#define __M_LEDS_H__

#include <stdbool.h>
#include <stdint.h>

#include "event_bus.h"
#include "nrf_pwr_mgmt.h"

#include "m_drv_led.h"

#if CONFIG_RTT_DEBUG_ENABLED
#define LED_12H_RED         1
#define LED_12H_GREEN       0
#else
#define LED_12H_RED         0
#define LED_12H_GREEN       1
#endif

#define LED_INVALID         0xFF


//************************************************************************************************************
// 功能描述:   	初始化LED配置及注册通知回调函数
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
uint32_t m_leds_init(void);

//************************************************************************************************************
// 功能描述:   	处理来自其他事件的自己关心的事件
// 函数参数:    p_event		->		事件类型
// 返回值  :	event_bus调度事件是否继续下一个回调任务(true: 回调事件终止, false: 继续下一个回调事件) 
//************************************************************************************************************
bool m_leds_event_handler(const event_t * p_event);



#endif /* __M_LEDS_H__ */
/** @} */
