#ifndef _DRV_UART_H__
#define _DRV_UART_H__

#include "stdint.h"
#include "app_uart.h"

typedef enum
{
    UART_EVT_RECV_OK,
    UART_EVT_SEND_OK,
}uart_evt_t;

typedef void (*uart_evt_handler_t)(uart_evt_t evt);

void uart_sps_send_data(uint8_t * p_data, uint16_t length);

uint32_t drv_uart_sps_init(app_uart_event_handler_t handler);

void uart_disable(void);


#endif /* TEST_MANUF_COMS_H__ */


