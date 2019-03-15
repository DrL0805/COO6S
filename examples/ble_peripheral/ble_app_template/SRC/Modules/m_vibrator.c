

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "drv_vibrator.h"
#include "AD_ShakeHandAlgorithm.h"
#include "resources.h"


#include "m_database.h"
#include "event_bus.h"

#include "sr3_config.h"
#include "includes.h"

#if CONFIG_RTT_VIBRATOR_ENABLED
#define MOTOR_Print(M, ...)	SEGGER_Print("MOTOR", M, ##__VA_ARGS__)
#else
#define MOTOR_Print(M, ...)
#endif


typedef enum{
    VIBRATION_SCALE_INVID = 0,
    VIBRATION_SCALE_WEAK,
    VIBRATION_SCALE_MEDIUM,
    VIBRATION_SCALE_STRONG,
}vibration_scale;

typedef struct {
     uint8_t callin_scale;
     uint8_t alarm_scale;
     uint8_t msg_scale;
     uint8_t long_sit_scale;
     uint8_t step_goal_finish_scale;
}vibration_setting_t;

#if CONFIG_VIBRATOR_ENABLED

static bool m_vibrator_enabled = false;
static bool m_is_ble_connected = false;
static bool m_vibrator_valided = false;
static bool m_busy             = false;

vibration_setting_t vibration_setting ={0};

#ifdef  CONFIG_BOARD_PH000   
static drv_vibrator_config_t const config_electrify            = {125, 875,  1,  0,   1};  // 上电
static drv_vibrator_config_t const config_electrify_reset      = {125, 875,  1,  0,   1};  // 复位
static drv_vibrator_config_t const config_auth_req             = {125, 125,  4,  0,   1};  // 授权请求
static drv_vibrator_config_t const config_alarm_clk            = {125, 375, 20,  0,   1};  // 闹钟提醒
static drv_vibrator_config_t const config_msg_long             = {125, 375, 10,  0,   1};  // 来电等重要消息提醒 
static drv_vibrator_config_t const config_msg_short            = {125, 375,  4,  0,   1};  // 其他消息提醒
static drv_vibrator_config_t const config_photo_req            = {125, 875,  1,  0,   1};  // 请求拍照
static drv_vibrator_config_t const config_find_phone           = {125, 875,  1,  0,   1};  // 查找手机
static drv_vibrator_config_t const config_steps_goal_finished  = {125, 375,  10, 0,   1};  // 步数目标完成
static drv_vibrator_config_t const config_linkloss             = {125, 875,  3,  0,   1};  // 已经断开
static drv_vibrator_config_t const config_linkloss_config      = {125, 875,  1,  0,   1};  // 有新的断开提醒设置
static drv_vibrator_config_t const config_long_sit             = {125, 375,  4,  0,   1};  // 久坐提醒
static drv_vibrator_config_t const config_vibration_strong     = {125, 375, 20,  0,   1};  // 高振感提醒
static drv_vibrator_config_t const config_vibration_medium     = {125, 375, 10,  0,   1};  // 中振感提醒
static drv_vibrator_config_t const config_vibration_weak       = {125, 375, 4,   0,   1};  // 低振感提醒

static drv_vibrator_config_t  config_spec_event_remind         = {125, 375, 4,   0,   1};  
static drv_vibrator_config_t  config_complete_mag_adjust       = {125, 875, 1,   0,   1};  
#else
static drv_vibrator_config_t const config_electrify            = {125, 875,  1,  0,   1};  // 上电
static drv_vibrator_config_t const config_electrify_reset      = {125, 875,  1,  0,   1};  // 复位
static drv_vibrator_config_t const config_auth_req             = {125, 125,  4,  0,   1};  // 授权请求
static drv_vibrator_config_t const config_alarm_clk            = {125, 375, 20,  0,   1};  // 闹钟提醒
static drv_vibrator_config_t const config_msg_long             = {125, 375, 10,  0,   1};  // 来电等重要消息提醒 
static drv_vibrator_config_t const config_msg_short            = {125, 375,  4,  0,   1};  // 其他消息提醒
static drv_vibrator_config_t const config_photo_req            = {125, 875,  1,  0,   1};  // 请求拍照
static drv_vibrator_config_t const config_find_phone           = {125, 875,  1,  0,   1};  // 查找手机
static drv_vibrator_config_t const config_steps_goal_finished  = {125, 375,  8,  0,   1};  // 步数目标完成
static drv_vibrator_config_t const config_linkloss             = {125, 875,  3,  0,   1};  // 已经断开
static drv_vibrator_config_t const config_linkloss_config      = {125, 875,  1,  0,   1};  // 有新的断开提醒设置
static drv_vibrator_config_t const config_long_sit             = {125, 375,  4,  0,   1};  // 久坐提醒

static drv_vibrator_config_t const config_vibration_strong     = {125, 375, 20,  0,   1};  // 高振感提醒
static drv_vibrator_config_t const config_vibration_medium     = {125, 375, 10,  0,   1};  // 中振感提醒
static drv_vibrator_config_t const config_vibration_weak       = {125, 375, 4,   0,   1};  // 低振感提醒

static drv_vibrator_config_t  config_spec_event_remind         = {125, 375, 4,   0,   1};  
static drv_vibrator_config_t  config_complete_mag_adjust       = {125, 875, 1,   0,   1};  
#endif

static bool     m_photo_mode_enter                  = false;  // 拍照请求标志

static void virator_ind_start(drv_vibrator_config_t config)
{
    MOTOR_Print("m_vibrator: virator_ind_start");
    if ((m_busy) || (config.on == 0))
    {
        return;
    }   
    APP_ERROR_CHECK(drv_vibrator_stop());
    APP_ERROR_CHECK(drv_vibrator_start(config));
}

static void virator_ind_stop(void )
{
    MOTOR_Print("m_vibrator: virator_ind_stop");
    m_busy = false;
    APP_ERROR_CHECK(drv_vibrator_stop());
}


static void drv_vibrator_event_handler(drv_vibrator_evt_type_t evt, uint16_t time)
{
    if (evt == DRV_VIBRATOR_EVT_STARTED)
    {
        MOTOR_Print("drv_vibrator_event_handler: true");
        m_busy = true;
        SetVibraTimeout(CEIL_DIV(time, 500));
        event_send(EVT_SYSTEM_VIBRATOR_STATE,m_busy);
    }
    else if (evt == DRV_VIBRATOR_EVT_DONE)
    {
        MOTOR_Print("drv_vibrator_event_handler: false");
        m_busy = false;
        event_send(EVT_SYSTEM_VIBRATOR_STATE,m_busy);
    }
}

static void app_recv_msg_process(bool is_cancel_msg)
{
    MOTOR_Print("m_vibrator: is_cancel_msg : %d",is_cancel_msg);
    uint32_t msg_bitmap = 0;

    if (is_cancel_msg)
    {
        msg_bitmap = system_database.cancel_msg_bitmap
                     & system_database.msg_switch_bitmap;
    }
    else
    {
        msg_bitmap = system_database.new_msg_bitmap
                     & system_database.msg_switch_bitmap;
    }


    if (msg_bitmap && !is_cancel_msg)
    {
        if (  CHECK_VALID_BIT(msg_bitmap, MSG_BIT_CALLIN)
           || CHECK_VALID_BIT(msg_bitmap, MSG_BIT_CONTEXT_CALLIN))
        {
            MOTOR_Print("system_database.callin_vibration_scale %02x",system_database.callin_vibration_scale);
            switch(system_database.callin_vibration_scale)
            {
                
                case VIBRATION_SCALE_STRONG:
                    virator_ind_start(config_vibration_strong);
                    break;
                case VIBRATION_SCALE_MEDIUM:
                    virator_ind_start(config_vibration_medium);
                    break;
                case VIBRATION_SCALE_WEAK:
                    virator_ind_start(config_vibration_weak);
                    break;
                
                default:
                    virator_ind_start(config_msg_long);
                    break;
                    
            }           
            MOTOR_Print("m_vibrator: config_callin");
        }
        else
        {
            MOTOR_Print("system_database.msg_vibration_scale %02x",system_database.msg_vibration_scale);
            switch(system_database.msg_vibration_scale)
            {
                
                case VIBRATION_SCALE_STRONG:
                    virator_ind_start(config_vibration_strong);
                    break;
                case VIBRATION_SCALE_MEDIUM:
                    virator_ind_start(config_vibration_medium);
                    break;
                case VIBRATION_SCALE_WEAK:
                    virator_ind_start(config_vibration_weak);
                    break;
                
                default:
                    virator_ind_start(config_msg_short);
                    break;
                    
            }
            MOTOR_Print("m_vibrator: config_msg");
        }
    }
    
}


ret_code_t m_vibrator_init(void)
{
    drv_vibrator_init(drv_vibrator_event_handler);
    return NRF_SUCCESS;
}

ret_code_t m_vibrator_enable(void)
{
    MOTOR_Print("m_vibrator: Enabled");
    ASSERT(m_vibrator_enabled == false);

    m_vibrator_enabled = true;

    return NRF_SUCCESS;
}

ret_code_t m_vibrator_disable(void)
{
    MOTOR_Print("m_vibrator: Disabled");
    ASSERT(m_vibrator_enabled == true);

    m_vibrator_enabled = false;

    return NRF_SUCCESS;
}

bool m_vibrator_event_handler(const event_t * p_event)
{
    if (p_event->type == EVT_SYSTEM_STATE)
    {
        if (p_event->system.system_state == SYSTEM_STATE_NORMAL)
        {
            m_vibrator_valided = true;
        }
        if (p_event->system.system_state == SYSTEM_STATE_RESUME_F_GREEN)
        {
            m_vibrator_valided = true;
        }

        else if (p_event->system.system_state == SYSTEM_STATE_ELETRIFY)
        {
            MOTOR_Print("m_vibrator: SYSTEM_STATE_ELETRIFY");
            virator_ind_start(config_electrify);
        }
        else if (p_event->system.system_state == SYSTEM_STATE_ELETRIFY_TEST)
        {
//            virator_ind_start(config_electrify_test);
        }
        else if (p_event->system.system_state == SYSTEM_STATE_RESET)
        {
            virator_ind_start(config_electrify_reset); // 手动复位
        }
        else if (p_event->system.system_state == SYSTEM_STATE_LOWPOWER)
        {
            m_vibrator_valided = false;
            virator_ind_stop();
        }
        else if (p_event->system.system_state == SYSTEM_STATE_GREEN_MODE)
        {
            m_vibrator_valided = false;
            virator_ind_stop();
            m_vibrator_disable();
        }
    }
    if (!m_vibrator_valided)
    {
        return false;
    }

    switch(p_event->type)
    {
        case EVT_SYSTEM_BT_CONN_STATE:
            if (p_event->system.data >= BT_CONN_STATE_CONNECTED)
            {
                m_is_ble_connected = true;
            }
            else 
            {
                m_is_ble_connected = false;
            }
            break;
            
        case EVT_SYSTEM_MAG_COMPLETE_ADJUST:
            virator_ind_start(config_complete_mag_adjust);
            break;

        case EVT_BUTTON_PRESSED:
            switch (p_event->button.button_event)
            {
                case BUTTON_EVT_PHOTO_REQ:
                    if (m_is_ble_connected)
                    {
                        virator_ind_start(config_photo_req);
                    }
                    break;

                case BUTTON_EVT_FIND_PHONE:
                    if (m_is_ble_connected)
                    {
                        virator_ind_start(config_find_phone);
                    }
                    break;

                case BUTTON_EVT_REJECT_CALLIN:
                    virator_ind_stop();
                    break;

                default:
                    break;
            }
            break;

        case EVT_COMS_APP_RECV:
            switch (p_event->coms.app_recv_event)
            {
                case APP_RECV_EVT_PHOTO_ENTER_MODE:
                    m_photo_mode_enter = true;
                    break;
                
                case APP_RECV_EVT_PHOTO_EXIT_MODE:               
                    m_photo_mode_enter = false;
                    break;
                
                
                case APP_RECV_EVT_MSG_NEW:
                    if(!m_photo_mode_enter)
                    {
                        app_recv_msg_process(false);
                    }
                    break;

                case APP_RECV_EVT_MSG_CANCEL:
                    if(!m_photo_mode_enter)
                    {
                        app_recv_msg_process(true);
                    }
                    break;

                case APP_RECV_EVT_AUTH_ENTER_MODE:
                    MOTOR_Print("m_vibrator: config_auth_req");
                    virator_ind_start(config_auth_req);
                   
                    break;

                case APP_RECV_EVT_LINKLOSS_ENABLED_CONFIG:
                    if (system_database.linkloss_remind_enabled)
                    {
                        virator_ind_start(config_linkloss_config);
                    }
                    break;
                    
                case APP_RECV_EVT_SPEC_EVENT_REMIND:                    
                    switch(system_database.vib_freq)
                    {
                        case 0://弱
                            config_spec_event_remind.on  = 125;
                            config_spec_event_remind.off = 875;
                            config_spec_event_remind.flash_count = system_database.vib_times;
                            break;
                        case 1://中
                            config_spec_event_remind.on  = 125;
                            config_spec_event_remind.off = 375;
                            config_spec_event_remind.flash_count = system_database.vib_times * 2 ;
                            break;
                        case 2://强
                            config_spec_event_remind.on  = 125;
                            config_spec_event_remind.off = 125;
                            config_spec_event_remind.flash_count = system_database.vib_times * 4;
                            break;
                    }
                    virator_ind_start(config_spec_event_remind);
                    break;
        
                default:
                    break;
            }
            break;

        case EVT_COMS_PROT_EXEC:
            if (PROT_EXEC_EVT_ALARM_TRIGGERED == p_event->coms.prot_execution)
            {
                MOTOR_Print("system_database.alarm_vibration_scale %02x",system_database.alarm_vibration_scale);
                switch(system_database.alarm_vibration_scale)
                {
                    
                    case VIBRATION_SCALE_STRONG:
                        virator_ind_start(config_vibration_strong);
                        break;
                    case VIBRATION_SCALE_MEDIUM:
                        virator_ind_start(config_vibration_medium);
                        break;
                    case VIBRATION_SCALE_WEAK:
                        virator_ind_start(config_vibration_weak);
                        break;
                    
                    default:
                        virator_ind_start(config_alarm_clk);
                        break;
                        
                }                
            }
            else if (PROT_EXEC_EVT_LINKLOSS_TRIGGERED == p_event->coms.prot_execution)
            {
                virator_ind_start(config_linkloss);
            }
            break;

        case EVT_SYSTEM_STEPS_GOAL_FINISHED:
            switch(system_database.step_goal_finish_scale)
            {
                
                case VIBRATION_SCALE_STRONG:
                    virator_ind_start(config_vibration_strong);
                    break;
                case VIBRATION_SCALE_MEDIUM:
                    virator_ind_start(config_vibration_medium);
                    break;
                case VIBRATION_SCALE_WEAK:
                    virator_ind_start(config_vibration_weak);
                    break;
                
                default:
                    virator_ind_start(config_steps_goal_finished);
                    break;
                    
            }                   
            break;

        case EVT_SYSTEM_LONG_SIT_TRIGGERED:
            MOTOR_Print(".EVT_SYSTEM_LONG_SIT_TRIGGERED ");
            switch(system_database.long_sit_scale)
            {
                
                case VIBRATION_SCALE_STRONG:
                    virator_ind_start(config_vibration_strong);
                    break;
                case VIBRATION_SCALE_MEDIUM:
                    virator_ind_start(config_vibration_medium);
                    break;
                case VIBRATION_SCALE_WEAK:
                    virator_ind_start(config_vibration_weak);
                    break;
                
                default:
                    virator_ind_start(config_long_sit);
                    break;
                    
            }  
            
            break;

        default:
            break;
    }

    return false;
}

bool m_vibrator_wakeup_prepare(nrf_pwr_mgmt_evt_t shutdown_type)
{
    MOTOR_Print(".m_vibrator_wakeup_prepare  m_busy %d", m_busy);
    bool ready = false;

    if (m_busy)
    {
    }
    else
    {
        virator_ind_stop();
        ready = true;
    }
    MOTOR_Print(".m_vibrator_wakeup_prepare  ready %d", ready);
    return ready;
    
}

NRF_PWR_MGMT_HANDLER_REGISTER(m_vibrator_wakeup_prepare, SHUTDOWN_PRIORITY_DEFAULT);

#endif /* CONFIG_VIBRATOR_ENABLED */


