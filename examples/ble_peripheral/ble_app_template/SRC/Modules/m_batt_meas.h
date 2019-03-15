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
 * @defgroup MOD_BATT_MEAS Battery monitoring module
 * @ingroup input
 * @{
 * @brief Functions for configuring the battery monitoring module.
 */
#ifndef __M_BATT_MEAS_H__
#define __M_BATT_MEAS_H__

#include <stdint.h>
#include "nrf_pwr_mgmt.h"
#include "event_bus.h"

//************************************************************************************************************
// 功能描述:   	ADC初始化
// 函数参数:    无
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t m_batt_meas_init(void);

//************************************************************************************************************
// 功能描述:   	注销ADC功能
// 函数参数:    无意义
// 返回值  :	无        
//************************************************************************************************************
ret_code_t m_batt_meas_uninit(void);

//************************************************************************************************************
// 功能描述:   	接收其它模块产生的事件，摘取自己关心的事件处理
// 函数参数:    p_event		->	事件类型
// 返回值  :	event_bus调度事件是否继续下一个回调任务(true: 回调事件终止, false: 继续下一个回调事件)        
//************************************************************************************************************
bool m_batt_meas_event_handler(const event_t * p_event);

#endif /* __M_BATT_MEAS_H__ */
/** @} */

