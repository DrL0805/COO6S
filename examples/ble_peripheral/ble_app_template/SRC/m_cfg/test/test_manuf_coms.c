

#include "sdk_common.h"
#include "app_uart.h"
#include "test_manuf_coms.h"
#include "app_debug.h"
#define RX_PIN_NUMBER                   22
#define TX_PIN_NUMBER                   23
#define CTS_PIN_NUMBER                  0xffffffff
#define RTS_PIN_NUMBER                  0xffffffff
#define HWFC                            false

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

static tmc_evt_handler_t m_handler = NULL;

static void app_uart_event_handler(app_uart_evt_t * p_app_uart_event)
{
    if (APP_UART_TX_EMPTY == p_app_uart_event->evt_type)
    {
        if (m_handler)
        {
            m_handler(TMC_EVT_SEND_OK);
        }
    }
    else if (APP_UART_COMMUNICATION_ERROR == p_app_uart_event->evt_type)
    {
        DBG1_EVT0("APP_UART_COMMUNICATION_ERROR");
//            APP_ERROR_HANDLER(p_event->data.error_communication);
    }
}

/*
static uint32_t uart_init(void)
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
        UART_BAUDRATE_BAUDRATE_Baud57600,
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       app_uart_event_handler,
                       APP_IRQ_PRIORITY_LOWEST,
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

void tmc_send_command(uint8_t * p_data, uint16_t length)
{
    uart_data_send(p_data, length);
}

uint32_t tmc_init(tmc_evt_handler_t handler)
{
    m_handler = handler;
    return uart_init();
}

void tmc_disable(void)
{
    uart_stop();
}
*/


