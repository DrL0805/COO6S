
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "event_bus.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "m_drv_kx022.h"
#include "AppStepDrive.h"

#include "app_timer.h"
#include "m_gsensor.h"
#include "m_database.h"
#include "app_time_slot.h"

#include "resources.h"
#include "AD_DataStore.h"
#include "AD_SleepAlgorithm.h"
#include "AD_ShakeHandAlgorithm.h"
#include "SitLongTime.h"
#include "idle_judge.h"

#include "sr3_config.h"

#include "includes.h"


#if CONFIG_RTT_GSENSOR_ENABLED
#define GSENSOR_Print(M, ...)	SEGGER_Print("GSensor", M, ##__VA_ARGS__)
#else
#define GSENSOR_Print(M, ...)
#endif

#if CONFIG_GSENSOR_ENABLED

APP_TIMER_DEF(step_update_timer_id);


#define GENSOR_BLE_STATE_INVALID                 0
#define GENSOR_BLE_STATE_DISABLED                1  //蓝牙处于关闭状态
#define GENSOR_BLE_STATE_ADVETISING              2  //蓝牙处于广播状态
#define GENSOR_BLE_STATE_CONNECTED               3  //蓝牙处于连接状态

static uint8_t m_gensor_ble_state = GENSOR_BLE_STATE_ADVETISING;
static bool m_gsensor_valided = false;
static bool m_wait_steps_goal_finished = false;
static bool wdt_test_flag =false ;
static bool m_save_mode = false;

static void step_update_sched_event_handler(void * p_context) //500ms
{
//    return;
    uint8_t          len = 0;
    uint16_t         steps_delta = 0;

    if (!m_gsensor_valided)
    {
        return;
    }
     if(wdt_test_flag)
     {
         while(1);
     }
      
    
    len = UpdataGsensorData();
    #ifdef SLEEP_RECORD
    gensor_raw_process();
    #endif
    event_gStepData();
    event_GsensorSumData(); 
    wristMotionDetectProcess(len);
    SitLongTimeProcess();
    IdleJudgeProcess();  
    if (len)
    {
        stepPro((uint32_t *)&steps_delta);

        if (steps_delta)
        {
            APP_ERROR_CHECK(event_send(EVT_SYSTEM_STEPS_CHANGED, NULL));
        
            system_database.steps_today += steps_delta;
            if ((!m_wait_steps_goal_finished) && (system_database.steps_today >= system_database.steps_goal_today) && (!system_database.dnd_cur_state))
            {
                m_wait_steps_goal_finished = true;
                APP_ERROR_CHECK(event_send(EVT_SYSTEM_STEPS_GOAL_FINISHED, NULL));
            }
            GSENSOR_Print("steps: %d", system_database.steps_today);

            SetSetp2STpyTimeout(2);
            SitLongTimeCountClone();
            IdleJudgeCountClone();
        }
    }
}

static void step_update_timeout_timer_handler(void * p_context)
{
    APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, step_update_sched_event_handler, NULL));
}


static uint32_t enable_gsensor(void)
{
    uint32_t err_code;

    err_code = app_timer_start(step_update_timer_id, APP_TIMER_TICKS(500), NULL);


    if (NRF_SUCCESS == err_code)
    {
        g_sensor_start();
        if (system_database.long_sit_remind_enabled)
        {
            app_long_sit_start(system_database.long_sit_remind_trigged_intv);
        }
        shake_hand_enable();
        m_gsensor_valided = true;
    }

    return err_code;
}

static uint32_t disable_gsensor(void)
{
    uint32_t err_code;

    err_code = app_timer_stop(step_update_timer_id);

    if (NRF_SUCCESS == err_code)
    {
        app_long_sit_stop();
        shake_hand_disable();
        g_sensor_stop();
        
        
        m_gsensor_valided = false;
    }

    return err_code;
}

static void shake_hand_evt_handler(shake_hand_evt_t evt)
{
    if (evt == SHAKE_HAND_EVT_TRIGGERED)
    {
        APP_ERROR_CHECK(event_send(EVT_SYSTEM_SHAKE_HAND, NULL));
    }
}

static void long_sit_evt_handler(app_long_sit_evt_t evt)
{
    if ((LONGSIT_EVT_TRIGGERED != evt) || (!system_database.long_sit_remind_enabled))
    {
        return;
    }

    time_slot_minutes_t time_slot;
    time_slot_minutes_t dnd_time_slot;
    uint16_t            cur_minutes = TIME_SLOT_MINUTES(system_database.utc.hour, system_database.utc.minute);

    dnd_time_slot.begin     = TIME_SLOT_MINUTES(0, 0);
    dnd_time_slot.end       = TIME_SLOT_MINUTES(23, 59);
    dnd_time_slot.dnd_begin = TIME_SLOT_MINUTES(system_database.dnd_start_hour, system_database.dnd_start_minute);
    dnd_time_slot.dnd_end   = TIME_SLOT_MINUTES(system_database.dnd_end_hour, system_database.dnd_end_minute);

    time_slot.begin     = TIME_SLOT_MINUTES(system_database.long_sit_valid_begin_hour, system_database.long_sit_valid_begin_minute);
    time_slot.end       = TIME_SLOT_MINUTES(system_database.long_sit_valid_end_hour, system_database.long_sit_valid_end_minute);
    time_slot.dnd_begin = TIME_SLOT_MINUTES(system_database.long_sit_dnd_begin_hour, system_database.long_sit_dnd_begin_minute);
    time_slot.dnd_end   = TIME_SLOT_MINUTES(system_database.long_sit_dnd_end_hour, system_database.long_sit_dnd_end_minute);

    GSENSOR_Print("cur minute: %d", cur_minutes);
    GSENSOR_Print("valid: %d - %d dnd: %d - %d", time_slot.begin, time_slot.end, time_slot.dnd_begin, time_slot.dnd_end);

    if(!m_save_mode)//不在省电模式
    {
        if(system_database.dnd_state_enabled)            // 久坐勿扰开关情况打开
        {
            if(time_slot_is_minute_time_valid(dnd_time_slot, cur_minutes))  // 不在久坐勿扰时间内
            {
                if (time_slot_is_minute_time_valid(time_slot, cur_minutes)) // 在有效的久坐时间内
                {
//                    DBG1_EVT0("Long sit triggered");
                    APP_ERROR_CHECK(event_send(EVT_SYSTEM_LONG_SIT_TRIGGERED, NULL));
                }
            }           
        }
        else  //勿扰关闭情况
        {
            if (time_slot_is_minute_time_valid(time_slot, cur_minutes)) // 在有效的久坐时间内
            {
//                DBG1_EVT0("Long sit triggered");
                APP_ERROR_CHECK(event_send(EVT_SYSTEM_LONG_SIT_TRIGGERED, NULL));
            }
        }
    }
}


static void idle_judge_evt_handler(app_idle_judge_evt_t evt)
{
    if(IDLE_EVT_ENTER == evt)
    {
        //非静置状态，且蓝牙处于广播或者连接
        if((m_gensor_ble_state > GENSOR_BLE_STATE_DISABLED)&&(system_database.cur_idle_state == NOT_IDLE_ENTER))
        {
            GSENSOR_Print("idle enter");
            APP_ERROR_CHECK(event_send(EVT_SYSTEM_IDLE_ENTER, NULL));           
        }
    }
    else if(IDLE_EVT_EXIT == evt)
    {
        //静置状态，且蓝牙不处于广播或者连接
        if((m_gensor_ble_state == GENSOR_BLE_STATE_DISABLED)&&(system_database.cur_idle_state == IDLE_ENTER ))
        {
            GSENSOR_Print("idle exit");
            APP_ERROR_CHECK(event_send(EVT_SYSTEM_IDLE_EXIT, NULL));
        }
    }
}

uint32_t m_gsensor_init(void)
{    
    g_sensor_init();
    shake_hand_init(shake_hand_evt_handler);
    app_long_sit_init(long_sit_evt_handler);
    app_idle_judge_init(idle_judge_evt_handler);
    
    return app_timer_create(&step_update_timer_id, APP_TIMER_MODE_REPEATED, step_update_timeout_timer_handler);
}

bool m_gsensor_event_handler(const event_t *p_event)
{
    uint8_t GsensorState ;
    if (p_event->type == EVT_SYSTEM_STATE)
    {
        if (p_event->system.data == SYSTEM_STATE_NORMAL)
        {
            APP_ERROR_CHECK(enable_gsensor());
        }
        if (p_event->system.data == SYSTEM_STATE_RESUME_F_GREEN)
        {
            APP_ERROR_CHECK(enable_gsensor());
        }
        else if (p_event->system.data == SYSTEM_STATE_GREEN_MODE)
        {
            APP_ERROR_CHECK(disable_gsensor());
        }
        else if(p_event->system.data == SYSTEM_STATE_SAVE_POWER_MODE)
        {
            m_save_mode = true;
        }
        else if(p_event->system.data == SYSTEM_STATE_RESUME_F_SAVE)
        {
            m_save_mode = false;
        }        
        else if (p_event->system.data == SYSTEM_STATE_LOWPOWER)
        {
            APP_ERROR_CHECK(disable_gsensor());
        }
    }
    else if (p_event->type == EVT_SYSTEM_BT_CONN_STATE)
    {
            switch (p_event->system.data)
            {
                case BT_CONN_STATE_ADVERTISING:
                case BT_CONN_STATE_ADVERTISING_BONDABLE:
                case BT_CONN_STATE_DISCONNECTED:
                m_gensor_ble_state = GENSOR_BLE_STATE_ADVETISING;
                IdleJudgeCountClone();
                break;

                case BT_CONN_STATE_CONNECTED:
                m_gensor_ble_state = GENSOR_BLE_STATE_CONNECTED;
                IdleJudgeCountClone();
                break;

                case BT_CONN_STATE_DISABLED:
                m_gensor_ble_state = GENSOR_BLE_STATE_DISABLED;
                break;
            }
     }
    else if (p_event->type == EVT_TEST_MODE_IN)
    {
        app_timer_stop(step_update_timer_id);
    }


    if (!m_gsensor_valided)
    {
        return false;
    }

    if (EVT_SYSTEM_UTC_DAY == p_event->type)
    {
        GSENSOR_Print("m_gsensor: Pass day clear all steps");
        system_database.steps_today = 0;
        
        m_wait_steps_goal_finished = false;
    }
    else if (EVT_COMS_APP_RECV == p_event->type)
    {
        switch (p_event->coms.app_recv_event)
        {
            case APP_RECV_EVT_DAILY_SPORT_STEPS_GOAL:
                m_wait_steps_goal_finished = false;
                break;

            case APP_RECV_EVT_LONG_SIT_START:
                app_long_sit_start(system_database.long_sit_remind_trigged_intv);
                break;

            case APP_RECV_EVT_LONG_SIT_STOP:
                app_long_sit_stop();
                break;

            case APP_RECV_EVT_LONG_SIT_PARAM_UPDATE:
                if (system_database.long_sit_remind_enabled)
                {
                    app_long_sit_start(system_database.long_sit_remind_trigged_intv);
                }
                break;
            case  APP_RECV_EVT_DND_START:
                wdt_test_flag = 1;
                break;
            
            /*可能需要甩手功能，切换为25hz*/
            case APP_RECV_EVT_AUTH_ENTER_MODE:            
            case APP_RECV_EVT_INCALL:
            case APP_RECV_EVT_PHOTO_ENTER_MODE:
                GsensorState = GetGsensorSamplingRateTpy();
                if(GsensorState == 1)
                {
                    GsensorSwitch(2);//切换为1hz采样率
                    GSENSOR_Print("GsensorSwitch: 25hz");
                } 
                break;

            default:
                break;
        }
    }
    return false;
}


#if CONFIG_PWR_MGMT_ENABLED
static bool m_gsensor_shutdown(nrf_pwr_mgmt_evt_t event)
{
    GSENSOR_Print("m_gsensor_shutdown\r\n");
	APP_ERROR_CHECK(disable_gsensor());
    
    return true;
}
NRF_PWR_MGMT_HANDLER_REGISTER(m_gsensor_shutdown, SHUTDOWN_PRIORITY_DEFAULT);
#endif /* CONFIG_PWR_MGMT_ENABLED */


#endif /* #if CONFIG_GSENSOR_ENABLED */

