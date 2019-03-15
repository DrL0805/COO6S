

#include "sdk_common.h"
#include "app_uart.h"
#include "drv_uart.h"
#include "app_debug.h"
#include "nrf_log.h"

#define RX_PIN_NUMBER                   22
#define TX_PIN_NUMBER                   23
#define CTS_PIN_NUMBER                  0xffffffff
#define RTS_PIN_NUMBER                  0xffffffff
#define HWFC                            false

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
#define BLE_NUS_MAX_DATA_LEN            20

//static uart_evt_handler_t m_handler = NULL;




static uint32_t uart_init(app_uart_event_handler_t handler)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200,
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       handler,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    return (err_code);
}

static void uart_stop(void)
{
    app_uart_close();
}

static void uart_data_send(uint8_t * p_data, uint16_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        while (app_uart_put(p_data[i]) != NRF_SUCCESS);
    }
}

void uart_sps_send_data(uint8_t * p_data, uint16_t length)
{  
    NRF_LOG_RAW_INFO("send "); 
    for (uint8_t i = 0; i < length; i++)
    {
        NRF_LOG_RAW_INFO(" %02x",p_data[i]); 
    }
    NRF_LOG_RAW_INFO(" \r\n"); 
    uart_data_send(p_data, length);
}

uint32_t drv_uart_sps_init(app_uart_event_handler_t handler)
{
//    m_handler = handler;
    return uart_init(handler);
}

void uart_disable(void)
{
    uart_stop();
}



