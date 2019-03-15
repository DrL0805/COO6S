

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "app_debug.h"

#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "app_timer.h"
#include "app_uart.h"

#include "m_uart.h"
#include "m_leds.h"
#include "test_manuf.h"

#include "includes.h"

#if 1//CONFIG_RTT_VIBRATOR_ENABLED
#define UART_Print(M, ...)	SEGGER_Print("UART", M, ##__VA_ARGS__)
#else
#define UART_Print(M, ...)
#endif


typedef struct{
    uint8_t rx_ing_flag;
    uint8_t rx_len;
    uint8_t tx_len;
    uint8_t rx_buf[100];
    uint8_t tx_buf[100];
}m_uart_data_t;  

APP_TIMER_DEF(uart_recv_timeout_timer_id);
//APP_TIMER_DEF(test_timer_id);


m_uart_data_t m_uart_data;

//static void app_uart_event_handler(app_uart_evt_t * p_app_uart_event)
//{
//    if (APP_UART_TX_EMPTY == p_app_uart_event->evt_type)
//    {
//        if (m_handler)
//        {
//            m_handler(UART_EVT_SEND_OK);
//        }
//    }
//    else if (APP_UART_DATA == p_app_uart_event->evt_type)
//    {
//        if (m_handler)
//        {
//            m_handler(UART_EVT_RECV_OK);
//        }
//    }
//    
//}

/*接收超时，判断为当次接收已经完成*/
static void uart_recv_timer_timeout_handler(void * p_context)
{  
   UART_Print("m_uart_data.rx_len %02x",m_uart_data.rx_len); 
   m_uart_data.rx_ing_flag =false;    
   
   if(m_uart_data.rx_len != 0x1E)	
    {
        APP_ERROR_CHECK(TPDU_ProtocolResolver(&m_uart_data.rx_buf[0], m_uart_data.rx_len));
    }
    m_uart_data.rx_len =0;
//    else
//    {
//        TPDU_SNBurnHandler(m_uart_data.rx_len, &m_uart_data.rx_buf[0]);
//    }
}
/*
#include "m_drv_led.h"
static void test_timer_timeout_handler(void * p_context)
{  
    static uint8_t flag = 1;
    if(flag)
    {
        led_on(0);
    }
    else
    {
        led_off(0);
    }
    flag =!flag;
}
*/



uint32_t restart_uart_timeout_handler(void)
{
    uint32_t err_code =0;
    err_code = app_timer_stop(uart_recv_timeout_timer_id);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return app_timer_start(uart_recv_timeout_timer_id,  APP_TIMER_TICKS(20), NULL);
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to
 *          a string. The string will be be sent over BLE when the last character received was a
 *          'new line' i.e '\r\n' (hex 0x0D) or if the string has reached a length of
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{

    switch (p_event->evt_type)
    {
        /*每个字节都会触发*/
        case APP_UART_DATA_READY:
//            DBG1_EVT0("APP_UART_DATA_READY");
            if( m_uart_data.rx_ing_flag) /*已经在接收，重启动超时定时器*/
            {
                APP_ERROR_CHECK(restart_uart_timeout_handler());
            }
            else /*串口刚开始接收当前帧数据*/
            {
                m_uart_data.rx_len = 0;
                m_uart_data.rx_ing_flag =true;
                APP_ERROR_CHECK(app_timer_start(uart_recv_timeout_timer_id,  APP_TIMER_TICKS(20), NULL));
            }
            
            UNUSED_VARIABLE(app_uart_get(&m_uart_data.rx_buf[m_uart_data.rx_len]));
            m_uart_data.rx_len++;
            break;
            
        case APP_UART_TX_EMPTY:
            UART_Print("APP_UART_TX_EMPTY");

        break;

        case APP_UART_COMMUNICATION_ERROR:
//            app_uart_close();
//            nrf_delay_ms(20);
//            drv_uart_sps_init(uart_event_handle);
//            DBG1_EVT1("APP_UART_COMMUNICATION_ERROR %d",p_event->data.error_communication);
//            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            UART_Print("APP_UART_FIFO_ERROR");
//            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}


void m_uart_get_string(void)
{
	uint8_t TmpTxByte;
    
	UART_Print("REG : %08x %08x %08x %08x\r\n",NRF_UARTE0->PSEL.RXD,NRF_UARTE0->EVENTS_RXDRDY,NRF_UART0->RXD,NRF_UART0->EVENTS_RXDRDY);
    
	if(app_uart_get(&TmpTxByte) == NRF_SUCCESS)
	{
        
		if( m_uart_data.rx_ing_flag)
		{			
			APP_ERROR_CHECK(restart_uart_timeout_handler());
		}
		else
		{
            m_uart_data.rx_len = 0;
            m_uart_data.rx_ing_flag =true;
            APP_ERROR_CHECK(app_timer_start(uart_recv_timeout_timer_id,  APP_TIMER_TICKS(20), NULL));
		}
        UART_Print("app_uart_get %02x cnt %d",TmpTxByte,m_uart_data.rx_len);
        m_uart_data.rx_buf[m_uart_data.rx_len] = TmpTxByte;
        m_uart_data.rx_len++;
	}    

}

uint32_t m_uart_init(void)
{
    uint32_t err_code = NRF_SUCCESS;
    
    err_code = app_timer_create(&uart_recv_timeout_timer_id,APP_TIMER_MODE_SINGLE_SHOT, uart_recv_timer_timeout_handler);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }
    return drv_uart_sps_init(uart_event_handle);
}






