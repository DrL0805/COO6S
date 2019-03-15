/**
 * @defgroup MOD_LEDS LED module
 * @ingroup output
 * @{
 * @brief LED module
 *
 * @details This module deals with the LEDs.
 */
#ifndef __M_UART_H__
#define __M_UART_H__

#include <stdbool.h>
#include <stdint.h>

#include "event_bus.h"
#include "drv_uart.h"







void m_uart_get_string(void);

uint32_t m_uart_init(void);
uint32_t m_uart_uninit(void);
bool m_uart_event_handler(const event_t *p_event);


#endif
