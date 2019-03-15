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
#ifndef __M_WATCH_HAND_H__
#define __M_WATCH_HAND_H__

#include <stdbool.h>
#include <stdint.h>

#include "event_bus.h"


bool m_watch_hand_event_handler(const event_t * p_evt);


uint32_t m_watch_hand_init(void);


#endif /* __M_WATCH_HAND_H__ */

