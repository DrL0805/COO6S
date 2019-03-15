
#include <stdint.h>             /* C ��ͷ�ļ� */
#include <stdbool.h>

#include "sr3_config.h"       /* ��Դ���á��ײ���Դͷ�ļ� */
#include "nrf_assert.h"
#include "app_timer.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"

#include "event_bus.h"

#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_daily_sport.h"

#include "includes.h"

#if CONFIG_RTT_PROTOCOL_INTERACT_ENABLED
#define Daily_Print(M, ...)	SEGGER_Print("P_DAILY", M, ##__VA_ARGS__)
#else
#define Daily_Print(M, ...)
#endif

static bool m_daily_sport_init = false;
static bool m_is_ble_connected = false;
static uint8_t ds_mode =0;  //�ճ��˶�����

__STATIC_INLINE void send_total_steps(void)
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_DS_TOTAL_DAILY_STEPS;
    send_evt.header.evt_len = 2;
    send_evt.evt_params.scene_daily_sport.total_daily_steps = __REV16(system_database.steps_today);

    protocol_send_evt(send_evt);
}

bool m_protocol_daily_sport_event_handler(const event_t * p_event)
{
    ASSERT(m_daily_sport_init);

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
                ds_mode = 0;
            }
            break;

        case EVT_SYSTEM_STEPS_CHANGED:
            if(ds_mode)
            {
                send_total_steps();
            }
            break;
    }

    return false;
}

bool m_protocol_daily_sport_prot_evt_execution(protocol_evt_t * p_evt)
{
    ASSERT(m_daily_sport_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_DAILY_SPORT)
    {
        return false; // ������ daily_sport ��Э�飬�˳�
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // Ĭ�Ϸ����Ҳ����ӿڵ�֡����

    Daily_Print("<***** daily_sport evt_id 0x%04x *****>", p_evt->header.evt_id);

    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_GET_DS_TOTAL_DAILY_STEPS:
            p_evt->header.evt_id = PROT_EVT_ID_RET_DS_TOTAL_DAILY_STEPS;
            p_evt->header.evt_len = 2;
            p_evt->evt_params.scene_daily_sport.total_daily_steps = __REV16(system_database.steps_today);
            ack_state = ACK_STATE_INVALID;
            break;
        
        case PROT_EVT_ID_CALL_SCENE_DS_MODE:
            ds_mode = (p_evt->evt_params.scene_daily_sport.ds_mode.mode);
            Daily_Print("ds_mode: %d", ds_mode);
            ack_state = ACK_STATE_SUCCESS;
            break;
                

        case PROT_EVT_ID_SET_DS_STEPS_GOAL:
            if(system_database.steps_goal_today < __REV16(p_evt->evt_params.scene_daily_sport.ds_goal.set_goal))
            {               
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_DAILY_SPORT_STEPS_GOAL));
            }
            system_database.steps_goal_today = __REV16(p_evt->evt_params.scene_daily_sport.ds_goal.set_goal);
            Daily_Print("Set steps goal: %d", system_database.steps_goal_today);
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_GET_DS_STEPS_GOAL:
            p_evt->header.evt_id = PROT_EVT_ID_RET_DS_STEPS_GOAL;
            p_evt->header.evt_len = 2;
            p_evt->evt_params.scene_daily_sport.ds_goal.set_goal = __REV16(system_database.steps_goal_today);
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

uint32_t m_protocol_daily_sport_init(void)
{
    uint32_t status = NRF_SUCCESS;

    m_daily_sport_init = true;

    return status;
}


