
#include <stdint.h>             /* C 库头文件 */
#include <stdbool.h>

#include "nrf_log.h"

#include "sr3_config.h"       /* 资源配置、底层资源头文件 */
#include "nrf_assert.h"
#include "app_timer.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"
#include "AD_DataStore.h"

#include "app_isched.h"
#include "resources.h"

#include "event_bus.h"
#include "nrf_pwr_mgmt.h"         /* 模块头文件 */
#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_sensor_trans.h"
//#include "app_debug_event_logger.h"

#include "includes.h"

#if CONFIG_RTT_PROTOCOL_SENSOR_TRANS_ENABLED
#define SP_Print(M, ...)	SEGGER_Print("SENSOR_UPLOAD", M, ##__VA_ARGS__)
#else
#define SP_Print(M, ...)
#endif


static bool m_is_ble_connected = false;
static bool m_sensor_trans_init = false;


/*
*	红外温度检测完成后，上传检测值
*/
__STATIC_INLINE void send_ir_temp(uint16_t temp)
{
	if (!m_is_ble_connected) return;
	
	protocol_evt_t	send_evt;
	
	SP_Print("ir send: %d:0x%.4x\n\r", temp, temp);
	
	send_evt.header.evt_id = PROT_EVT_ID_ECHO_SENSOR_IR_TEMP;
    send_evt.header.evt_len = 2;
    send_evt.evt_params.sensor_trans.ir_data.temp = __REV16(temp);

    protocol_send_evt(send_evt);
	
}

__STATIC_INLINE void send_body_temp(uint16_t temp)
{
	if (!m_is_ble_connected) return;
	
	protocol_evt_t	send_evt;
	
	SP_Print("body send: %d:0x%.4x\n\r", temp, temp);
	
	send_evt.header.evt_id = PROT_EVT_ID_ECHO_SENSOR_BODY_TEMP;
    send_evt.header.evt_len = 2;
    send_evt.evt_params.sensor_trans.body_data.temp = __REV16(temp);

    protocol_send_evt(send_evt);
	
}

__STATIC_INLINE void send_pressure(uint32_t data)
{
	if (!m_is_ble_connected) return;
	
	protocol_evt_t	send_evt;
    
    uint16_t    val;
	
	SP_Print("ir send: %d:0x%.4x\n\r", data, data);
	
	send_evt.header.evt_id = PROT_EVT_ID_ECHO_SENSOR_PRESSURE;
    send_evt.header.evt_len = 4;
    
    val = data & 0xFFFF; 
    send_evt.evt_params.sensor_trans.pressure_data.pressure.temp  = __REV16(val);
    val = data >> 16;
    send_evt.evt_params.sensor_trans.pressure_data.pressure.press = __REV16(val);

    protocol_send_evt(send_evt);
	
}

bool m_protocol_sensor_trans_prot_evt_execution(protocol_evt_t * p_evt)
{
    ASSERT(m_sensor_trans_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_SENSOR_UPLOAD)
    {
        return false; // 不属于 pack_trans 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // 默认返回找不到接口的帧错误
//    uint16_t data_type;
//    uint16_t pack_index;
//    uint16_t content_count;
//    uint16_t content_index;
//    uint32_t all_data_len;
//    uint32_t utc_second;

    SP_Print("<***** pack_trans evt_id 0x%04x *****>", p_evt->header.evt_id);

    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_GET_CONTENT_COUNT:  //02 f0 10

            ack_state = ACK_STATE_INVALID;
            break;

        case PROT_EVT_ID_GET_CONTENT_INFO:

            break;

        case PROT_EVT_ID_GET_PKG:  
        {
            
            break;
        }

        case PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_COUNT:

            break;

        case PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_INFO:

            break;

        case PROT_EVT_ID_CALL_DEL_ALL:
           

            break;

        case PROT_EVT_ID_CALL_DEL_DATATYPE:
            

            break;

        case PROT_EVT_ID_CALL_DEL_CONTENT_UTC:
        {
            

            break;
        }
		

		case PROT_EVT_ID_ECHO_SENSOR_IR_TEMP:
			break;
		
		case PROT_EVT_ID_GET_SENSOR_BODY_TEMP:
			APP_ERROR_CHECK(event_send(EVT_BODY_STATE, BODY_EVT_CHECK));
			SP_Print("PROT_EVT_ID_GET_SENSOR_BODY_TEMP\n\r");
			ack_state = ACK_STATE_SUCCESS;
			break;
		
		
		case PROT_EVT_ID_GET_SENSOR_PRESSURE:
			APP_ERROR_CHECK(event_send(EVT_PRESSURE_STATE, PRESSURE_EVT_CHECK));
			SP_Print("PROT_EVT_ID_GET_SENSOR_PRESSURE\n\r");
			ack_state = ACK_STATE_SUCCESS;
			break;

        default:
            break;
    }

    if (ack_state != ACK_STATE_INVALID)
    {
        p_evt->header.evt_id = PROT_EVT_ID_ACK;
        p_evt->evt_params.ack.state = ack_state;
    }

    return true;
}

uint32_t m_protocol_sensor_trans_init(void)
{
    uint32_t status = NRF_SUCCESS;


    m_sensor_trans_init = true;

    return status;
}

bool m_protocol_sensor_trans_event_handler(const event_t * p_event)
{
    ASSERT(m_sensor_trans_init);


    switch (p_event->type)
    {
        case EVT_SYSTEM_BT_CONN_STATE:
            if (p_event->system.ble_conn_state >= BT_CONN_STATE_CONNECTED)
            {
                m_is_ble_connected = true;                
            }
            else
            {
                m_is_ble_connected = false;
            }
            break;
			
		case EVT_IR_STATE:
			SP_Print("aaaabbb****###\n\r");
			if (IR_EVT_FINISH == p_event->ir.ir_event)
			{
				send_ir_temp(p_event->ir.data);
			}
			break;
			
		case EVT_BODY_STATE:
			if (BODY_EVT_RESULT == p_event->body.body_event)
			{
				send_body_temp(p_event->body.data);
			}
			break;
			
		case EVT_PRESSURE_STATE:
			if (PRESSURE_EVT_RESULT == p_event->pressure.pressure_event)
			{
				send_pressure(p_event->pressure.data);
			}
			break;
    }
    return false;
}


