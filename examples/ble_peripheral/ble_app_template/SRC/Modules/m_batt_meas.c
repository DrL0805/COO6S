/**
 * Copyright (c) 2016 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdint.h>

#include "nrf_drv_saadc.h"
#include "nrf_pwr_mgmt.h"
#include "app_debug.h"
#include "app_timer.h"

#include "event_bus.h"
#include "m_batt_meas.h"

#include "resources.h"
#include "sr3_config.h"
#include "m_database.h"

#include "includes.h"

#if CONFIG_RTT_BATT_ENABLED
#define BATT_Print(M, ...)	SEGGER_Print("BATT", M, ##__VA_ARGS__)
#else
#define BATT_Print(M, ...)
#endif

#if CONFIG_BATT_MEAS_ENABLED

#define NRF_LOG_MODULE_NAME m_batt_meas
#define NRF_LOG_LEVEL CONFIG_BATT_MEAS_MODULE_LOG_LEVEL
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Verify SDK configuration.
STATIC_ASSERT(SAADC_ENABLED);

#define M_BLE_STATE_INVALID                 0
#define M_BLE_STATE_DISABLED                1  //蓝牙处于关闭状态
#define M_BLE_STATE_ADVETISING              2  //蓝牙处于广播状态
#define M_BLE_STATE_CONNECTED               3  //蓝牙处于连接状态


#define ADC_CHANNEL     0
#define ADC_DIVIDER     6	// 1/6分压
#define ADC_REFERENCE   600 // 600mV
#define ADC_MAX_CONV    ((1 << 12) - 1)		// ADC采样最大值 12Bits

APP_TIMER_DEF           (m_lower_power_timer);
APP_TIMER_DEF           (m_batt_timer);                 /**< Battery measurement timer. */
//static uint8_t          m_batt_meas_prev_level = 255;   /**< Previous notified battery level. */
#if CONFIG_PWR_MGMT_ENABLED
static bool             m_batt_meas_going_down;         /**< True if module shutdown was requested. */
#endif



/* ADC是否已经初始化 */
static bool m_batt_is_run = false;

/* LED是否处于忙碌状态 */
static bool m_led_busy = false;

static uint8_t m_vibrator_busy = 0;


static uint8_t          m_batt_meas_prev_level = 255;
static bool             m_batt_meas_send_event_enabled = false;
//static bool             m_batt_meas_busy = false;

static uint8_t          m_batt_meas_lowpower_count = 0;
static bool saadc_evt_calibratedone_flag =false;


static uint8_t sadc_convert_cnt = 0;
static uint8_t m_ble_state = M_BLE_STATE_INVALID;

static bool sadc_convert_test_mode = false;
static bool get_batt_vol_flag = false;

extern uint16_t batt_vol;


#define SAADC_SAMPLE_CNT		16
typedef struct
{
	uint8_t		number;
	int16_t		saadc_value[SAADC_SAMPLE_CNT];
}saadc_sample_t;

saadc_sample_t	saadc_sample;



static void adc_pin_ctrl_on(void)
{

    nrf_gpio_pin_set(CONFIG_IO_VIBRATOR);
    if (CONFIG_IO_ADC_ENABLE_ACTIVE_HIGH)
    {
        nrf_gpio_pin_set(CONFIG_IO_ADC_ENABLE_PIN);
    }
    else
    {
        nrf_gpio_pin_clear(CONFIG_IO_ADC_ENABLE_PIN);
    }
    nrf_delay_ms(2);
}


static void adc_pin_ctrl_off(void)
{
//    BATT_Print("adc_pin_ctrl_off: m_vibrator_busy (%d)",m_vibrator_busy);
    if(!m_vibrator_busy)
    {
        nrf_gpio_pin_clear(CONFIG_IO_VIBRATOR); 
    }
     
    if (CONFIG_IO_ADC_ENABLE_ACTIVE_HIGH)
    {
        nrf_gpio_pin_clear(CONFIG_IO_ADC_ENABLE_PIN);
    }
    else
    {
        nrf_gpio_pin_set(CONFIG_IO_ADC_ENABLE_PIN);
    }


}

__STATIC_INLINE void adc_pin_ctrl_init(void)
{
    nrf_gpio_cfg_output(CONFIG_IO_ADC_ENABLE_PIN);
}


static void m_batt_sample_start(void)
{
	static nrf_saadc_value_t buffer[1];
	
    uint32_t err = nrf_drv_saadc_buffer_convert(buffer, 1);
    if( err == NRF_SUCCESS )
    {
        APP_ERROR_CHECK(nrf_drv_saadc_sample());
    }
    else if(err != NRF_ERROR_BUSY)
    {
        APP_ERROR_CHECK(err);
    }
}

static void m_batt_continue(void)
{
//    BATT_Print("m_batt_continue: (%d)",sadc_convert_cnt);
	if (++sadc_convert_cnt >= SAADC_SAMPLE_CNT)
	{ 
        adc_pin_ctrl_off();
	}
    else 
    {
        m_batt_sample_start();
    }
}

static uint16_t m_batt_filter(uint16_t *pValue, uint8_t len)
{
	uint8_t		i, j;
	uint32_t	value = 0;
	
	if (len < 5)
        return *pValue;
//		return 0xFFFF;
	
	
	for (i = 0; i < len - 1; i++)
	{
		for (j = 0; j < len - i - 1; j++)
		{
			if (pValue[j] < pValue[j+1])
			{
				pValue[j] ^= pValue[j+1];
				pValue[j+1] ^= pValue[j];
				pValue[j] ^= pValue[j+1];
			}
		}
	}
	
	
	// 去头去尾2个
	for (i = 2; i < len - 2; i++)
	{
		value += pValue[i];
	}
	
	value /= (len - 4);
	
	return value;
}

static void echo_batt_vol_power(uint16_t vol)
{
    static uint8_t cnt = 1;
    if (m_ble_state == M_BLE_STATE_CONNECTED)
    {
        for(int i =0 ;i< cnt;i++)
        {
            protocol_evt_t send_evt;            
            send_evt.header.evt_id = PROT_EVT_ID_ECHO_PROD_BATT_VOL;
            send_evt.header.evt_len = 2;
            send_evt.evt_params.dev_info.prod_batt_vol.batt_vol = __REV16(vol);
            protocol_send_evt(send_evt);
        }
    }
}

//************************************************************************************************************
// 功能描述:   	计算电量，并发出低电的呼喊
// 函数参数:    p_context	->	ADC寄存器原始值
// 返回值  :	无        
//************************************************************************************************************

/**@brief Process ADC data */
static void m_batt_meas_process(void *p_context)
{

    nrf_saadc_value_t measurement = *(nrf_saadc_value_t *)(p_context);

    uint8_t level = 0;
	
	if (measurement < 0)
		measurement = 0;
	
	saadc_sample.saadc_value[saadc_sample.number] = measurement;
    
 
    

    
	if (++saadc_sample.number >= SAADC_SAMPLE_CNT)
	{
		saadc_sample.number = 0;
		measurement = m_batt_filter((uint16_t *)saadc_sample.saadc_value, SAADC_SAMPLE_CNT);

		batt_vol = ((uint32_t)(measurement) * ADC_DIVIDER * ADC_REFERENCE) / ADC_MAX_CONV;
        
        BATT_Print("voltage: (%u mV)",batt_vol);
        
        #if CONFIG_RTT_DEBUG_ENABLED
        batt_vol = (batt_vol * 3 ) ;
        #else
        batt_vol = (batt_vol * 3 ) >> 1;
        #endif
//        
        if(get_batt_vol_flag)
        {
            echo_batt_vol_power(batt_vol);
            get_batt_vol_flag = false;
        }
        
        if(sadc_convert_test_mode)
        {
            APP_ERROR_CHECK(event_send(EVT_TEST_BATT_VOL_END, batt_vol));
        }
     

        if (batt_vol > 2500)
        {
            m_batt_meas_lowpower_count = 0;
            level = 95;
            system_database.battery_state = 1;
        }
        else if (batt_vol < 2000)
        {
            BATT_Print("voltage < CONFIG_BATT_MEAS_LOWPOWER_LEVEL : app_timer_start");
            m_batt_meas_lowpower_count++;
            level = 5;
            APP_ERROR_CHECK(app_timer_start(m_batt_timer,
                                            APP_TIMER_TICKS(1000u * 30),
                                            NULL)); 
        }
        else 
        {
            m_batt_meas_lowpower_count = 0;
            level = 50;
            system_database.battery_state = 1;
        }

        m_batt_meas_prev_level = level;
        system_database.battery_level = level;

        BATT_Print("m_batt_meas: Battery level: %u%% (%u mV)", level, batt_vol);

        if (m_batt_meas_send_event_enabled)
        {
            if (m_batt_meas_lowpower_count > 3)
            {
                m_batt_meas_lowpower_count = 0;
                system_database.battery_state = 0;
                BATT_Print("m_batt_meas: Low power: %dmv", batt_vol);

                APP_ERROR_CHECK(event_send(EVT_SYSTEM_BATTERY_LEVEL, 0));
//                APP_ERROR_CHECK(app_timer_stop(m_batt_timer));
                /*5分钟过后进入低功耗模式*/
                APP_ERROR_CHECK(app_timer_start(m_lower_power_timer,
                                            APP_TIMER_TICKS(1000u * 30),
                                            NULL));    
            }
        }	
    }
   
    m_batt_continue();      
}


static void m_lower_power_timeout_handler(void* p_context)
{   
    APP_ERROR_CHECK(event_send(EVT_SYSTEM_STATE, SYSTEM_STATE_LOWPOWER));
}



//************************************************************************************************************
// 功能描述:   	启动ADC采样
// 函数参数:    无意义
// 返回值  :	无        
//************************************************************************************************************
static void m_batt_meas_timeout_handler(void* p_context)
{

#if CONFIG_PWR_MGMT_ENABLED
    /*
     * Timeout handler might be called after shutdown, when the SAADC driver is already uninitialized.
     * In such case, the measurement cannot be perfomed.
     */
    if (m_batt_meas_going_down)
    {
        return;
    }
#endif

	// 蜂鸣器和LED都未工作时，才启动ADC采样，避免某些大功耗器件拉低当前电压
	if ((!m_vibrator_busy) && (!m_led_busy))
	{
		sadc_convert_cnt = 0;

        adc_pin_ctrl_on();
        m_batt_sample_start();
	}
    else
    {
        BATT_Print("m_batt_meas_timeout_handler:  app_timer_start");
        APP_ERROR_CHECK(app_timer_start(m_batt_timer,
                                            APP_TIMER_TICKS(1000u * CONFIG_BATT_MEAS_POLL_RATE),
                                            NULL));       
    }
}

//************************************************************************************************************
// 功能描述:   	ADC采样回调函数
// 函数参数:    p_event		->	ADC触发事件
// 返回值  :	无        
//************************************************************************************************************
static void m_batt_meas_saadc_event_handler(const nrf_drv_saadc_evt_t *p_event)
{
    switch (p_event->type)
    {
		// 校准完成
        case NRF_DRV_SAADC_EVT_CALIBRATEDONE:
            // Perform first measurement just after calibration.
            APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler,
                                                 m_batt_meas_timeout_handler,
                                                 NULL));

            // The following measurements will be done at regular intervals.
#if 0
            APP_ERROR_CHECK(app_timer_start(m_batt_timer,
                                            APP_TIMER_TICKS(1000u * 10),
                                            NULL));

#endif
            break;

		// 采样完成
        case NRF_DRV_SAADC_EVT_DONE:
            ASSERT(p_event->data.done.size == 1);
            APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler,
                                                 m_batt_meas_process,
                                                 p_event->data.done.p_buffer));
            break;

        default:
            /* Ignore */
            break;
    }
}


//************************************************************************************************************
// 功能描述:   	ADC初始化
// 函数参数:    无
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t m_batt_meas_init(void)
{
    nrf_saadc_channel_config_t adc_channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);   //NRF_SAADC_INPUT_AIN2  //NRF_SAADC_INPUT_VDD
    ret_code_t status;

	if (true == m_batt_is_run)
		return NRF_SUCCESS;

    // Verify the SAADC driver configuration.
    ASSERT(SAADC_CONFIG_RESOLUTION      == NRF_SAADC_RESOLUTION_12BIT);
    ASSERT(adc_channel_config.reference == NRF_SAADC_REFERENCE_INTERNAL);
    ASSERT(adc_channel_config.gain      == NRF_SAADC_GAIN1_6);

    // Enable Burst Mode if oversampling is enabled.
    adc_channel_config.burst = (SAADC_CONFIG_OVERSAMPLE != 0) ? NRF_SAADC_BURST_ENABLED :
                                                                NRF_SAADC_BURST_DISABLED;

    status = nrf_drv_saadc_init(NULL, m_batt_meas_saadc_event_handler);
    if (status != NRF_SUCCESS)
    {
        return status;
    }

    status = nrf_drv_saadc_channel_init(ADC_CHANNEL, &adc_channel_config);
    if (status != NRF_SUCCESS)
    {
        return status;
    }
    adc_pin_ctrl_init();
	// 创建采样任务
    status = app_timer_create(&m_batt_timer, APP_TIMER_MODE_SINGLE_SHOT, m_batt_meas_timeout_handler);
    if (status != NRF_SUCCESS)
    {
        return status;
    }
    status = app_timer_create(&m_lower_power_timer, APP_TIMER_MODE_SINGLE_SHOT, m_lower_power_timeout_handler);
    if (status != NRF_SUCCESS)
    {
        return status;
    }

    // 触发校准事件
    status =  nrf_drv_saadc_calibrate_offset();
    system_database.battery_state = 1;
	if (NRF_SUCCESS == status)
	{
		m_batt_is_run = true;
	}
    
	return status;
}


//************************************************************************************************************
// 功能描述:   	注销ADC功能
// 函数参数:    无意义
// 返回值  :	无        
//************************************************************************************************************
ret_code_t m_batt_meas_uninit(void)
{
	if (false == m_batt_is_run)
		return NRF_SUCCESS;

	app_timer_stop(m_batt_timer);
//	APP_ERROR_CHECK(nrf_drv_saadc_channel_uninit(ADC_CHANNEL));		//	此函数经常由于m_cb.adc_state != NRF_SAADC_STATE_IDLE，导致函数退出
	nrf_drv_saadc_uninit();
  
	m_batt_is_run = false;
   
	return NRF_SUCCESS;
}



//************************************************************************************************************
// 功能描述:   	接收其它模块产生的事件，摘取自己关心的事件处理
// 函数参数:    p_event		->	事件类型
// 返回值  :	event_bus调度事件是否继续下一个回调任务(true: 回调事件终止, false: 继续下一个回调事件)        
//************************************************************************************************************
bool m_batt_meas_event_handler(const event_t * p_event)
{
    if (p_event->type == EVT_SYSTEM_BT_CONN_STATE)
    {
        switch (p_event->system.data)
        {
            case BT_CONN_STATE_CONNECTED:
            BATT_Print("m_batt_timer: BT_CONN_STATE_CONNECTED");    
            m_ble_state = M_BLE_STATE_CONNECTED;
            break;

            case BT_CONN_STATE_DISABLED:
            BATT_Print("m_batt_timer: BT_CONN_STATE_DISABLED");
            m_ble_state = M_BLE_STATE_DISABLED;
            break;
        }
     }
    else if (p_event->type == EVT_SYSTEM_STATE)
    {
        if (p_event->system.system_state == SYSTEM_STATE_POWERON_FAIL) // 开机失败状态发送电量，以使 LED 模块能够指示
        {
            APP_ERROR_CHECK(event_send(EVT_SYSTEM_BATTERY_LEVEL, m_batt_meas_prev_level));
        }
        else if (p_event->system.system_state > SYSTEM_STATE_POWERON_FAIL) // 开机后才允许发送电量状态
        {
            m_batt_meas_send_event_enabled = true;
        }
        else
        {
            m_batt_meas_send_event_enabled = false;
        }
    }
    else if (p_event->type == EVT_SYSTEM_UTC_DAY)
    {
        if (saadc_evt_calibratedone_flag) 
        {
            APP_ERROR_CHECK(app_timer_start(m_batt_timer,
                                            APP_TIMER_TICKS(1000u * 30),
                                            NULL));
        }
    }
    else if (p_event->type == EVT_SYSTEM_VIBRATOR_STATE)
    {
         m_vibrator_busy = p_event->system.data;
    }
    else if (p_event->type == EVT_SYSTEM_LED_STATE)
    {
         m_led_busy = p_event->system.data;
    }
    
    else if (p_event->type == EVT_TEST_BATT_VOL_START)
    {
        sadc_convert_test_mode = true;
        APP_ERROR_CHECK(app_timer_start(m_batt_timer,APP_TIMER_TICKS(100),NULL));
    }
    
    else if (p_event->type == EVT_COMS_APP_RECV)
    {
        get_batt_vol_flag = true;
        if(p_event->coms.app_recv_event == APP_RECV_EVT_GET_BATT_VOL)
        {
            APP_ERROR_CHECK(app_timer_start(m_batt_timer,APP_TIMER_TICKS(100),NULL));
        }
    }
    
    return false;
}

#if CONFIG_PWR_MGMT_ENABLED
//************************************************************************************************************
// 功能描述:   	关机操作
// 函数参数:    event		->	关机事件类型
// 返回值  :	关机调度事件是否继续下一个回调任务(true: 继续下一步，直到关机, false: 终止此次关机)        
//************************************************************************************************************
static bool m_batt_meas_shutdown(nrf_pwr_mgmt_evt_t event)
{
    m_batt_meas_going_down = true;

	m_batt_meas_uninit();
    
    return true;
}
NRF_PWR_MGMT_HANDLER_REGISTER(m_batt_meas_shutdown, SHUTDOWN_PRIORITY_DEFAULT);
#endif /* CONFIG_PWR_MGMT_ENABLED */
#endif /* CONFIG_BATT_MEAS_ENABLED */
