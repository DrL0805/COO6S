
#include <stdint.h>             /* C 库头文件 */
#include <stdbool.h>
#include <string.h>

#include "sr3_config.h"       /* 资源配置、底层资源头文件 */
#include "nrf_assert.h"
#include "app_timer.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"
#include "utc.h"
#include "app_alarm_clock.h"
#include "AD_DataStore.h"

#include "app_isched.h"
#include "resources.h"
#include "event_bus.h"

#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_devinfo.h"

#include "includes.h"

#if CONFIG_RTT_PROTOCOL_DEVINFO_ENABLED
#define P_DevInfo_Print(M, ...)	SEGGER_Print("P_DevInfo", M, ##__VA_ARGS__)
#else
#define P_DevInfo_Print(M, ...)
#endif


#define HOUR_SYSTEM             12
#define WATCH_HAND_POS_MAX      (HOUR_SYSTEM * 3600) 


static const uint8_t SEND_EVT_PROD_SN = 1;
static const uint8_t SEND_EVT_PROD_MANAGER_SN = 2;

APP_TIMER_DEF(devinfo_timeout_timer_id);
//APP_TIMER_DEF(send_alarm_triggerd_timeout_timer_id);

static bool m_is_ble_connected = false;
static bool m_devinfo_init = false;
static bool m_auth_mode = false;
static bool m_photo_mode = false;
static bool m_save_mode = false;

 uint16_t batt_vol;


static utc_time_t utcbytes_to_utc(uint8_t * const utc_bytes)
{
    utc_time_t utc_time;

    utc_time.year   = BCD_TO_DEC(utc_bytes[0]) + 2000;
    utc_time.month  = BCD_TO_DEC(utc_bytes[1]);
    utc_time.day    = BCD_TO_DEC(utc_bytes[2]);
    utc_time.hour   = BCD_TO_DEC(utc_bytes[3]);
    utc_time.minute = BCD_TO_DEC(utc_bytes[4]);
    utc_time.second = BCD_TO_DEC(utc_bytes[5]);

    return utc_time;
}
static void utc_to_utcbytes(utc_time_t utc_time,uint8_t *utc_bytes)
{
    P_DevInfo_Print("utc: %02x/%02x/%02x %02x:%02x:%02x",utc_time.year,utc_time.month,utc_time.day,utc_time.hour,utc_time.minute,utc_time.second);
    if(utc_time.year>2000)
    {
       utc_time.year -= 2000; 
    }
    else
    {
       utc_time.year -= 1900; 
    }
    utc_bytes[0] = DEC_TO_BCD(utc_time.year) ;
    utc_bytes[1] = DEC_TO_BCD(utc_time.month);
    utc_bytes[2] = DEC_TO_BCD(utc_time.day);
    utc_bytes[3] = DEC_TO_BCD(utc_time.hour);
    utc_bytes[4] = DEC_TO_BCD(utc_time.minute);
    utc_bytes[5] = DEC_TO_BCD(utc_time.second);
}

/*
 * state: 1, 授权成功 0, 授权失败
*/
__STATIC_INLINE void send_auth_state(bool state)
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_SET_PROD_AUTH;
    send_evt.header.evt_len = 1;
    send_evt.evt_params.dev_info.prod_auth.set_auth_state = state;

    protocol_send_evt(send_evt);
}

__STATIC_INLINE void send_prod_sn(void)
{
    if (!m_is_ble_connected) return;

    uint8_t         sn[20] = {0};
    protocol_evt_t  send_evt;

    get_sn(sn, 20);

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_PROD_SN;
    send_evt.header.evt_len = 11;

    send_evt.evt_params.dev_info.prod_sn.msb = 0;
    memcpy(send_evt.evt_params.dev_info.prod_sn.sn, &sn[0], 10);
    protocol_send_evt(send_evt);

    send_evt.evt_params.dev_info.prod_sn.msb = 1;
    memcpy(send_evt.evt_params.dev_info.prod_sn.sn, &sn[10], 10);
    protocol_send_evt(send_evt);
}


__STATIC_INLINE void send_prod_manager_sn(void)
{
    #ifdef CONFIG_BOARD_P1015
    uint8_t sn[20]={0,0,18,0,0,1,0,0,0,'C','0','0','1','S',0,0,0,0};
    #else
    uint8_t sn[20]={0};
    #endif
    
    if (!m_is_ble_connected) return;


    protocol_evt_t  send_evt;



    send_evt.header.evt_id = PROT_EVT_ID_ECHO_PROD_MANAGER_SN;
    send_evt.header.evt_len = 11;

    send_evt.evt_params.dev_info.prod_sn.msb = 0;
    memcpy(send_evt.evt_params.dev_info.prod_manager_sn.sn, &sn[0], 9);
    protocol_send_evt(send_evt);

    send_evt.evt_params.dev_info.prod_sn.msb = 1;
    memcpy(send_evt.evt_params.dev_info.prod_manager_sn.sn, &sn[9], 9);
    protocol_send_evt(send_evt);
}


__STATIC_INLINE void send_alarm_clock_triggered(uint8_t index)
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_ALARM_CLK;
    send_evt.header.evt_len = 1;
    send_evt.evt_params.dev_info.alarm_clk.echo_alram_clock_index = index;

    protocol_send_evt(send_evt);
}



static void send_df_reset_info_sched_handler(void * p_context)
{
    protocol_evt_t  send_evt;

    reset_total_info_t reset_total_info;
    w25x80_read(0x0004000, (uint8_t *)&reset_total_info, sizeof(reset_total_info));
    send_evt.header.evt_id = PROT_EVT_ID_RET_PROD_DF_RESET_INFO;
    send_evt.header.evt_len = sizeof(prot_reset_tot_info_evt_t);
    
   
    
  
    send_evt.evt_params.dev_info.reset_total_info.index        = 0;
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_cnt  = __REV16(reset_total_info.reset_cnt);
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_type = reset_total_info.reset_type;
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_utc  = __REV(reset_total_info.reset_utc);
    protocol_send_evt(send_evt);
    
    memset(&send_evt,0x00,sizeof(send_evt));
    send_evt.header.evt_id = PROT_EVT_ID_RET_PROD_DF_RESET_INFO;
    send_evt.header.evt_len = sizeof(prot_reset_tot_info_evt_t);
    send_evt.evt_params.dev_info.reset_total_info.index        = 1;
    memcpy(send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info2.debug_info,reset_total_info.debug_info,sizeof(reset_total_info.debug_info));
    protocol_send_evt(send_evt);
   

    w25x80_erase_sector(0x0004000);
}

static void send_hard_reset_info_sched_handler(void * p_context)
{
    protocol_evt_t  send_evt;

    reset_total_info_t reset_total_info;
    w25x80_read(0x0005000, (uint8_t *)&reset_total_info, sizeof(reset_total_info));
    send_evt.header.evt_id = PROT_EVT_ID_RET_PROD_HARD_RESET_INFO;
    send_evt.header.evt_len = sizeof(prot_reset_tot_info_evt_t);
    
   
    
  
    send_evt.evt_params.dev_info.reset_total_info.index        = 0;
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_cnt  =  __REV16(reset_total_info.reset_cnt);
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_type = reset_total_info.reset_type;
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_utc  =  __REV(reset_total_info.reset_utc);
    protocol_send_evt(send_evt);
    
    memset(&send_evt,0x00,sizeof(send_evt));
    send_evt.header.evt_id = PROT_EVT_ID_RET_PROD_HARD_RESET_INFO;
    send_evt.header.evt_len = sizeof(prot_reset_tot_info_evt_t);
    send_evt.evt_params.dev_info.reset_total_info.index        = 1;
    memcpy(send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info2.debug_info,reset_total_info.debug_info,sizeof(reset_total_info.debug_info));
    protocol_send_evt(send_evt);
   

    w25x80_erase_sector(0x0005000);
}



static void send_err_reset_info_sched_handler(void * p_context)
{
    protocol_evt_t  send_evt;

    reset_total_info_t reset_total_info;
    w25x80_read(0x0006000, (uint8_t *)&reset_total_info, sizeof(reset_total_info));
    send_evt.header.evt_id = PROT_EVT_ID_RET_PROD_ERR_RESET_INFO;
    send_evt.header.evt_len = sizeof(prot_reset_tot_info_evt_t);
    
   
    
  
    send_evt.evt_params.dev_info.reset_total_info.index        = 0;
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_cnt  =  __REV16(reset_total_info.reset_cnt);
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_type = reset_total_info.reset_type;
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info1.reset_utc  =  __REV(reset_total_info.reset_utc);
    protocol_send_evt(send_evt);
    
    memset(&send_evt,0x00,sizeof(send_evt));
    send_evt.header.evt_id = PROT_EVT_ID_RET_PROD_ERR_RESET_INFO;
    send_evt.header.evt_len = sizeof(prot_reset_tot_info_evt_t);
    send_evt.evt_params.dev_info.reset_total_info.index        = 1;
    memcpy(send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info2.debug_info,reset_total_info.debug_info,sizeof(reset_total_info.debug_info));
    send_evt.evt_params.dev_info.reset_total_info.reset_info_pack.reset_info2.err_code  = __REV(reset_total_info.err_code);
    protocol_send_evt(send_evt);
   

    w25x80_erase_sector(0x0006000);
}



static void devinfo_timeout_timer_handler(void * p_context)
{
//    //DBG0_EVT0("devinfo_timeout_timer_handler");
    if (m_auth_mode)
    {
        m_auth_mode = 0;
        send_auth_state(0); // 发送授权失败
        APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_AUTH_EXIT_MODE));
    }
}




static void alarm_clock_evt_handler(app_alarm_clock_evt_t * p_evt)
{
    if ( (APP_ALARM_CLOCK_EVT_TRIGGERED == p_evt->evt_id) && (! m_photo_mode) && (! m_save_mode) )
    {       
        send_alarm_clock_triggered(p_evt->param.alarm_clock_index);
        APP_ERROR_CHECK(event_send(EVT_COMS_PROT_EXEC, PROT_EXEC_EVT_ALARM_TRIGGERED));
    }
}

static uint32_t alarm_clock_init(void)
{
    uint32_t err_code;

    app_alarm_clock_config_t config;

    config.p_param_list          = system_database.alarm_clk_config_list;
    config.trigger_time.p_hour   = &system_database.utc.hour;
    config.trigger_time.P_minute = &system_database.utc.minute;
    config.trigger_time.p_week   = &system_database.utc.week;

    err_code = app_alarm_clock_init(&config, alarm_clock_evt_handler);

    if (err_code == NRF_SUCCESS)
    {
        err_code = app_alarm_clock_enable();
    }

    return err_code;
}

static void echo_app_low_power(void)
{
    if (!m_is_ble_connected) return;
//    //DBG0_EVT0("echo_app_low_power");
    protocol_evt_t send_evt;            
    send_evt.header.evt_id = PROT_EVT_ID_ECHO_PROD_SOC_NEW;
    send_evt.header.evt_len = 2;
    send_evt.evt_params.dev_info.prod_soc_new.type =0;
    send_evt.evt_params.dev_info.prod_soc_new.percent = system_database.battery_state;
    protocol_send_evt(send_evt);
}


static void get_utc_date(void)
{
    if (!m_is_ble_connected) return;
//    //DBG0_EVT0("get_utc_date");
    protocol_evt_t send_evt;            
    send_evt.header.evt_id = PROT_EVT_ID_GET_DATE;
    send_evt.header.evt_len = 0;
    protocol_send_evt(send_evt);
}


		
bool m_protocol_devinfo_event_handler(const event_t * p_event)
{
    ASSERT(m_devinfo_init);

    switch (p_event->type)
    {
         case EVT_SYSTEM_STATE :
            if(p_event->system.data == SYSTEM_STATE_SAVE_POWER_MODE)
            {
                m_save_mode = true;
            }
            else if(p_event->system.data == SYSTEM_STATE_RESUME_F_SAVE)
            {
                m_save_mode = false;
            }
            else if(p_event->system.data == SYSTEM_STATE_GREEN_MODE)
            {
                m_save_mode = true;
            }
            else if(p_event->system.data == SYSTEM_STATE_RESUME_F_GREEN)
            {
                m_save_mode = false;
            }
        case EVT_SYSTEM_BATTERY_LEVEL:
            if (p_event->system.data == 0)
            {
                echo_app_low_power();
            }
        break;
        
        case EVT_SYSTEM_UTC_MINUTE:
            app_alarm_clock_minute_process();
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
            break;
            
       case EVT_COMS_APP_RECV:     
           switch (p_event->coms.app_recv_event)
           {
              case APP_RECV_EVT_PHOTO_ENTER_MODE:
                  m_photo_mode = true ;
                  break;
              case APP_RECV_EVT_PHOTO_EXIT_MODE:
                  m_photo_mode = false ;
                  break;                  
           }
    
        case EVT_BUTTON_PRESSED:
            switch (p_event->button.button_event)
            {
                case BUTTON_EVT_AUTH_OK:
                    if (m_auth_mode)
                    {
                        m_auth_mode = 0;
                        send_auth_state(1); // 发送授权成功
                        APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_AUTH_EXIT_MODE));
                        app_timer_stop(devinfo_timeout_timer_id);
                    }
                    break;
                default : 
                    break;
            }

        default:
            break;
    }

    return false;
}


static uint8_t get_product_information(uint16_t * p_evt_id, prot_dev_info_evt_t * p_dev_info_evt)
{
    uint16_t ret_evt_id = PROT_EVT_ID_INVALID;
    uint8_t  evt_len = 0;

    switch (*p_evt_id)
    {
        case PROT_EVT_ID_GET_PROD_NAME:
            ret_evt_id = PROT_EVT_ID_RET_PROD_NAME;
            evt_len = strlen(CONFIG_PRODUCT_NAME);
            evt_len = (evt_len > 11) ? 11 : evt_len;
            memcpy(p_dev_info_evt->prod_name.name, CONFIG_PRODUCT_NAME, evt_len);
            break;

        case PROT_EVT_ID_GET_PROD_VER:
            ret_evt_id = PROT_EVT_ID_RET_PROD_VER;
            evt_len = 3;
            p_dev_info_evt->prod_ver.fw_version[0] = CONFIG_PRODUCT_FW_VER_MAJOR;
            p_dev_info_evt->prod_ver.fw_version[1] = CONFIG_PRODUCT_FW_VER_MINOR;
            p_dev_info_evt->prod_ver.fw_version[2] = CONFIG_PRODUCT_FW_VER_TEST;
            break;

        case PROT_EVT_ID_GET_PROD_DATE:
            ret_evt_id = PROT_EVT_ID_RET_PROD_DATE;
            break;

        case PROT_EVT_ID_GET_PROD_SOC:
            ret_evt_id = PROT_EVT_ID_RET_PROD_SOC;
            evt_len = 1;
            p_dev_info_evt->prod_soc.percent = system_database.battery_level;
            break;
        

    
        

        default:
            break;
    }

    *p_evt_id = ret_evt_id;

    return evt_len;
}

static void send_evt_sched_event_handler(void * p_context)
{
    uint8_t send_evt = *((uint8_t *)p_context);


    if (send_evt == SEND_EVT_PROD_SN)
    {
        send_prod_sn();
    }
    else if(send_evt == SEND_EVT_PROD_MANAGER_SN)
    {
        send_prod_manager_sn();
    }
}



bool m_protocol_devinfo_prot_evt_execution(protocol_evt_t * p_evt)
{
    ASSERT(m_devinfo_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_DEV_INFO)
    {
        return false; // 不属于 devinfo 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // 默认返回找不到接口的帧错误



    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_GET_PROD_NAME:
        case PROT_EVT_ID_GET_PROD_VER:
        case PROT_EVT_ID_GET_PROD_DATE:
        case PROT_EVT_ID_GET_PROD_SOC:       
            p_evt->header.evt_len = get_product_information(&p_evt->header.evt_id, &p_evt->evt_params.dev_info);
            ack_state = ACK_STATE_INVALID;
            break;
        
        case PROT_EVT_ID_GET_PROD_SOC_NEW:
            P_DevInfo_Print("PROT_EVT_ID_GET_PROD_SOC_NEW : system_database.battery_state: %d",system_database.battery_state);
            p_evt->header.evt_id = PROT_EVT_ID_RET_PROD_SOC_NEW;
            p_evt->header.evt_len = 2;
            p_evt->evt_params.dev_info.prod_soc_new.type =0;
            p_evt->evt_params.dev_info.prod_soc_new.percent = system_database.battery_state;
            ack_state = ACK_STATE_INVALID;
            break;
        
        case PROT_EVT_ID_GET_PROD_BATT_VOL:

            p_evt->header.evt_id = PROT_EVT_ID_RET_PROD_BATT_VOL;
            p_evt->header.evt_len = 2;          
            p_evt->evt_params.dev_info.prod_batt_vol.batt_vol = __REV16(batt_vol);
            APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_GET_BATT_VOL));
            ack_state = ACK_STATE_INVALID;
            break;
        
        
         case PROT_EVT_ID_GET_PROD_KEY_RESET_INFO:
            
            P_DevInfo_Print("PROT_EVT_ID_GET_PROD_RESET_INFO");

            uint8_t button_info[4];
            w25x80_read(0x0003000, (uint8_t *)&button_info, 2);
            p_evt->header.evt_id = PROT_EVT_ID_RET_PROD_KEY_RESET_INFO;
            p_evt->header.evt_len = 4;      
            p_evt->evt_params.dev_info.key_reset_info.key_info[0] = button_info[0];
            p_evt->evt_params.dev_info.key_reset_info.key_info[1] = button_info[1];
            p_evt->evt_params.dev_info.key_reset_info.key_info[2] = button_info[2];
            p_evt->evt_params.dev_info.key_reset_info.key_info[3] = button_info[3];
            w25x80_erase_sector(0x0003000);
            ack_state = ACK_STATE_INVALID;            
            break;
        
       case PROT_EVT_ID_GET_PROD_DF_RESET_INFO:
            P_DevInfo_Print("PROT_EVT_ID_GET_PROD_DF_RESET_INFO");
    
            APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler, send_df_reset_info_sched_handler, NULL));
            ack_state = ACK_STATE_SUCCESS; 
            break;
       
       case PROT_EVT_ID_GET_PROD_HARD_RESET_INFO:
            P_DevInfo_Print("PROT_EVT_ID_GET_PROD_HARD_RESET_INFO");
    
            APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler, send_hard_reset_info_sched_handler, NULL));
            ack_state = ACK_STATE_SUCCESS; 
            break;
              
        case PROT_EVT_ID_GET_PROD_ERR_RESET_INFO:
            P_DevInfo_Print("PROT_EVT_ID_GET_PROD_ERR_RESET_INFO");
    
            APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler, send_err_reset_info_sched_handler, NULL));
            ack_state = ACK_STATE_SUCCESS; 
        
            #if DEBUG_RESET_INFO
            if(system_database.reset_reason == 00000004)
            {
                uint8_t temp[6] = {0};
                w25x80_read(0x0006000,temp,6);
                SEGGER_RTT_printf(0,"%02x %02x %02x %02x %02x %02x ",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]);   
                p_evt->evt_params.dev_info.reset_info.reset_nums   = system_database.reset_cnt;
                p_evt->evt_params.dev_info.reset_info.reset_reason = __REV(system_database.reset_reason);
                p_evt->evt_params.dev_info.reset_info.reset_line[0] = temp[4];
                p_evt->evt_params.dev_info.reset_info.reset_line[1] = temp[5];
                p_evt->evt_params.dev_info.reset_info.reset_info.reset_str[0] = temp[0];
                p_evt->evt_params.dev_info.reset_info.reset_info.reset_str[1] = temp[1];
                p_evt->evt_params.dev_info.reset_info.reset_info.reset_str[2] = temp[2];
                p_evt->evt_params.dev_info.reset_info.reset_info.reset_str[3] = temp[3];
            }
            else
            {
                p_evt->evt_params.dev_info.reset_info.reset_nums   = system_database.reset_cnt;
                p_evt->evt_params.dev_info.reset_info.reset_reason = __REV(system_database.reset_reason);
                p_evt->evt_params.dev_info.reset_info.reset_info.reset_wdt_addr   = __REV(system_database.wdt_reset_pc_addr);
            }         
            #endif
                       
            break;
            
            case PROT_EVT_ID_GET_PROD_RESET_INFO:
            P_DevInfo_Print("PROT_EVT_ID_GET_PROD_RESET_INFO");
            
            #define RESET_REASON_ADDR      0x00015000 // 复位reason保存
           
            uint32_t temp;
            w25x80_read(RESET_REASON_ADDR, (uint8_t *)&temp, 4);
            w25x80_read(RESET_REASON_ADDR, (uint8_t *)&system_database.reset_reason, sizeof(system_database.reset_reason));
            p_evt->header.evt_id = PROT_EVT_ID_RET_PROD_RESET_INFO;
            p_evt->header.evt_len = sizeof(prot_reset_info_evt_t);
            
            P_DevInfo_Print("PROT_EVT_ID_GET_PROD_RESET_INFO %08x %08x",system_database.reset_reason,temp);            
            if(system_database.reset_reason == 00000004)
            {
                p_evt->evt_params.dev_info.reset_info.reset_nums   = system_database.reset_cnt;
                p_evt->evt_params.dev_info.reset_info.reset_reason = __REV(system_database.reset_reason);
            }
            else
            {
                p_evt->evt_params.dev_info.reset_info.reset_nums   = system_database.reset_cnt;
                p_evt->evt_params.dev_info.reset_info.reset_reason = __REV(system_database.reset_reason);
                p_evt->evt_params.dev_info.reset_info.reset_info.reset_wdt_addr   = __REV(system_database.wdt_reset_pc_addr);
            } 
            ack_state = ACK_STATE_INVALID; 
            break;
        case PROT_EVT_ID_GET_PROD_SN:
            ack_state = ACK_STATE_SUCCESS;
            APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler, send_evt_sched_event_handler, (void *)&SEND_EVT_PROD_SN));

            break;
        
        case PROT_EVT_ID_GET_PROD_MANAGER_SN:
            ack_state = ACK_STATE_SUCCESS;
            APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler, send_evt_sched_event_handler, (void *)&SEND_EVT_PROD_MANAGER_SN));
            break;
        
        case PROT_EVT_ID_GET_PROD_PRO_NUM:            
            p_evt->header.evt_id = PROT_EVT_ID_RET_PROD_PRO_NUM;
            p_evt->header.evt_len = 10;
            memcpy(&p_evt->evt_params.dev_info.prod_pro_num.pro_num,CONFIG_PROJECT_NUM,5);
            memcpy(&p_evt->evt_params.dev_info.prod_pro_num.pro_type,CONFIG_HARDWARE_REVISION,5);
            ack_state = ACK_STATE_INVALID; 
            break;
                
        case PROT_EVT_ID_GET_PROD_MAC:
            p_evt->header.evt_id = PROT_EVT_ID_RET_PROD_MAC;
            p_evt->header.evt_len = 6;
            memcpy(&p_evt->evt_params.dev_info.prod_mac.mac,system_database.ble_addr,6);
            ack_state = ACK_STATE_INVALID;          
            break;
        
        case PROT_EVT_ID_SET_DATE:
        {           
            uint8_t * utc_bytes = p_evt->evt_params.dev_info.date.utc_bytes;
            system_database.utc_seconds = utc_to_second(utcbytes_to_utc(utc_bytes));
            system_database.timezone_int =p_evt->evt_params.dev_info.date.timezone_int;           // 时区整数部分，-12~12
            system_database.timezone_decimal =p_evt->evt_params.dev_info.date.timezone_decimal;       // 时区小数部分
            system_database.week  =p_evt->evt_params.dev_info.date.week;                        // 星期
            DBG1_EVT1("Set date second: %d", system_database.utc_seconds);
            DBG1_EVT6("Set date utc: %02x/%02x/%02x %02x:%02x:%02x", utc_bytes[0], utc_bytes[1], utc_bytes[2], utc_bytes[3], utc_bytes[4], utc_bytes[5]);
            ack_state = ACK_STATE_SUCCESS;
            break;
        }
        
        case PROT_EVT_ID_GET_DATE:
        {
            prot_date_evt_t ret_data_param;
            p_evt->header.evt_id  = PROT_EVT_ID_RET_DATE;
            p_evt->header.evt_len = sizeof(prot_date_evt_t);
            ret_data_param.timezone_decimal = system_database.timezone_decimal;
            ret_data_param.timezone_int = system_database.timezone_int;
            ret_data_param.week   =system_database.week;
            utc_to_utcbytes(second_to_utc(system_database.utc_seconds),ret_data_param.utc_bytes);
            P_DevInfo_Print("Ret date utc: %02x/%02x/%02x %02x:%02x:%02x", ret_data_param.utc_bytes[0],ret_data_param.utc_bytes[1], ret_data_param.utc_bytes[2], 
                                                                     ret_data_param.utc_bytes[3],ret_data_param.utc_bytes[4], ret_data_param.utc_bytes[5]);
            p_evt->evt_params.dev_info.date = ret_data_param;
            ack_state = ACK_STATE_INVALID;
        }
        break;
		
		case PROT_EVT_ID_RET_DATE:
        {
            uint8_t * utc_bytes = p_evt->evt_params.dev_info.date.utc_bytes;
			
            system_database.utc_seconds = utc_to_second(utcbytes_to_utc(utc_bytes));
            system_database.timezone_int =p_evt->evt_params.dev_info.date.timezone_int;           // 时区整数部分，-12~12
            system_database.timezone_decimal =p_evt->evt_params.dev_info.date.timezone_decimal;       // 时区小数部分
            system_database.week  =p_evt->evt_params.dev_info.date.week;                        // 星期
			system_database.watch_hand_0_pos = (system_database.utc_seconds % WATCH_HAND_POS_MAX);
            P_DevInfo_Print("RET date second: %d", system_database.utc_seconds);
            P_DevInfo_Print("RET date utc: %02x/%02x/%02x %02x:%02x:%02x", utc_bytes[0], utc_bytes[1], utc_bytes[2], utc_bytes[3], utc_bytes[4], utc_bytes[5]);
            ack_state = ACK_STATE_SUCCESS;
        }
        break;
		

        case PROT_EVT_ID_CALL_PROD_AUTH:
        {
            if (p_evt->evt_params.dev_info.prod_auth.call_auth_req == 0) // 授权请求
            {
                m_auth_mode = 1;
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_AUTH_ENTER_MODE));
                app_timer_start(devinfo_timeout_timer_id, APP_TIMER_TICKS(60000), NULL);
            }
            else // 强制授权
            {
                APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, APP_RECV_EVT_AUTH_FORCE_OK));
            }
            ack_state = ACK_STATE_SUCCESS;
            break;
        }
        
        case PROT_EVT_ID_SET_ALARM_CLK:
        {
            prot_alarm_clk_param_t param = p_evt->evt_params.dev_info.alarm_clk.set_param;

            if ((param.num > 0) && (param.num <= APP_ALARM_CLOCK_COUNT_MAX))
            {
                uint8_t index = param.num - 1;

                system_database.alarm_clk_config_list[index].num            = param.num;
                system_database.alarm_clk_config_list[index].valid.valid    = 1;
                system_database.alarm_clk_config_list[index].valid.enabled  = param.enable;
                system_database.alarm_clk_config_list[index].hour           = BCD_TO_DEC(param.hours);
                system_database.alarm_clk_config_list[index].minute         = BCD_TO_DEC(param.minute);
                system_database.alarm_clk_config_list[index].week.repeat    = param.weeks.weeks.repeat;
                system_database.alarm_clk_config_list[index].week.weeks     = param.weeks.weeks.week_list;

                P_DevInfo_Print("Set alram clock: num: %d, hour: %d, minute: %d", system_database.alarm_clk_config_list[index].num, 
                                                                            system_database.alarm_clk_config_list[index].hour,
                                                                            system_database.alarm_clk_config_list[index].minute);
            }
            ack_state = ACK_STATE_SUCCESS;
            break;
        }

        case PROT_EVT_ID_GET_ALARM_CLK:
        {
            uint8_t num = p_evt->evt_params.dev_info.alarm_clk.get_num;
            prot_alarm_clk_param_t  ret_param_temp;

            if ((num > 0) && (num <= APP_ALARM_CLOCK_COUNT_MAX))
            {
                uint8_t                 index = num - 1;

                p_evt->header.evt_id  = PROT_EVT_ID_RET_ALARM_CLK;
                p_evt->header.evt_len = sizeof(prot_alarm_clk_param_t);

                ret_param_temp.num                      = num;
                ret_param_temp.enable                   = system_database.alarm_clk_config_list[index].valid.enabled;
                ret_param_temp.hours                    = DEC_TO_BCD(system_database.alarm_clk_config_list[index].hour);
                ret_param_temp.minute                   = DEC_TO_BCD(system_database.alarm_clk_config_list[index].minute);
                ret_param_temp.weeks.weeks.repeat       = system_database.alarm_clk_config_list[index].week.repeat;
                ret_param_temp.weeks.weeks.week_list    = system_database.alarm_clk_config_list[index].week.weeks;

                p_evt->evt_params.dev_info.alarm_clk.ret_param = ret_param_temp;
            }
            else if (num == 0)
            {
                protocol_evt_t ret_evt;
                p_evt->header.evt_id  = PROT_EVT_ID_INVALID;

                for (uint8_t index = 0; index < APP_ALARM_CLOCK_COUNT_MAX; index++)
                {
                    ret_evt.header.evt_id  = PROT_EVT_ID_RET_ALARM_CLK;
                    ret_evt.header.evt_len = sizeof(prot_alarm_clk_param_t);

                    ret_param_temp.num                      = index + 1;
                    ret_param_temp.enable                   = system_database.alarm_clk_config_list[index].valid.enabled;
                    ret_param_temp.hours                    = DEC_TO_BCD(system_database.alarm_clk_config_list[index].hour);
                    ret_param_temp.minute                   = DEC_TO_BCD(system_database.alarm_clk_config_list[index].minute);
                    ret_param_temp.weeks.weeks.repeat       = system_database.alarm_clk_config_list[index].week.repeat;
                    ret_param_temp.weeks.weeks.week_list    = system_database.alarm_clk_config_list[index].week.weeks;

                    ret_evt.evt_params.dev_info.alarm_clk.ret_param = ret_param_temp;

                    protocol_send_evt(ret_evt);
                }
            }
            else
            {
                p_evt->header.evt_id  = PROT_EVT_ID_INVALID;
            }
            ack_state = ACK_STATE_INVALID;
            break;
        }
        
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

uint32_t m_protocol_devinfo_init(void)
{
    uint32_t status = NRF_SUCCESS;

    status = app_timer_create(&devinfo_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, devinfo_timeout_timer_handler);
//    if (status == NRF_SUCCESS)
//    {
//        status = app_timer_create(&send_alarm_triggerd_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, send_alarm_triggerd_timer_handler);
//    }

    if (status == NRF_SUCCESS)
    {
        status = alarm_clock_init();
    }


    if (status == NRF_SUCCESS)
    {
        m_devinfo_init = true;
    }

    return status;
}


