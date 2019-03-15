
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "math.h"
#include "event_bus.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "app_debug.h"

#include "app_timer.h"

#include "m_database.h"
#include "app_time_slot.h"

#include "resources.h"
#include "sr3_config.h"
#include "drv_mag.h"

#include "m_mag.h"
#include "algorithm_abc.h"
#include "m_drv_qmc7983.h"
#include "includes.h"

#if CONFIG_GEOMAGNETIC_ENABLED

#if CONFIG_RTT_MAG_ENABLED
#define MAG_Print(M, ...)    SEGGER_Print("MAG", M, ##__VA_ARGS__)
#else
#define MAG_Print(M, ...)
#endif

#define COMPASS_MODE_TIMEOUT 30    // s
#define COMPASS_TIMEOUT      100   // ms

APP_TIMER_DEF(m_mag_timer_id);
APP_TIMER_DEF(m_mag_start_correct_timer_id);
APP_TIMER_DEF(m_mag_correct_timer_id);
APP_TIMER_DEF(m_compass_mode_timer_id);

uint32_t disable_mag_measure(void);
uint32_t enable_mag_measure(void);

static bool m_mag_init_flag = false;
static bool mag_measure_enable = false;
//static bool mag_measure_busy = false;




static void mag_compass_mode_timeout_timer_handler(void * p_context);


void mag_adjust_process(void);
void mag_compass_mode_process(void);



int16_t		mag_data[3]={0};

static CalParm_t cal_parm;                            /* 校准参数 */
#define ANGLE_BUF_CNT   15
static uint16_t angle_buf[ANGLE_BUF_CNT] = {0};
uint8_t abgle_buf_cnt = 0;


static void send_mag_measure_data(uint16_t data)  //发送mag测量数据
{
      static uint8_t buf[2];
      buf[0] = (data >> 8) & 0xff;
      buf[1] =  data & 0xff;
      protocol_send_data(buf,2);
}

static void send_mag_measure_raw_data(int16_t data[3])  //发送mag raw数据
{
    static uint8_t buf[6];
    buf[0] =  (data[0] >> 8) & 0xff;
    buf[1] =  data[0] & 0xff;
    buf[2] =  (data[1] >> 8) & 0xff;
    buf[3] =  data[1] & 0xff;
    buf[4] =  (data[2] >> 8) & 0xff;
    buf[5] =  data[2] & 0xff;
    protocol_send_data(buf,6);
}


uint32_t enable_mag_measure(void)
{
    MAG_Print("enable_mag_measure"); 
    uint32_t err_code = NRF_SUCCESS; 
//    Drv_Mag_WakeUp();
    mag_measure_enable = true;
    return err_code;
}

uint32_t disable_mag_measure(void)
{
    MAG_Print("disable_mag_measure"); 
    uint32_t err_code = NRF_SUCCESS;
//    Drv_Mag_GotoSleep();
    mag_measure_enable = false;
    return err_code;
}
/*开始校准程序*/
void mag_start_correct_timeout_timer_handler(void *p_text)
{
    mag_adjust_process();
}


/*定时获取地磁数据，计算为角度*/
void m_mag_timeout_handler(void *p_text)
{
    
    calState_e cal_state;
    uint16_t cur_angle;
    
    uint8_t i,j;
//    Drv_Mag_Read(mag_data);
    MAG_Print("x : %d ,y : %d ,z: %d\r\n",mag_data[0],mag_data[1],mag_data[2]);
    /*计算当前角度*/
    
//    send_mag_measure_raw_data(mag_data);
    
    cal_state = alg_compass_getAngle(mag_data,&cur_angle);
    
    
    angle_buf[abgle_buf_cnt] = cur_angle;
    if(++abgle_buf_cnt >= ANGLE_BUF_CNT)
    {
        for ( i = 0; i < ANGLE_BUF_CNT - 1; i++)
        {
            for ( j = 0; j < ANGLE_BUF_CNT - i - 1; j++)
            {
                if (angle_buf[j] < angle_buf[j+1])
                {
                    angle_buf[j]   ^= angle_buf[j+1];
                    angle_buf[j+1] ^= angle_buf[j];
                    angle_buf[j]   ^= angle_buf[j+1];
                }
            }
        }
        // 去头去尾2个
        for (i = 2; i < ANGLE_BUF_CNT - 2; i++)
        {
            cur_angle += angle_buf[i];
        }	
	    cur_angle /= (ANGLE_BUF_CNT - 4);
        abgle_buf_cnt = 0 ;
    }
    
    if(cal_state == COMPASS_CAL_DONE)
    {
        MAG_Print("angle : %d\r\n",cur_angle);
        event_send(EVT_SYSTEM_MAG_COMPASS_ANGLE,cur_angle);
//        send_mag_measure_data(cur_angle);
    }
}
    
/*定时获取地磁数据，进行地磁参数校准*/
void m_mag_correct_timeout_handler(void *p_text)
{
//	Drv_Mag_Read(mag_data);
	uint32_t ret_code ;
    static uint16_t cnt = 0;
    MAG_Print("correct handler : x : %d ,y : %d ,z: %d\r\n",mag_data[0],mag_data[1],mag_data[2]);
    
    send_mag_measure_raw_data(mag_data);
    ret_code = alg_compass_calibration(mag_data,(uint8_t *)&cal_parm);
    
    cnt++;
    if(cnt % 25 == 0)
    {
        MAG_Print("EVT_SYSTEM_MAG_ADJUSTING");
        event_send(EVT_SYSTEM_MAG_ADJUSTING,NULL);
    }
    
    MAG_Print("alg_compass_calibration: ret_code %d,\r\n",ret_code);
    if(ret_code == COMPASS_CAL_DONE)
    {
        MAG_Print("EVT_SYSTEM_MAG_COMPLETE_ADJUST");
        event_send(EVT_SYSTEM_MAG_COMPLETE_ADJUST,NULL);
        /*关闭校准定时*/
        app_timer_stop(m_mag_correct_timer_id);
        /*开启定时测量角度*/
        APP_ERROR_CHECK(app_timer_start(m_mag_timer_id, APP_TIMER_TICKS(COMPASS_TIMEOUT * 1), NULL));
        /*打开指北针模式超时定时*/
        APP_ERROR_CHECK(app_timer_start(m_compass_mode_timer_id, APP_TIMER_TICKS(COMPASS_MODE_TIMEOUT * 1000), NULL));  
    }    
    else if(ret_code == COMPASS_CAL_FAIL)
    {
        app_timer_stop(m_mag_correct_timer_id);
        event_send(EVT_SYSTEM_MAG_COMPASS_EXIT,NULL);
        if(mag_measure_enable)
        {
            APP_ERROR_CHECK(disable_mag_measure());
        }
    }

}

/*
指北针模式超时退出函数
*/
void mag_compass_mode_timeout_timer_handler(void *p_text)
{
     MAG_Print("mag_compass_mode_timeout_timer_handler");
    /*关闭定时测量角度*/
    app_timer_stop(m_mag_timer_id);
    /*退出指北针模式*/
    event_send(EVT_SYSTEM_MAG_COMPASS_EXIT,NULL);
    if(mag_measure_enable)
    {
        APP_ERROR_CHECK(disable_mag_measure());
    }
    abgle_buf_cnt = 0;
}
    

bool m_mag_event_handler(const event_t *p_event)  
{
//    uint32_t err_code = NRF_SUCCESS;
    if (!m_mag_init_flag)
    {
        return false;
    }
    if (EVT_SYSTEM_STATE == p_event->type)
    {
        if (p_event->system.data == SYSTEM_STATE_LOWPOWER)
        {
            if(mag_measure_enable)
            {
                APP_ERROR_CHECK(disable_mag_measure());
            }
        }       
    }
    else if (EVT_SYSTEM_MAG_COMPASS_ENTER == p_event->type)
    {
       mag_compass_mode_process();
    }
    else if (EVT_COMS_APP_RECV == p_event->type)
    {
        switch (p_event->coms.app_recv_event)
        {
            default:
                break;
        }
    }
    
    else if (EVT_BUTTON_PRESSED == p_event->type)
    {
        switch (p_event->button.button_event)
        {
            case BUTTON_EVT_ADJUST_COMPASS:
                 MAG_Print("BUTTON_EVT_ADJUST_COMPASS"); 
                 mag_adjust_process();
                 /*手动校准，先关闭指北针超时模式定时器*/
                 app_timer_stop(m_compass_mode_timer_id);
                break;
            
            case BUTTON_EVT_EXIT_COMPASS:
                 MAG_Print("BUTTON_EVT_EXIT_COMPASS"); 
                 app_timer_stop(m_mag_timer_id);
                break;
            
            default:
                break;
        }
    }

    return false;
}



/*地磁校准函数*/
void mag_adjust_process(void)
{
    if(!mag_measure_enable)
    {
        APP_ERROR_CHECK(enable_mag_measure());
    }
    alg_compass_init(NULL);
    alg_compass_calibration_init();
//    Drv_Mag_Set(MAG_DATA_RATE_50HZ,MAG_SCALE_RANGE_8GS);
    APP_ERROR_CHECK(app_timer_start(m_mag_correct_timer_id, APP_TIMER_TICKS(40), NULL));
}

/*指北模式函数*/
void mag_compass_mode_process(void)
{
    MAG_Print("mag_compass_process");
    if(!mag_measure_enable)
    {
        APP_ERROR_CHECK(enable_mag_measure());
        mag_measure_enable = true;
    }
    /*已经校准*/
    if(get_cal_flag())
    {    
        /*开启定时测量角度*/
        APP_ERROR_CHECK(app_timer_start(m_mag_timer_id, APP_TIMER_TICKS(COMPASS_TIMEOUT * 1), NULL));
        /*开启指北针模式超时定时 30s*/
        APP_ERROR_CHECK(app_timer_start(m_compass_mode_timer_id, APP_TIMER_TICKS(COMPASS_MODE_TIMEOUT * 1000), NULL));   
    }
    else
    {
        event_send(EVT_SYSTEM_MAG_ADJUST,NULL);
        /*5s后开始校准*/
        APP_ERROR_CHECK(app_timer_start(m_mag_start_correct_timer_id, APP_TIMER_TICKS(5 * 1000), NULL));
    }
}



uint32_t m_mag_init(void)
{    
    uint32_t err_code = NRF_SUCCESS;
//    APP_ERROR_CHECK(Drv_Mag_Open());
//    APP_ERROR_CHECK(Drv_Mag_GotoSleep());
    
    drv_mag_init();
    drv_mag_sleep();
    
	APP_ERROR_CHECK(app_timer_create(&m_mag_timer_id, APP_TIMER_MODE_REPEATED, m_mag_timeout_handler));
    APP_ERROR_CHECK(app_timer_create(&m_mag_correct_timer_id, APP_TIMER_MODE_REPEATED, m_mag_correct_timeout_handler));
    APP_ERROR_CHECK(app_timer_create(&m_compass_mode_timer_id, APP_TIMER_MODE_SINGLE_SHOT, mag_compass_mode_timeout_timer_handler));
    APP_ERROR_CHECK(app_timer_create(&m_mag_start_correct_timer_id, APP_TIMER_MODE_SINGLE_SHOT, mag_start_correct_timeout_timer_handler));
    
    
    m_mag_init_flag = true;
    return err_code;
}

uint32_t m_mag_uninit(void)
{    
//    Drv_Mag_Close();
    m_mag_init_flag = false;
    return NRF_SUCCESS;
}



bool m_mag_wakeup_prepare(nrf_pwr_mgmt_evt_t shutdown_type)
{
    if(mag_measure_enable)
    {
        APP_ERROR_CHECK(disable_mag_measure());
    }
    m_mag_uninit();
    return true;
}


//NRF_PWR_MGMT_HANDLER_REGISTER(m_mag_wakeup_prepare, SHUTDOWN_PRIORITY_DEFAULT);
#endif

