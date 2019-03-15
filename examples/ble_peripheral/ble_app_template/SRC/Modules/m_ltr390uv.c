
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


#include "event_bus.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"


#include "app_timer.h"

#include "m_database.h"
#include "app_time_slot.h"

#include "resources.h"
#include "sr3_config.h"
#include "drv_sw_i2c_uv.h"
#include "drv_ltr_390uv.h"
#include "m_ltr390uv.h"
#include "includes.h"


#if CONFIG_RTT_FLASH_OP_ENABLED
#define UV_Print(M, ...)    SEGGER_Print("M_UV", M, ##__VA_ARGS__)
#else
#define UV_Print(M, ...)
#endif


#if CONFIG_UV_ENABLED

APP_TIMER_DEF(uv_data_update_timer_id);
APP_TIMER_DEF(uv_measure_timerout_id);


uint32_t disable_uv_measure(void);
uint32_t enable_uv_measure(void);

static bool m_uv_init_flag = false;
static bool uv_measure_enable = false;
static bool uv_measure_busy = false;

typedef struct uv_reg_data_t{
    uint8_t uvs_data0;
    uint8_t uvs_data1; 
    uint8_t uvs_data2;     
}uv_reg_data;

static uv_reg_data uv_measure_buf[5]={0};

static void uv_data_update_timeout_timer_handler(void * p_context);
static void uv_measure_timeout_timer_handler(void * p_context);


uint32_t m_uv_init(void)
{    
    uint32_t err_code = NRF_SUCCESS;
    drv_ltr_390uv_init();
    m_uv_init_flag = true;
    err_code = app_timer_create(&uv_data_update_timer_id, APP_TIMER_MODE_REPEATED, uv_data_update_timeout_timer_handler);
    if(err_code == NRF_SUCCESS)
    {
        return app_timer_create(&uv_measure_timerout_id, APP_TIMER_MODE_SINGLE_SHOT, uv_measure_timeout_timer_handler);
    }
    return err_code;
}

uint32_t m_uv_uninit(void)
{    
    drv_ltr_390uv_uninit();
    m_uv_init_flag = false;
    return NRF_SUCCESS;
}

#define swop(x,y)                       \
do{                                     \
    uint32_t temp =0;                   \
    temp = x;                           \
    x = y;                              \
    y =temp;                            \
} while(0)


static void send_uv_measure_data(uint16_t data)  //发送uv测量数据
{


    protocol_evt_t send_evt;

    send_evt.header.evt_id = PORT_EVT_ID_ECHO_UV_MEASURE;
    send_evt.header.evt_len = 6;
    send_evt.evt_params.sensor_data.measure_data.vis_measure_data = __REV16(0xFFFF);
    send_evt.evt_params.sensor_data.measure_data.ir_measure_data  = __REV16(0xFFFF);
    send_evt.evt_params.sensor_data.measure_data.uv_measure_data  = __REV16(data);

    protocol_send_evt(send_evt);
}



static void send_uv_measure_index_data(uint8_t data)  //发送uv测量数据
{
    protocol_evt_t send_evt;
    send_evt.header.evt_id = PORT_EVT_ID_ECHO_UV_MEASURE_INDEX;
    send_evt.header.evt_len = 1;
    send_evt.evt_params.sensor_data.uv_measure_index = data;
    protocol_send_evt(send_evt);
}




static void uv_update_sched_event_handler(void * p_context)
{
    uint32_t temp_buf[5]={0};
    for(int i =0;i<5;i++)
    {
        temp_buf[i] = (uv_measure_buf[i].uvs_data2 * 65536) + (uv_measure_buf[i].uvs_data1 * 256) + uv_measure_buf[i].uvs_data0;
    }
    /******************/    
    /*找出数组中最大的三个数   排挤法，时间复杂度O(n),*/
    /*预设数组中最大的3个数的下标为0,1,2*/
    uint8_t k1 = 0;
    uint8_t k2 = 1;
    uint8_t k3 = 2;
    for(int i=0; i<5; i++)
    { 

        if (temp_buf[i] >= temp_buf[k3])  /*当前数大于第3个最大的数*/
        { 
            if (temp_buf[i] >= temp_buf[k2]) /*当前数大于第2个最大的数*/
            { 
                if (temp_buf[i] >= temp_buf[k1]) /*当前数大于第1个最大的数*/
                { 
                    /*交换下标，最大数下标为i，第二大第三大的数下标替换为原来的第一和第二，挤出最小的数*/
                    k3 = k2; 
                    k2 = k1;
                    k1 = i; 
                }
                else 
                { 
                    k3 = k2; 
                    k2 = i; 
                } 
            }
            else
            { 
                k3 = i;
            }
         }
    }/************/
    uint16_t temp = (temp_buf[k1] +  temp_buf[k2] +  temp_buf[k3])/3 ;
    send_uv_measure_data(temp);
    /*
    18x 20bit  : uv source irradiance(uw/cm2) =  uv count / 100  (uw/cm2)
    uv index = mw/m2 * 0.04 = (uv count / 100)*10mw /m2 * 0.04
    1uw / cm^2 =10^4 uw / m^2 = 10mw /m2
    */
    uint8_t uv_index = temp / 261; //250 fix to 261
    UV_Print("uv_measure : uv_index %d ",uv_index);
    send_uv_measure_index_data(uv_index);
}


static void uv_measure_timeout_timer_handler(void * p_context)
{
    disable_uv_measure();
}
    
static void uv_data_update_timeout_timer_handler(void * p_context)
{
	 uint32_t ret;
     static uint8_t cnt = 0;
     if(cnt < 5)
     {
        ret = ltr_390uv_read_uv_data((uint8_t *)&uv_measure_buf[cnt]);
        UV_Print("uv_measure %d %d %d",uv_measure_buf[cnt].uvs_data0,uv_measure_buf[cnt].uvs_data1,uv_measure_buf[cnt].uvs_data2);
        cnt++;
     }
     else
     {
         cnt = 0;
         disable_uv_measure();
         APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, uv_update_sched_event_handler, NULL));
     }
}


uint32_t enable_uv_measure(void)
{
    if(uv_measure_enable == true)
    {
        return NRF_SUCCESS;
    }
    if (!m_uv_init_flag)
    {
        return NRF_SUCCESS;
    }
    uint32_t err_code = NRF_SUCCESS;
    
    err_code = ltr_390uv_enable();
    if(err_code!=NRF_SUCCESS)
    {
        UV_Print("ltr_390uv_enable %d",err_code); 
        return err_code;
    }
    uv_measure_enable = true;
    err_code = app_timer_start(uv_data_update_timer_id,APP_TIMER_TICKS(500),NULL);
    if(err_code == NRF_SUCCESS)
    {
        return app_timer_start(uv_measure_timerout_id,APP_TIMER_TICKS(5000),NULL);  //
    }
    return err_code;
}

uint32_t disable_uv_measure(void)
{
    UV_Print("disable_uv_measure"); 
    uv_measure_enable = false;
    uint32_t err_code = NRF_SUCCESS;
    if (!m_uv_init_flag)
    {
        return err_code;
    }
    err_code = app_timer_stop(uv_data_update_timer_id);
    if(err_code == NRF_SUCCESS)
    {
        err_code = app_timer_stop(uv_measure_timerout_id);
    }
    if(err_code == NRF_SUCCESS)
    {
        return ltr_390uv_disable();
    }
    return err_code;
}

bool m_uv_event_handler(const event_t *p_event)  
{
    uint32_t err_code = NRF_SUCCESS;
    if (!m_uv_init_flag)
    {
        return false;
    }
    if (p_event->type == EVT_SYSTEM_STATE)
    {
        if (p_event->system.data == SYSTEM_STATE_LOWPOWER)
        {
            if(uv_measure_enable)
            {
                APP_ERROR_CHECK(disable_uv_measure());
            }
        }
    }

    else if (EVT_COMS_APP_RECV == p_event->type)
    {
        switch (p_event->coms.app_recv_event)
        {
            case APP_RECV_EVT_UV_MEASURE_START:                
                 err_code = enable_uv_measure();
                 UV_Print("enable_uv_measure %d",err_code); 
                break;

            default:
                break;
        }
    }

    return false;
}    

bool m_uv_wakeup_prepare(nrf_pwr_mgmt_evt_t shutdown_type)
{
    UV_Print(" %s \r\n", __func__);
    if(uv_measure_enable)
    {
        APP_ERROR_CHECK(disable_uv_measure());
    }
    m_uv_uninit();
    return true;
}
NRF_PWR_MGMT_HANDLER_REGISTER(m_uv_wakeup_prepare, SHUTDOWN_PRIORITY_DEFAULT);

#endif

