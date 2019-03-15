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
// ��������:   	ADC��ʼ��
// ��������:    ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t m_batt_meas_init(void);

//************************************************************************************************************
// ��������:   	ע��ADC����
// ��������:    ������
// ����ֵ  :	��        
//************************************************************************************************************
ret_code_t m_batt_meas_uninit(void);

//************************************************************************************************************
// ��������:   	��������ģ��������¼���ժȡ�Լ����ĵ��¼�����
// ��������:    p_event		->	�¼�����
// ����ֵ  :	event_bus�����¼��Ƿ������һ���ص�����(true: �ص��¼���ֹ, false: ������һ���ص��¼�)        
//************************************************************************************************************
bool m_batt_meas_event_handler(const event_t * p_event);

#endif /* __M_BATT_MEAS_H__ */
/** @} */

