
#include <stdint.h>             /* C 库头文件 */
#include <stdbool.h>
#include <string.h>
#include "sr3_config.h"       /* 资源配置、底层资源头文件 */
#include "nrf_assert.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"
#include "ble_gap.h"
#include "event_bus.h"

#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_link.h"
#include "includes.h"
#if CONFIG_RTT_PROTOCOL_INTERACT_ENABLED
#define Link_Print(M, ...)	SEGGER_Print("P_LINK", M, ##__VA_ARGS__)
#else
#define Link_Print(M, ...)
#endif

static bool m_link_init = false;

bool m_protocol_link_event_handler(const event_t * p_event)
{
    ASSERT(m_link_init);

/*
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

        case EVT_BUTTON_PRESSED:
            switch (p_event->button.button_event)
            {
            }
            break;
    }
*/
    return false;
}

bool m_protocol_link_prot_evt_execution(protocol_evt_t * p_evt)
{
    ASSERT(m_link_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_BLE_LINK)
    {
        return false; // 不属于 link 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // 默认返回找不到接口的帧错误

//    //Link_Print("<***** link evt_id 0x%04x  evt_len  0x%02x*****>", p_evt->header.evt_id ,p_evt->header.evt_len);

    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_SET_BLE_ADV_NAME:
        {          
            system_database.adv_name.name_flag = 0xABCD;
            system_database.adv_name.name_len = p_evt->header.evt_len % 12;           
            memcpy(&system_database.adv_name.name[0],(const uint8_t *)p_evt->evt_params.link.adv_name.adv_name_str,system_database.adv_name.name_len);               
            

            ble_gap_conn_sec_mode_t sec_mode;
            ret_code_t              status;

            BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
            
            status = sd_ble_gap_device_name_set(&sec_mode,
                                                (const uint8_t *)system_database.adv_name.name,
                                                system_database.adv_name.name_len);
            Link_Print("m_coms_ble: sd_ble_gap_device_name_set change name  status:%d ",status);
            //必须重新初始化广播才能改变蓝牙名                                                
            system_database.name_change_flag = true;                                     
            ack_state =  ACK_STATE_SUCCESS;          
        }
        break;
        
        case PROT_EVT_ID_IOS_PAIR_REQ: //ios 请求配对                         
             APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_IOS_PAIRING_REQ));
             ack_state = ACK_STATE_INVALID;
            break;
             
        case PROT_EVT_ID_SET_LINKLOSS_REMIND_SWITCH:
        {
            if (1 == p_evt->evt_params.link.linkloss_remind_enable.enabled)
            {
                system_database.linkloss_remind_enabled = 1;
            }
            else if (0 == p_evt->evt_params.link.linkloss_remind_enable.enabled)
            {
                system_database.linkloss_remind_enabled = 0;
            }
            else
            {
                ack_state = ACK_STATE_ERROR_INTF_PARAM;
                break;
            }

            APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_LINKLOSS_ENABLED_CONFIG));
            ack_state = ACK_STATE_SUCCESS;
            break;
        }

        case PROT_EVT_ID_GET_LINKLOSS_REMIND_SWITCH:
            p_evt->header.evt_id = PROT_EVT_ID_RET_LINKLOSS_REMIND_SWITCH;
            p_evt->header.evt_len = 1;
            p_evt->evt_params.link.linkloss_remind_enable.enabled = system_database.linkloss_remind_enabled;

            ack_state = ACK_STATE_INVALID;
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

uint32_t m_protocol_link_init(void)
{
    uint32_t status = NRF_SUCCESS;

    m_link_init = true;

    return status;
}


