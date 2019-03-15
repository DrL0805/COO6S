
/**
 * @defgroup MOD_LEDS LED module
 * @ingroup output
 * @{
 * @brief LED module
 *
 * @details This module deals with the LEDs.
 */
#ifndef __M_VIBRATOR_H__
#define __M_VIBRATOR_H__

#include <stdbool.h>
#include <stdint.h>

#include "event_bus.h"


ret_code_t m_vibrator_init(void);

ret_code_t m_vibrator_enable(void);

ret_code_t m_vibrator_disable(void);

bool m_vibrator_event_handler(const event_t * p_evt);


#endif /* __M_VIBRATOR_H__ */


