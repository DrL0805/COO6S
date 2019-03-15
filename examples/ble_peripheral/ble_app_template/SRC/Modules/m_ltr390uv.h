/**
 * @defgroup MOD_LEDS LED module
 * @ingroup output
 * @{
 * @brief LED module
 *
 * @details This module deals with the LEDs.
 */
#ifndef __M_UV_H__
#define __M_UV_H__

#include <stdbool.h>
#include <stdint.h>

#include "event_bus.h"


uint32_t m_uv_init(void);
uint32_t m_uv_uninit(void);
bool m_uv_event_handler(const event_t *p_event);


#endif
