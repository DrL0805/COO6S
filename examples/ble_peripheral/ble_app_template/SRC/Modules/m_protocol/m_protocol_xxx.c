
#include <stdint.h>             /* C 库头文件 */
#include <stdbool.h>

#include "c001s_config.h"       /* 资源配置、底层资源头文件 */
#include "nrf_assert.h"
#include "app_timer.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"

#include "event_bus.h"
#include "m_pwr_mgmt.h"         /* 模块头文件 */
#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_xxx.h"


APP_TIMER_DEF(x_timeout_timer_id);

static bool m_xxx_init = false;
static bool m_is_ble_connected = false;


__STATIC_INLINE void send_x(void)
{
/*
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_X;
    send_evt.header.evt_len = x;
    send_evt.evt_params.xxx = x;

    protocol_send_evt(send_evt);
*/
}

static void x_timeout_timer_handler(void * p_context)
{

}

bool m_protocol_xxx_event_handler(const event_t * p_event)
{
    ASSERT(m_xxx_init);

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

bool m_protocol_xxx_prot_evt_execution(protocol_evt_t * p_evt)
{
    ASSERT(m_xxx_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_XXX)
    {
        return false; // 不属于 xxx 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // 默认返回找不到接口的帧错误
/*
    DBG0_EVT1("xxx evt_id 0x%04x", p_evt->header.evt_id);

    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_CALL_X:
        {
            if (p_evt->evt_params.xxx.x == x)
            {
                
            }
        
            ack_state = ACK_STATE_SUCCESS;
            break;
        }

        case PROT_EVT_ID_SET_X:
        {
            x = p_evt->evt_params.xxx.x;

            ystem_runtime_database.x = x;

            ack_state = ACK_STATE_SUCCESS;
            break;
        }

        case PROT_EVT_ID_GET_X:
            p_evt->header.evt_id = PROT_EVT_ID_RET_X;
            p_evt->header.evt_len = x;
            p_evt->evt_params.xxx.x = system_database.x;

            ack_state = ACK_STATE_INVALID;
            break;

        default:
            break;
    }
*/

    if (ack_state != ACK_STATE_INVALID)
    {
        p_evt->header.evt_id = PROT_EVT_ID_ACK;
        p_evt->evt_params.ack.state = ack_state;
    }

    return true;
}

uint32_t m_protocol_xxx_init(void)
{
    uint32_t status = NRF_SUCCESS;

    status = app_timer_create(&x_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, x_timeout_timer_handler);

    m_xxx_init = true;

    return status;
}


