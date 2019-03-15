
#include <stdlib.h>
#include "sr3_config.h"
#include "nrf_assert.h"
#include "app_isched.h"
#include "app_debug.h"
#include "app_timer.h"
#include "m_system_state.h"
#include "m_button.h"
#include "app_timer.h"
#include "resources.h"
#include "m_database.h"

#include "nrf_delay.h"
#include "nrf_log.h"
#include "includes.h"

#define POWERON_ENTER_NORMAL  0

#if CONFIG_RTT_SYSTEM_STATE_ENABLED
#define M_SYSTEM_Print(M, ...)	SEGGER_Print("M_SYSTEM", M, ##__VA_ARGS__)
#else
#define M_SYSTEM_Print(M, ...)
#endif


uint8_t cur_state;

APP_TIMER_DEF(system_state_timer_id);

static void update_state(uint8_t state)
{
    M_SYSTEM_Print("update_state: %d",state);
    if (SYSTEM_STATE_STORAGE == state)
    {
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
    }

    APP_ERROR_CHECK(event_send(EVT_SYSTEM_STATE, state));

    cur_state = state;
}

/* 开机按键判断定时器 */
static void m_system_state_timeout_handler(void * p_context)
{
//    //DBG0_EVT0("m_system_state_timeout_handler");
    static uint32_t count_10ms = 0;
    static uint32_t pressed_count_10ms = 0;    

    if (m_button_check_enter_normal_state_button())
    {
        count_10ms = 0;
        pressed_count_10ms++;

        if (pressed_count_10ms == 1) // 按下触发
        {
            APP_ERROR_CHECK(event_send(EVT_SYSTEM_PRESSED_TO_POWERON, NULL));
        }
        else if (pressed_count_10ms == 250) // 长按触发
        {
            M_SYSTEM_Print("EVT_SYSTEM_PRESSED_TO_ENTER_NORMAL\r\n");
            APP_ERROR_CHECK(event_send(EVT_SYSTEM_PRESSED_TO_ENTER_NORMAL, NULL));
        }
    }
    else
    {
        if ((pressed_count_10ms < 250) && (count_10ms == 0)) // 短按释放触发
        {
            M_SYSTEM_Print(" : update_state(SYSTEM_STATE_POWERON_FAIL)");
            pressed_count_10ms = 0;
            update_state(SYSTEM_STATE_POWERON_FAIL);
        }
        else if (pressed_count_10ms >= 250) // 长按释放触发
        {
            APP_ERROR_CHECK(app_timer_stop(system_state_timer_id));
            M_SYSTEM_Print("pressed_count_10ms : update_state(SYSTEM_STATE_NORMAL)");
            update_state(SYSTEM_STATE_NORMAL);
        }

        count_10ms++;

        /* 开机失败，等待 4s(4s的时间依据为 LED 电量指示的时间最长值)后进仓储模式，期间若有新的按键则重新进行开机按键判断 */
        if (count_10ms >= 410)
        {
            APP_ERROR_CHECK(app_timer_stop(system_state_timer_id));
            update_state(SYSTEM_STATE_STORAGE);
        }
    }
}

ret_code_t m_system_state_init(bool is_resume, bool is_pressed, bool is_pressed_reset,uint32_t reset_error)
{
    M_SYSTEM_Print("m_system_state_init:  is_resume : %d , is_pressed : %d, is_pressed_reset : %d. reset_error : %d .reset_error_code : %d \r\n",is_resume,is_pressed,is_pressed_reset,reset_error,system_database.reset_error_code);
    //DBG0_EVT0("m_system_state: System init");
    M_SYSTEM_Print("m_system_state: is_resume :%x  is_pressed :%x  is_pressed_reset :%x",is_resume,is_pressed,is_pressed_reset);

    ret_code_t err_code = NRF_SUCCESS;
    cur_state = SYSTEM_STATE_STORAGE;

    if (is_pressed_reset) // 长按6s按键按键复位，直接进仓储
    {
        //DBG0_EVT0("update_state(SYSTEM_STATE_STORAGE)");      
        while(m_button_check_enter_normal_state_button());     // 等待按键释放
        update_state(SYSTEM_STATE_STORAGE);
    }

    
    if (system_database.reset_error_code) //不是手动复位
    {
        M_SYSTEM_Print("m_system_state: Reset from error reset: %d", system_database.reset_error_code);
        system_database.reset_error_code = 0;
        M_SYSTEM_Print("reset_error_code : update_state(SYSTEM_STATE_NORMAL)");
        update_state(SYSTEM_STATE_NORMAL);

        return NRF_SUCCESS;
    }
    

//    system_database.reset_reason = reset_error;
    //DBG0_EVT0("reset_reason : %08x",reset_error);


    if (!is_resume)
    {
        if (is_pressed) // 此处不应该条件成立
        {
            M_SYSTEM_Print("m_system_state: Error !!!!!!! Enter manufation test mode !!!!!!!");
        }
        else // 复位启动
        {
            
            #if 0
            if(reset_error == 0)
            {
                NRF_LOG_INFO("m_system_state: SYSTEM_STATE_ELETRIFY_TEST\r\n");
                update_state(SYSTEM_STATE_ELETRIFY_TEST);
                update_state(SYSTEM_STATE_STORAGE);
            }
            else
            {
                NRF_LOG_INFO("m_system_state: SYSTEM_STATE_ELETRIFY\r\n");
                update_state(SYSTEM_STATE_ELETRIFY);
                update_state(SYSTEM_STATE_STORAGE);
            } 
            system_database.reset_reason = reset_error;
            #else
            M_SYSTEM_Print("m_system_state: SYSTEM_STATE_ELETRIFY\r\n");
            update_state(SYSTEM_STATE_ELETRIFY);
            update_state(SYSTEM_STATE_STORAGE);
            #endif
        }
    }
    else // 按键唤醒启动
    {
        M_SYSTEM_Print("m_system_state:Powering on!\r\n");
        #if 1
        err_code = app_timer_create(&system_state_timer_id, 
                                    APP_TIMER_MODE_REPEATED,
                                    m_system_state_timeout_handler);
        if (err_code == NRF_SUCCESS)
        {
            err_code = app_timer_start(system_state_timer_id, 
                                        APP_TIMER_TICKS(10u /* ms */), 
                                        NULL);
        }
        #endif
    }

    return err_code;
}

static void dfu_isched_event_handler(void *p_context)
{ 
    reset_total_info_t reset_total_info;
    memset(&reset_total_info,0x00,sizeof(reset_total_info));
    system_database.df_reset_cnt = system_database.df_reset_cnt + 1;
    reset_total_info.reset_cnt  = system_database.df_reset_cnt;
    reset_total_info.reset_type = 0x03;
    reset_total_info.reset_utc = system_database.utc_seconds;
    char str[4] = {'d','f','d','f'};
    uint16_t line_num = __LINE__;
    memcpy(&reset_total_info.debug_info,str,4);
    reset_total_info.debug_info[4] = (line_num >> 8) & 0xff;
    reset_total_info.debug_info[5] = line_num & 0xff;
    ret_code_t    err = w25x80_erase_sector(0x0004000);
   
    if (NRF_SUCCESS == err)
    {
        err = w25x80_write(0x0004000, (uint8_t *)&reset_total_info, sizeof(reset_total_info));
    }  
    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);
}



static void reset_isched_event_handler(void *p_context)
{
    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_RESET);
}

bool m_system_state_event_handler(const event_t *p_event)
{
    ASSERT(p_event != NULL);

    switch (p_event->type)
    {
        case EVT_BUTTON_PRESSED:
            if (BUTTON_EVT_SYSTEM_RESET == p_event->button.button_event)
            {
                update_state(SYSTEM_STATE_RESET);
            }
            else if (BUTTON_EVT_SYSTEM_RESUME_F_GREEN == p_event->button.button_event)
            {
                update_state(SYSTEM_STATE_RESUME_F_GREEN);
            }
            else if (BUTTON_EVT_SYSTEM_RESUME_F_SAVE == p_event->button.button_event)
            {
                update_state(SYSTEM_STATE_RESUME_F_SAVE);
            }        
            break;

        case EVT_SYSTEM_STATE:
            if (SYSTEM_STATE_DFU == p_event->system.system_state)
            {
                app_isched_event_put(&g_bg_scheduler, dfu_isched_event_handler, NULL);
            }
            else if (SYSTEM_STATE_RESET == p_event->system.system_state)
            {
                app_isched_event_put(&g_bg_scheduler, reset_isched_event_handler, NULL);
            }
            
            break;
            
        case EVT_TEST_MODE_OUT:
            update_state(SYSTEM_STATE_RESET);
            break;
        
        default:
            /* Ignore */
            break;
    }

    return false;
}
