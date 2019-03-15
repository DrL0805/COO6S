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
#ifndef __M_GSENSOR_H__
#define __M_GSENSOR_H__

#include <stdbool.h>
#include <stdint.h>

#include "event_bus.h"
//#include "m_pwr_mgmt.h"

uint32_t m_gsensor_init(void);

bool m_gsensor_event_handler(const event_t *p_event);

//void step_update_sched_event_handler(void);

//bool m_gsensor_wakeup_prepare(m_pwr_mgmt_shutdown_type_t shutdown_type);

#endif /* __M_GSENSOR_H__ */

