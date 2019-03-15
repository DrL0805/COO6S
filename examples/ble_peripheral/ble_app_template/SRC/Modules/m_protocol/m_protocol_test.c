
#include <stdint.h>             /* C 库头文件 */
#include <stdbool.h>

#include "sr3_config.h"       /* 资源配置、底层资源头文件 */
#include "nrf_assert.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"
#include "app_timer.h"
#include "event_bus.h"
       /* 模块头文件 */
#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_test.h"
#include "app_isched.h"
#include "resources.h"
#include "m_drv_w25x80.h"
#include "m_drv_kx022.h"
#include "drv_mag.h"
#include "nrf_delay.h"

#include "m_drv_led.h"
#include "drv_vibrator.h"
#include "nrf_gpio.h"
#include "drv_watch_hand.h"
#include "includes.h"


#if 0
#define ProtoTest_Print(M, ...)    SEGGER_Print("ProtocolTest", M, ##__VA_ARGS__)
#else
#define ProtoTest_Print(M, ...)
#endif

#define TEST_FLASH      0x01
#define TEST_GENSOR     0x02
#define TEST_BLE        0x08
#define TEST_MAG        0x0D

#define TEST_LED        0x05
#define TEST_MOTOR      0x06
#define TEST_KEY        0x07
#define TEST_MOVT       0x09
#define TEST_BATT_ADC   0x10


APP_TIMER_DEF(test_mode_out_timeout_timer_id);   //退出测试模式定时器
APP_TIMER_DEF(led_test_timeout_timer_id);        //led测试模式定时器

static bool m_test_init = false;
static bool m_is_ble_connected = false;
static bool m_test_mode_en = false;
static bool m_test_key_en = false;

//static uint32_t m_test_key_times = 0;


drv_led_config_t drv_led_config ={0};

drv_wh_config_t  wh_config = {0, DRV_WH_DIR_FORWARD};


  


static void echo_test_result(uint8_t test_item,uint8_t submodule ,uint8_t result)
{
    ProtoTest_Print("echo_test_result");
    if (!m_is_ble_connected) return;
 
    protocol_evt_t send_evt;
                
    send_evt.header.evt_id = PROT_EVT_ID_ECHO_AUTO_TEST;  //
    send_evt.header.evt_len = 3; 

    send_evt.evt_params.test_mode.test_mode_auto_test.echo_param.test_item  = test_item;
    send_evt.evt_params.test_mode.test_mode_auto_test.echo_param.test_submodule = submodule;
    send_evt.evt_params.test_mode.test_mode_auto_test.echo_param.result = result;

    protocol_send_evt(send_evt);
}



static void echo_test_key_result(uint8_t key_id)
{
    
    static uint32_t key0_times =0;
    static uint32_t key1_times =0;
    static uint32_t key2_times =0;
    
    
    uint32_t key_times;
    
    
    
    if(key_id == 0)
    {
        key0_times++;
        key_times = key0_times;
    }
    else if(key_id == 1)
    {
        key1_times++;
        key_times = key1_times;
    }
    else if(key_id == 2)
    {
        key2_times++;
        key_times = key2_times;
    }
    ProtoTest_Print("echo_test_key_result %d  %d" ,key_id,key_times);
    
    if (!m_is_ble_connected) return;
   
    protocol_evt_t send_evt;
                
    send_evt.header.evt_id = PROT_EVT_ID_ECHO_KEY_TEST;  //
    send_evt.header.evt_len = 5; 

    send_evt.evt_params.test_mode.test_mode_test_key.echo_param.key_id  = key_id;
    send_evt.evt_params.test_mode.test_mode_test_key.echo_param.key_times = __REV(key_times);

    protocol_send_evt(send_evt);

}

static uint32_t test_flash(void)
{
    uint32_t     nrf_err = NRF_SUCCESS;
    uint8_t      uuid_buf[8]={0};
    uint8_t      i;

    ProtoTest_Print("Auto Test: flash");

    if (NRF_SUCCESS == nrf_err)
    {
        nrf_err = NRF_ERROR_INVALID_DATA;

        w25x80_read_uuid(uuid_buf);

        for (i = 0; i < 8; i++)
        {
            ProtoTest_Print("uuid_buf: %d",uuid_buf[i]);
            if (uuid_buf[i] != 0)
            {
                
                nrf_err = NRF_SUCCESS;
                break;
            }
        }
    }
    if(!w25x80_test())
    {
        return NRF_ERROR_INTERNAL;
    }
    return nrf_err;
}

static uint32_t test_gsensor(void)
{
    if (UpdataGsensorData())
    {
        return NRF_SUCCESS;
    }
    else
    {
        for(int i =0;i<4;i++)
        {
            nrf_delay_ms(10);
            if (UpdataGsensorData())
            {
                return NRF_SUCCESS;
            }
        }
        return NRF_ERROR_INTERNAL;
    }
}

static void test_list_sched_handler(void * p_context)
{
      
    protocol_evt_t send_evt;
    
    memset(&send_evt,0x00,sizeof(send_evt));
                 
    send_evt.header.evt_id = PROT_EVT_ID_RET_TEST_LIST;
    send_evt.header.evt_len = 11;
    
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.pack_index = 0x00;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[0] = TEST_LED;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[1] = 0x01;

    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[0] = TEST_LED;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[1] = 0x02;

    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[2] = TEST_MOTOR;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[3] = 0x00;

    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[4] = TEST_MOVT;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[5] = 15;

    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[6] = TEST_MOVT;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[7] = 27;
            
    protocol_send_evt(send_evt);
    
    
    memset(&send_evt,0x00,sizeof(send_evt));
                 
    send_evt.header.evt_id = PROT_EVT_ID_RET_TEST_LIST;
    send_evt.header.evt_len = 11;
    
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.pack_index = 0xFF;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[0] = TEST_KEY;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[1] = 0x01;

    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[0] = TEST_KEY;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[1] = 0x02;

    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[2] = TEST_KEY;
    send_evt.evt_params.test_mode.test_mode_test_list.ret_param.test_list[3] = 0x03;
         
    protocol_send_evt(send_evt);
}

static void test_mode_auto_test_sched_handler(void * p_context)
{
       uint8_t result;
    
       if(NRF_SUCCESS == test_gsensor())
       {
           result = 0;
       }
       else
       {
           result = 1;
       }
       echo_test_result(TEST_GENSOR,0,result); 
       
       if(NRF_SUCCESS == test_flash())
       {
           result = 0;
       }
       else
       {
           result = 1;
       }
       echo_test_result(TEST_FLASH,0,result);

//       if(NRF_SUCCESS == Drv_Mag_SelfTest())
       {
           result = 0;
       }
//       else
       {
           result = 1;
       }
       echo_test_result(TEST_MAG,0,result); 
       
       echo_test_result(TEST_BLE,0,0); 

       APP_ERROR_CHECK(event_send(EVT_TEST_BATT_VOL_START,NULL));
   
}

static void test_mode_out_timeout_timer_handler(void * p_context)
{
//    event_send(EVT_TEST_MODE_OUT,NULL);
    event_send(EVT_BUTTON_PRESSED,BUTTON_EVT_SYSTEM_RESET);
}


static void test_mode_timeout_timer_handler(void * p_context)
{
//    DBG0_EVT0("mode_timeout_timer_handler");
}


static void led_test_timeout_timer_handler(void * p_context)
{
//    DBG0_EVT0("led_test_timeout_timer_handler");
    static uint8_t flag = 0;
    if(flag)
    {
        drv_led_config.led = 0;
    }
    else
    {
        drv_led_config.led = 1;
    }
    APP_ERROR_CHECK(drv_led_start(drv_led_config));
    flag = !flag;
}

bool m_protocol_test_event_handler(const event_t * p_event)
{
    ASSERT(m_test_init);
    uint32_t vol;
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
            if(m_test_key_en)
            {
                if (BUTTON_EVT_BTN0_SHORT_REL == p_event->button.button_event)
                {
                    echo_test_key_result(0x01); //3h                        
                }
                else if (BUTTON_EVT_BTN1_SHORT_REL == p_event->button.button_event)
                {
                    echo_test_key_result(0x00);  //2h                          
                }
                else if (BUTTON_EVT_BTN2_SHORT_REL == p_event->button.button_event)
                {
                    echo_test_key_result(0x02);  //4h                           
                }
            }
            break;
            
        case EVT_TEST_BATT_VOL_END:
            vol = p_event->test.test_data;
            ProtoTest_Print("EVT_TEST_BATT_VOL_END %d",vol);
            if(vol > 2800)
            {
                
                echo_test_result(TEST_BATT_ADC,0,0); 
            }
            else
            {
                echo_test_result(TEST_BATT_ADC,0,2); 
            }
            echo_test_result(0x00,0x00,0x00); /*自动测试结束*/
            break;
            
#include "m_drv_qmc7983.h"
        case EVT_COMS_APP_RECV :
            if(p_event->coms.app_recv_event == APP_RECV_EVT_PHOTO_EXIT_MODE)
            {
                drv_mag_init();
            }
            break;
        
        default:
            break;

    }

    return false;
}

bool m_protocol_test_prot_evt_execution(protocol_evt_t * p_evt)
{
    ASSERT(m_test_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_TEST_MODE)
    {
        return false; // 不属于 test 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // 默认返回找不到接口的帧错误

    ProtoTest_Print("<***** link evt_id 0x%04x *****>", p_evt->header.evt_id);
    
    uint8_t led_num;
    uint8_t led_switch;
    uint8_t led_switch_time;
    uint8_t movt_action;
    uint8_t movt_id;

    switch (p_evt->header.evt_id)
    {     
        case PROT_EVT_ID_CALL_TEST_MODE_EN:
            ProtoTest_Print("PROT_EVT_ID_CALL_TEST_MODE_EN");
             m_test_mode_en = p_evt->evt_params.test_mode.test_mode_en.call_param.test_mode_en;
             if(m_test_mode_en)
             {
                 event_send(EVT_TEST_MODE_IN,NULL);                 
             }
             else
             {
                 app_timer_stop(led_test_timeout_timer_id);
                 
                 APP_ERROR_CHECK(app_timer_start(test_mode_out_timeout_timer_id,
                                    APP_TIMER_TICKS(500),
                                    NULL));
             }
             
            p_evt->header.evt_id = PROT_EVT_ID_ECHO_TEST_MODE_EN;
            p_evt->header.evt_len = 2;
            p_evt->evt_params.test_mode.test_mode_en.ret_param.test_mode_en = m_test_mode_en;
            p_evt->evt_params.test_mode.test_mode_en.ret_param.result = 0x01; //succeed
            ack_state = ACK_STATE_INVALID;
            
            break;
        case PROT_EVT_ID_CALL_AUTO_TEST:
            ProtoTest_Print("PROT_EVT_ID_CALL_AUTO_TEST");
            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, test_mode_auto_test_sched_handler, NULL));
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_CALL_SUBJECT_TEST:
            ack_state = ACK_STATE_SUCCESS;
            break;

        case PROT_EVT_ID_GET_TEST_LIST:
            ProtoTest_Print("PROT_EVT_ID_GET_TEST_LIST");     
            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, test_list_sched_handler, NULL));
            break;

        case PROT_EVT_ID_CALL_LED_TEST:
            ProtoTest_Print("PROT_EVT_ID_CALL_LED_TEST");
            
            led_num = p_evt->evt_params.test_mode.test_mode_test_led.call_param.led_num;
            led_switch = p_evt->evt_params.test_mode.test_mode_test_led.call_param.state_switch;
            led_switch_time = p_evt->evt_params.test_mode.test_mode_test_led.call_param.switch_time;
            
            if(led_num != 0 )
            {
                drv_led_config.led = led_num -1;
                if(led_switch == 0)
                {
                    drv_led_config.on = led_switch_time*10;
                    drv_led_config.off = led_switch_time*10;
                    drv_led_config.count = 0;
                    APP_ERROR_CHECK(drv_led_start(drv_led_config));
                }
                else if(led_switch == 1)
                {
                    drv_led_config.on = 0;
                    drv_led_config.off = 100;
                    drv_led_config.count = 0;
                    APP_ERROR_CHECK(drv_led_start(drv_led_config));
                    
                }
                else if(led_switch == 2) 
                {
                    drv_led_config.on = 100;
                    drv_led_config.off = 0;
                    drv_led_config.count = 0;
                    APP_ERROR_CHECK(drv_led_start(drv_led_config));                    
                }
            }
            else
            {
                drv_led_config.on = 25;
                drv_led_config.off = 25;
                drv_led_config.count = 1;
                ProtoTest_Print("on %d  off%d",drv_led_config.on,drv_led_config.off);
                APP_ERROR_CHECK(app_timer_start(led_test_timeout_timer_id,APP_TIMER_TICKS(500),NULL));                
            }
            ack_state = ACK_STATE_SUCCESS;
            break;
            
        case PROT_EVT_ID_CALL_MOTOR_TEST:
            if(p_evt->evt_params.test_mode.test_mode_test_motor.call_param.motor_en)
            {
                nrf_gpio_pin_set(CONFIG_IO_VIBRATOR);
            }
            else
            {
                nrf_gpio_pin_clear(CONFIG_IO_VIBRATOR);
            }
            ack_state = ACK_STATE_SUCCESS;
            break;
        case PROT_EVT_ID_CALL_MOVT_TEST:
            
            movt_action = p_evt->evt_params.test_mode.test_mode_test_movt.call_param.movt_action;
            movt_id = p_evt->evt_params.test_mode.test_mode_test_movt.call_param.movt_id;
//            movt_speed = p_evt->evt_params.test_mode.test_mode_test_movt.call_param.movt_speed;
//            movt_steps = __REV16(p_evt->evt_params.test_mode.test_mode_test_movt.call_param.movt_steps);
            ProtoTest_Print("movt_id %d",movt_id);
            if(movt_action == 0)
            {
                 if(movt_id == 27)
                 {
                     event_send(EVT_TEST_MOVT_HAND_1_FORWARD,NULL);
                 }
                 else if(movt_id == 15)
                 {
                     event_send(EVT_TEST_MOVT_HAND_0_FORWARD,NULL);
                 }
            }          
            ack_state = ACK_STATE_SUCCESS;
            break;
        case PROT_EVT_ID_CALL_KEY_TEST:
            m_test_key_en = true;
            ack_state = ACK_STATE_SUCCESS;
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



   
uint32_t m_protocol_test_init(void)
{
    uint32_t status = NRF_SUCCESS;

    m_test_init = true;
    status = app_timer_create(&test_mode_out_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, test_mode_out_timeout_timer_handler);
    if(NRF_SUCCESS == status)
    {
        status = app_timer_create(&led_test_timeout_timer_id, APP_TIMER_MODE_REPEATED, led_test_timeout_timer_handler);
    }

    return status;
}


