#include <stdint.h>
#include <stdbool.h>

#include "app_timer.h"
#include "app_debug.h"
#include "app_error.h"

#include "m_system_state.h"
#include "protocol.h"
#include "resources.h"
#include "sr3_config.h"
#include "m_protocol.h"
#include "m_protocol_interact.h"
#include "m_database.h"
#include "m_ble_ancs_c.h"
#include "app_time_slot.h"

#include "includes.h"

#if CONFIG_RTT_PROTOCOL_INTERACT_ENABLED
#define P_Interact_Print(M, ...)	SEGGER_Print("P_INTERACT", M, ##__VA_ARGS__)
#else
#define P_Interact_Print(M, ...)
#endif




#define TEST_FRANKLINISM 0                          // 静电测试所用，每秒发送一条遥控拍照指令

static bool m_is_ble_connected = false;
static bool m_photo_mode_enter = false;             // 拍照请求标志
static bool m_watch_hand_mode_enter = false;        // 拍照请求标志
static bool m_is_linkloss_timer_start = false;      // 断开计时开始，超时发送提醒，重连停止计时
//bool system_database.dnd_cur_state = false;                    // 进入勿扰模式

APP_TIMER_DEF(mode_timeout_timer_id);         // 用于模式超时退出，如拍照模式，校针模式等
APP_TIMER_DEF(linkloss_timeout_timer_id);     // 用于断开时计时，超时发送提醒

__STATIC_INLINE bool is_mode_idle(void)
{
    uint8_t mode_enter_cnt = m_photo_mode_enter + m_watch_hand_mode_enter;

    ASSERT(mode_enter_cnt < 2);

    return (mode_enter_cnt) ? false : true;
}

__STATIC_INLINE void clear_all_mode(void)
{
//    //DBG0_EVT0("clear_all_mode");
    if (m_photo_mode_enter)
    {
        m_photo_mode_enter = false;
        APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_PHOTO_EXIT_MODE));
    }
    else if (m_watch_hand_mode_enter)
    {
        m_watch_hand_mode_enter = false;
        APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_WATCH_HAND_EXIT_MODE));
    }
}

__STATIC_INLINE void send_photo_req(void)
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_CALL_PHOTO_REQ;
    send_evt.header.evt_len = 0;

    protocol_send_evt(send_evt);
}

__STATIC_INLINE void send_findphone_req(void)
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_CALL_FIND_PHONE;
    send_evt.header.evt_len = 0;

    protocol_send_evt(send_evt);
}

__STATIC_INLINE void send_reject_callin(void)
{
    if (!m_is_ble_connected) return;
    //DBG1_EVT0("send_reject_callin.");
    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_CALL_INCALL_RESP;
    send_evt.header.evt_len = 1;
    send_evt.evt_params.interact.incall_resp.cmd = 0; // 拒接

    protocol_send_evt(send_evt);
}

__STATIC_INLINE void send_accept_callin(void)
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_CALL_INCALL_RESP;
    send_evt.header.evt_len = 1;
    send_evt.evt_params.interact.incall_resp.cmd = 1; // 接听

    protocol_send_evt(send_evt);
}


static void enter_green_mode_sched_handler(void * p_context)
{
    uint8_t mode = *(uint8_t *)p_context;
    if(mode == 0x02)
    {
        APP_ERROR_CHECK(event_send(EVT_SYSTEM_STATE, SYSTEM_STATE_GREEN_MODE));
    }
    else if(mode == 0x01)
    {
        APP_ERROR_CHECK(event_send(EVT_SYSTEM_STATE, SYSTEM_STATE_SAVE_POWER_MODE));
    }
    
    
}


static void mode_timeout_timer_handler(void * p_context)
{
//    //DBG0_EVT0("mode_timeout_timer_handler");
    if (is_mode_idle() == false) // 已经超时了，仍有模式没退出
    {
        clear_all_mode();
    }
}

static void linkloss_timeout_timer_handler(void * p_context)
{
    //DBG0_EVT0("linkloss_timeout_timer_handler");
    m_is_linkloss_timer_start = false;
    if (!system_database.dnd_cur_state)
    {
//        //DBG0_EVT0("Linkloss time out");
        APP_ERROR_CHECK(event_send(EVT_COMS_PROT_EXEC, PROT_EXEC_EVT_LINKLOSS_TRIGGERED));
    }
}

static void linkloss_reminder_process(uint8_t ble_conn_state)
{
    if (ble_conn_state >= BT_CONN_STATE_CONNECTED)
    {
        if (m_is_linkloss_timer_start)
        {
            APP_ERROR_CHECK(app_timer_stop(linkloss_timeout_timer_id));
            m_is_linkloss_timer_start = false;
        }
    }
    else if (ble_conn_state == BT_CONN_STATE_DISCONNECTED)
    {
        if ((!m_is_linkloss_timer_start) && (system_database.linkloss_remind_enabled))
        {
            m_is_linkloss_timer_start = true;
            APP_ERROR_CHECK(app_timer_start(linkloss_timeout_timer_id,
                                            APP_TIMER_TICKS(CONFIG_LINKLOSS_REMINDER_TIMEOUT * 1000),
                                            NULL));
        }
    }
}

static void do_not_disturb_state_proecess(void)
{
    if (!system_database.dnd_state_enabled)
    {
        system_database.dnd_cur_state = false;
        return;
    }

    time_slot_minutes_t time_slot;
    uint16_t            cur_minutes = TIME_SLOT_MINUTES(system_database.utc.hour, system_database.utc.minute);

    time_slot.begin     = 0;
    time_slot.end       = TIME_SLOT_MINUTES(23, 59);
    time_slot.dnd_begin = TIME_SLOT_MINUTES(system_database.dnd_start_hour, system_database.dnd_start_minute);
    time_slot.dnd_end   = TIME_SLOT_MINUTES(system_database.dnd_end_hour, system_database.dnd_end_minute);

    P_Interact_Print("cur minute: %d", cur_minutes);
    P_Interact_Print("       dnd: %d - %d", time_slot.dnd_begin, time_slot.dnd_end);

    if (time_slot_is_minute_time_valid(time_slot, cur_minutes)) // 不在勿扰时间段内
    {
        //DBG1_EVT0("Not in dnd state");
        system_database.dnd_cur_state = false;
    }
    else
    {
        //DBG1_EVT0("In dnd state");
        system_database.dnd_cur_state = true;
    }
}


bool m_protocol_ineract_event_handler(const event_t * p_event)
{
    #if 1
    switch (p_event->type)
    {
    #if TEST_FRANKLINISM
        case EVT_SYSTEM_UTC_SECOND:
            if (m_is_ble_connected)
            {
                send_photo_req();
            }
            break;
    #endif // TEST_FRANKLINISM

        case EVT_SYSTEM_UTC_MINUTE:
            do_not_disturb_state_proecess();
            break;

        case EVT_SYSTEM_BT_CONN_STATE:
            if (p_event->system.ble_conn_state >= BT_CONN_STATE_CONNECTED)
            {
                m_is_ble_connected = true;
            }
            else
            {
                m_is_ble_connected = false;
            }

            linkloss_reminder_process(p_event->system.ble_conn_state);
            break;

        case EVT_BUTTON_PRESSED:
            switch (p_event->button.button_event)
            {
                case BUTTON_EVT_PHOTO_REQ:
                    if (m_photo_mode_enter)
                    {
                        
                        send_photo_req();
                                                
                    }
                    break;

                case BUTTON_EVT_FIND_PHONE:
                    send_findphone_req();
                    break;

                case BUTTON_EVT_REJECT_CALLIN:
                    #if CONFIG_ANCS_C_ENABLED
                    m_coms_ble_ancs_c_reject_callin();
                    
                    send_reject_callin();
                    
                    #endif
                    
                    break;

                case BUTTON_EVT_ACCEPT_CALLIN:
                    send_accept_callin();
                    break;
            }
            break;
    }
    #endif
    
    return false;
}

bool m_protocol_interact_prot_evt_execution(protocol_evt_t * p_evt)
{
    
    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_INTERACTION)
    {
        return false; // 不属于 interact 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF;

    P_Interact_Print("<***** interact evt_id 0x%04x *****>", p_evt->header.evt_id);

    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_CALL_NEW_MSG:
        {
            P_Interact_Print("<***** PROT_EVT_ID_CALL_NEW_MSG *****>");
            uint32_t new_msg = 0;

            new_msg = __REV(p_evt->evt_params.interact.new_msg.msg_bitmap);

            if ((new_msg & system_database.msg_switch_bitmap) != 0)
            {
                system_database.new_msg_bitmap = new_msg;
                if ((!system_database.dnd_cur_state)&&(!m_photo_mode_enter))
                {
                    APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_MSG_NEW));
                }
                else
                {
//                    //DBG0_EVT0("In do not disturb state or photo mode, not sending new msg");
                }
            }
            ack_state = ACK_STATE_SUCCESS;
            break;
        }
    
        case PROT_EVT_ID_SET_MSG_SWTICH:
            system_database.msg_switch_bitmap = __REV(p_evt->evt_params.interact.new_msg.msg_bitmap);
            P_Interact_Print("recv.msg_switch_bitmap: %04x",system_database.msg_switch_bitmap);
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_GET_MSG_SWTICH:
            p_evt->header.evt_id = PROT_EVT_ID_RET_MSG_SWTICH;
            p_evt->header.evt_len = 4;
            p_evt->evt_params.interact.msg_switch.msg_switch_bitmap = __REV(system_database.msg_switch_bitmap);
            ack_state =  ACK_STATE_INVALID;
            break;

        case PROT_EVT_ID_CALL_CANCEL_MSG:
        {
            uint32_t cancel_msg = 0;
        
            cancel_msg = __REV(p_evt->evt_params.interact.cancel_msg.msg_bitmap);
        
            if ((cancel_msg & system_database.msg_switch_bitmap) != 0)
            {
                system_database.cancel_msg_bitmap = cancel_msg;
                if (!system_database.dnd_cur_state)
                {
                    APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_MSG_CANCEL));
                }
                else
                {
//                    //DBG0_EVT0("In do not disturb state, not sending cancel msg");
                }
            }
            ack_state = ACK_STATE_SUCCESS;
            break;
        }

        case PROT_EVT_ID_CALL_PHOTO_MODE:
            if (p_evt->evt_params.interact.photo_mode.call_enter_exit == 1)
            {
                while(1);
                if (is_mode_idle())
                {
                    m_photo_mode_enter = true;
                    APP_ERROR_CHECK(app_timer_start(mode_timeout_timer_id,
                                    APP_TIMER_TICKS(CONFIG_MODE_TIMEOUT * 1000),
                                    NULL));
                }
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_PHOTO_ENTER_MODE));
            }
            else
            {
                m_photo_mode_enter = false;
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_PHOTO_EXIT_MODE));
                APP_ERROR_CHECK(app_timer_stop(mode_timeout_timer_id));
            }
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_ECHO_PHOTO_HEARTBEAT:
            APP_ERROR_CHECK(app_timer_stop(mode_timeout_timer_id));
            APP_ERROR_CHECK(app_timer_start(mode_timeout_timer_id,
                            APP_TIMER_TICKS(30 * 1000),
                            NULL));
            APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_PHOTO_MODE_HEART));
            p_evt->header.evt_id = PROT_EVT_ID_INVALID;
            ack_state = ACK_STATE_INVALID;
            break;

        case PROT_EVT_ID_CALL_WATCH_HAND_MODE:
            if (p_evt->evt_params.interact.watch_hand_mode.enter_exit == 1)
            {
                if (is_mode_idle())
                {
                    m_watch_hand_mode_enter = true;
                    APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_WATCH_HAND_ENTER_MODE));
                    APP_ERROR_CHECK(app_timer_start(mode_timeout_timer_id,
                                    APP_TIMER_TICKS(CONFIG_MODE_TIMEOUT * 1000),
                                    NULL));
                }
            }
            else
            {
                m_watch_hand_mode_enter = false;
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_WATCH_HAND_EXIT_MODE));
                APP_ERROR_CHECK(app_timer_stop(mode_timeout_timer_id));
            }
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_CALL_WATCH_HAND_CTRL:
        {
            uint8_t cmd = p_evt->evt_params.interact.watch_hand_ctrl.cmd;
            uint16_t event = APP_RECV_EVT_INVALID;

            if (!m_watch_hand_mode_enter) break;

            switch (cmd) // 0 恢复行针, 1 暂停行针, 2 指针正转 1 步, 3 指针反转 1 步, 4 使指针进入校针状态
            {
                case 0: event = APP_RECV_EVT_WATCH_HAND_RESUME;         break;
                case 1: event = APP_RECV_EVT_WATCH_HAND_STOP;           break;
                case 2: event = APP_RECV_EVT_WATCH_HAND_STEP_FORWARD;   break;
                case 3: event = APP_RECV_EVT_WATCH_HAND_STEP_BACKWARD;  break;
                case 4: event = APP_RECV_EVT_WATCH_HAND_CHECK;          break;
            }

            
            if (event != APP_RECV_EVT_INVALID)
            {
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, event));
            }
            ack_state = ACK_STATE_SUCCESS;
            break;
        }
            
        case PROT_EVT_ID_SET_WATCH_HAND_PARAM:
        {
            uint8_t watch_hand_id = p_evt->evt_params.interact.watch_hand_param.watch_hand_id;
            uint16_t pos = __REV16(p_evt->evt_params.interact.watch_hand_param.watch_hand_pos);
            P_Interact_Print("pos: %d, id : %d", pos,watch_hand_id);
            
            if (watch_hand_id == 1)
            {
                system_database.watch_hand_1_pos = pos / 10;
                P_Interact_Print("watch_hand_0_pos: %d ,watch_hand_1_pos: %d", system_database.watch_hand_0_pos,system_database.watch_hand_1_pos);

            }
            else if (watch_hand_id == 27)   
            {
                pos = pos % 4320;
                system_database.watch_hand_1_pos = pos;
            }        
            APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_WATCH_HAND_SET_PARAM));          
            ack_state = ACK_STATE_SUCCESS;
            break;
        }
 
        case PROT_EVT_ID_ECHO_WATCH_HAND_HEARTBEAT:
            APP_ERROR_CHECK(app_timer_stop(mode_timeout_timer_id));
            APP_ERROR_CHECK(app_timer_start(mode_timeout_timer_id,
                            APP_TIMER_TICKS(CONFIG_MODE_TIMEOUT * 1000),
                            NULL));
            p_evt->header.evt_id = PROT_EVT_ID_INVALID;
            ack_state = ACK_STATE_INVALID;
            break;

        case PROT_EVT_ID_SET_LONG_SIT_SWITCH:            /* 久坐开关 */
            if (p_evt->evt_params.interact.long_sit_switch.remind_enabled == 1)
            {
                system_database.long_sit_remind_enabled = true;

                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_LONG_SIT_START));
            }
            else if (p_evt->evt_params.interact.long_sit_switch.remind_enabled == 0)
            {
                system_database.long_sit_remind_enabled = false;
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_LONG_SIT_STOP));
            }
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_GET_LONG_SIT_SWITCH:            /* 获取久坐开关配置 */
            p_evt->header.evt_id = PROT_EVT_ID_RET_LONG_SIT_SWITCH;
            p_evt->header.evt_len = 1;
            p_evt->evt_params.interact.long_sit_switch.remind_enabled = system_database.long_sit_remind_enabled ? 1 : 0;
            P_Interact_Print("Long sit ret: %s", system_database.long_sit_remind_enabled ? "enable" : "disable");

            ack_state = ACK_STATE_INVALID;
            break;

        case PROT_EVT_ID_SET_LONG_SIT_PARAM:            /* 设置久坐参数 */
            P_Interact_Print("Long sit set: interval: %x minutes", p_evt->evt_params.interact.long_sit_param.intv);
            system_database.long_sit_remind_trigged_intv    =__REV16(p_evt->evt_params.interact.long_sit_param.intv);
            system_database.long_sit_valid_begin_hour       = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.valid_time_begin.hour);
            system_database.long_sit_valid_begin_minute     = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.valid_time_begin.minute);
            system_database.long_sit_valid_end_hour         = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.valid_time_end.hour);
            system_database.long_sit_valid_end_minute       = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.valid_time_end.minute);
            system_database.long_sit_dnd_begin_hour         = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.dnd_time_begin.hour);
            system_database.long_sit_dnd_begin_minute       = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.dnd_time_begin.minute);
            system_database.long_sit_dnd_end_hour           = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.dnd_time_end.hour);
            system_database.long_sit_dnd_end_minute         = BCD_TO_DEC(p_evt->evt_params.interact.long_sit_param.dnd_time_end.minute);

            P_Interact_Print("Long sit set: interval: %d minutes", system_database.long_sit_remind_trigged_intv);
            P_Interact_Print("       valid: %d:%d - %d:%d",        system_database.long_sit_valid_begin_hour, system_database.long_sit_valid_begin_minute,
                                                            system_database.long_sit_valid_end_hour, system_database.long_sit_valid_end_minute);
            P_Interact_Print("         dnd: %d:%d - %d:%d",        system_database.long_sit_dnd_begin_hour, system_database.long_sit_dnd_begin_minute,
                                                            system_database.long_sit_dnd_end_hour, system_database.long_sit_dnd_end_minute);
            APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_LONG_SIT_PARAM_UPDATE));
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_GET_LONG_SIT_PARAM:            /* 获取久坐参数 */
            p_evt->header.evt_id = PROT_EVT_ID_RET_LONG_SIT_PARAM;
            p_evt->header.evt_len = 10;
            p_evt->evt_params.interact.long_sit_param.valid_time_begin.hour     = DEC_TO_BCD(system_database.long_sit_valid_begin_hour);
            p_evt->evt_params.interact.long_sit_param.valid_time_begin.minute   = DEC_TO_BCD(system_database.long_sit_valid_begin_minute);
            p_evt->evt_params.interact.long_sit_param.valid_time_end.hour       = DEC_TO_BCD(system_database.long_sit_valid_end_hour);
            p_evt->evt_params.interact.long_sit_param.valid_time_end.minute     = DEC_TO_BCD(system_database.long_sit_valid_end_minute);
            p_evt->evt_params.interact.long_sit_param.dnd_time_begin.hour       = DEC_TO_BCD(system_database.long_sit_dnd_begin_hour);
            p_evt->evt_params.interact.long_sit_param.dnd_time_begin.minute     = DEC_TO_BCD(system_database.long_sit_dnd_begin_minute);
            p_evt->evt_params.interact.long_sit_param.dnd_time_end.hour         = DEC_TO_BCD(system_database.long_sit_dnd_end_hour);
            p_evt->evt_params.interact.long_sit_param.dnd_time_end.minute       = DEC_TO_BCD(system_database.long_sit_dnd_end_minute);
            p_evt->evt_params.interact.long_sit_param.intv                      = __REV16(system_database.long_sit_remind_trigged_intv);

            ack_state = ACK_STATE_INVALID;

            P_Interact_Print("Long sit ret: interval: %d minutes", system_database.long_sit_remind_trigged_intv);
            P_Interact_Print("       valid: %d:%d - %d:%d",        system_database.long_sit_valid_begin_hour, system_database.long_sit_valid_begin_minute,
                                                            system_database.long_sit_valid_end_hour, system_database.long_sit_valid_end_minute);
            P_Interact_Print("         dnd: %d:%d - %d:%d",        system_database.long_sit_dnd_begin_hour, system_database.long_sit_dnd_begin_minute,
                                                            system_database.long_sit_dnd_end_hour, system_database.long_sit_dnd_end_minute);
            break;

        case PROT_EVT_ID_SET_DND_SWITCH:                /* 勿扰开关 */
            if (p_evt->evt_params.interact.dnd_switch.remind_enabled == 1)
            {
                system_database.dnd_state_enabled = true;
                //DBG1_EVT0("Do not disturb begin");
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_DND_START));
            }
            else if (p_evt->evt_params.interact.dnd_switch.remind_enabled == 0)
            {
                system_database.dnd_state_enabled = false;
                //DBG1_EVT0("Do not disturb end");
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_DND_STOP));
            }
            do_not_disturb_state_proecess();
            ack_state = ACK_STATE_SUCCESS;
            break;


        case PROT_EVT_ID_GET_DND_SWITCH:                /* 获取勿扰开关配置 */
            p_evt->header.evt_id = PROT_EVT_ID_RET_DND_SWITCH;
            p_evt->header.evt_len = 1;
            p_evt->evt_params.interact.dnd_switch.remind_enabled = system_database.dnd_state_enabled ? 1 : 0;
        
            ack_state = ACK_STATE_INVALID;
            break;

        case PROT_EVT_ID_SET_DND_PARAM:                 /* 勿扰参数 */
            system_database.dnd_start_hour           = BCD_TO_DEC(p_evt->evt_params.interact.dnd_param.begin_time.hour);
            system_database.dnd_start_minute         = BCD_TO_DEC(p_evt->evt_params.interact.dnd_param.begin_time.minute);
            system_database.dnd_end_hour             = BCD_TO_DEC(p_evt->evt_params.interact.dnd_param.end_time.hour);
            system_database.dnd_end_minute           = BCD_TO_DEC(p_evt->evt_params.interact.dnd_param.end_time.minute);

            P_Interact_Print("Do not disturb set: %d:%d - %d:%d",  system_database.dnd_start_hour, system_database.dnd_start_minute, 
                                                            system_database.dnd_end_hour, system_database.dnd_end_minute);
            APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_DND_PARAM_UPDATE));
            do_not_disturb_state_proecess();
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_GET_DND_PARAM:                 /* 获取勿扰参数 */
            p_evt->header.evt_id = PROT_EVT_ID_RET_DND_PARAM;
            p_evt->header.evt_len = 4;

            p_evt->evt_params.interact.dnd_param.begin_time.hour    = DEC_TO_BCD(system_database.dnd_start_hour);
            p_evt->evt_params.interact.dnd_param.begin_time.minute  = DEC_TO_BCD(system_database.dnd_start_minute);
            p_evt->evt_params.interact.dnd_param.end_time.hour      = DEC_TO_BCD(system_database.dnd_end_hour);
            p_evt->evt_params.interact.dnd_param.end_time.minute    = DEC_TO_BCD(system_database.dnd_end_minute);

            ack_state = ACK_STATE_INVALID;
        
            P_Interact_Print("Do not disturb ret: %d:%d - %d:%d",  system_database.dnd_start_hour, system_database.dnd_start_minute, 
                                                            system_database.dnd_end_hour, system_database.dnd_end_minute);
            break;      
        
        case PROT_EVT_ID_SET_VIBRATION_SCALE:  /* 设置振动反馈 */
            //DBG1_EVT0("SET_VIBRATION_SCALE");
            system_database.callin_vibration_scale  =p_evt->evt_params.interact.vibration_scale.callin_scale; //来电振动反馈
            system_database.msg_vibration_scale     =p_evt->evt_params.interact.vibration_scale.alarm_scale; //提醒
            system_database.alarm_vibration_scale   =p_evt->evt_params.interact.vibration_scale.alarm_scale; //闹钟
            system_database.long_sit_scale          =p_evt->evt_params.interact.vibration_scale.long_sit_scale;
            system_database.step_goal_finish_scale  =p_evt->evt_params.interact.vibration_scale.step_goal_finish_scale;
            ack_state = ACK_STATE_SUCCESS;
            break;
        case PROT_EVT_ID_GET_VIBRATION_SCALE:  /* 获取振动反馈 */
            p_evt->header.evt_id =PROT_EVT_ID_RET_VIBRATION_SCALE;
            p_evt->header.evt_len = 5;
            p_evt->evt_params.interact.vibration_scale.callin_scale = system_database.callin_vibration_scale;
            p_evt->evt_params.interact.vibration_scale.msg_scale    = system_database.msg_vibration_scale;
            p_evt->evt_params.interact.vibration_scale.alarm_scale  = system_database.alarm_vibration_scale;
            p_evt->evt_params.interact.vibration_scale.long_sit_scale               = system_database.long_sit_scale;
            p_evt->evt_params.interact.vibration_scale.step_goal_finish_scale       = system_database.step_goal_finish_scale;
            ack_state = ACK_STATE_INVALID;
            break;
        

        
        case PROT_EVT_ID_CALL_ENTER_GERRN_MODE:
            DBG1_EVT0("PROT_EVT_ID_CALL_ENTER_GERRN_MODE");
            static uint8_t mode = 0;
            mode = p_evt->evt_params.interact.prot_green_mode.green_mode;
            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, enter_green_mode_sched_handler, &mode));        
            ack_state = ACK_STATE_SUCCESS;
            break;
             
        case PROT_EVT_ID_SET_FUNCTION_SETTINGS:
            system_database.function_settings = __REV(p_evt->evt_params.interact.function_settings.settings);
            DBG1_EVT1("PROT_EVT_ID_SET_FUNCTION_SETTINGS function_settings : %08x ",system_database.function_settings);        
            ack_state = ACK_STATE_SUCCESS;
            break;
        
        case PROT_EVT_ID_GET_FUNCTION_SETTINGS:
            
            p_evt->header.evt_id = PROT_EVT_ID_RET_FUNCTION_SETTINGS;
            p_evt->header.evt_len = 4;
            p_evt->evt_params.interact.function_settings.settings = __REV(system_database.function_settings); 
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

uint32_t m_protocol_ineract_init(void)
{
    uint32_t status = NRF_SUCCESS;

    status = app_timer_create(&mode_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, mode_timeout_timer_handler);

    if (status == NRF_SUCCESS)
    {
        status = app_timer_create(&linkloss_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, linkloss_timeout_timer_handler);
    }

    return status;
}










