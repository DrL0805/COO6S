
#include <stdint.h>             /* C 库头文件 */
#include <stdbool.h>
#include <string.h>
//#include "c001s_config.h"       /* 资源配置、底层资源头文件 */
#include "nrf_assert.h"
#include "app_timer.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"
#include "app_isched.h"
#include "resources.h"

#include "event_bus.h"
        /* 模块头文件 */
#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_sleep_record.h"
#include "algorithm_sleep.h"


typedef struct 
{
    bool     busy;
    uint8_t  packs_count;
    uint8_t  index;
    uint32_t del_utc_record;
}m_sleep_record_pack_t;
    
m_sleep_record_pack_t m_sleep_record_pack;
static bool m_sleep_record_init = false;
static bool m_is_ble_connected = false;


static void send_sleep_packs_sched_handler(void * p_context);
static void del_sleep_packs_by_utc_sched_handler(void * p_context);

static void send_sleep_record_detail(uint8_t pack_index,uint32_t sleep_record_wake_utc,uint32_t sleep_record_sleep_utc)
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;
                
    send_evt.header.evt_id = PROT_EVT_ID_ECHO_SLEEP_RECORD_INFO;  //
    send_evt.header.evt_len = 9; 

    send_evt.evt_params.sleep_record.sleep_history_pkg.echo_param.detail_index = pack_index;
    send_evt.evt_params.sleep_record.sleep_history_pkg.echo_param.wake_up_utc = __REV(sleep_record_wake_utc);
    send_evt.evt_params.sleep_record.sleep_history_pkg.echo_param.sleep_utc = __REV(sleep_record_sleep_utc);

    protocol_send_evt(send_evt);
}

bool m_protocol_sleep_record_event_handler(const event_t * p_event)
{
    ASSERT(m_sleep_record_init);

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
            
        case EVT_COMS_APP_RECV:
            if (p_event->coms.app_recv_event == APP_RECV_EVT_DAILY_SPORT_STEPS_GOAL)
            {
//                test_sleep();
            }
            break;
            
        default:
            break;
    }
            
    return false;
}

bool m_protocol_sleep_record_prot_evt_execution(protocol_evt_t * p_evt)
{
    ASSERT(m_sleep_record_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_SLEEP_RECORD)
    {
        return false; // 不属于 sleep_record 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // 默认返回找不到接口的帧错误

    DBG0_EVT1("<***** sleep_record evt_id 0x%04x *****>", p_evt->header.evt_id);
    
    uint8_t index_num;
    uint8_t index;
    uint8_t wake_times;
    uint32_t sleep_record_start_utc;
    uint32_t sleep_record_stop_utc;
    
    

    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_GET_SLEEP_RECORD_COUNT:
            DBG0_EVT0("<***** PROT_EVT_ID_GET_SLEEP_RECORD_COUNT *****>");
            p_evt->header.evt_id = PROT_EVT_ID_RET_SLEEP_RECORD_COUNT;
            p_evt->header.evt_len = 1;
            index_num = get_valid_sleep_recode_content();
            p_evt->evt_params.sleep_record.sleep_history_index.sleep_histrory_index_num = index_num;
            ack_state = ACK_STATE_INVALID;
            break;     
        
        case PROT_EVT_ID_GET_SLEEP_RECORD_INFO: 
            DBG0_EVT0("<***** PROT_EVT_ID_GET_SLEEP_RECORD_INFO *****>");            
            index = p_evt->evt_params.sleep_record.sleep_history_pkg.get_param.pack_index;
                  
            p_evt->header.evt_id = PROT_EVT_ID_RET_SLEEP_RECORD_INFO;  //
            p_evt->header.evt_len = 9; 
            ack_state = ACK_STATE_INVALID;
            get_valid_sleep_recode_by_index(index,&wake_times,&sleep_record_start_utc,&sleep_record_stop_utc);
            p_evt->evt_params.sleep_record.sleep_history_pkg.ret_param.sleep_utc_start = __REV(sleep_record_start_utc);
            p_evt->evt_params.sleep_record.sleep_history_pkg.ret_param.sleep_utc_stop = __REV(sleep_record_stop_utc);
            p_evt->evt_params.sleep_record.sleep_history_pkg.ret_param.wake_times = wake_times;
        
            DBG0_EVT3("<***** %x %x %x *****>",wake_times,sleep_record_start_utc,sleep_record_stop_utc); 
            /*------上传中间醒来记录包--------*/
            m_sleep_record_pack.index = index;
            m_sleep_record_pack.packs_count = wake_times;
            if(wake_times)
            {
                APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, send_sleep_packs_sched_handler, NULL));
            }
            break;
            
        case PROT_EVT_ID_CALL_SLEEP_RECORD_DEL:
            
            m_sleep_record_pack.del_utc_record       = __REV(p_evt->evt_params.sleep_record.sleep_history_del.call_param.record_utc);     
            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, del_sleep_packs_by_utc_sched_handler, NULL));
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

static void del_sleep_packs_by_utc_sched_handler(void * p_context)
{
    uint8_t result = 0;
    uint32_t err_code = delete_valid_sleep_recode_by_utc(m_sleep_record_pack.del_utc_record);
    DBG0_EVT2("<***** delete utc:%x ;err_code :%d *****>",m_sleep_record_pack.del_utc_record,err_code);
    if(NRF_ERROR_INVALID_PARAM == err_code)
    {
        result = 2;
    }
    else if(NRF_SUCCESS == err_code)
    {
        result = 0;
    }
    
    protocol_evt_t send_evt;                
    send_evt.header.evt_id = PROT_EVT_ID_ECHO_SLEEP_RECORD_DEL;  //
    send_evt.header.evt_len = 5; 
    send_evt.evt_params.sleep_record.sleep_history_del.echo_param.record_utc = __REV(m_sleep_record_pack.del_utc_record);
    send_evt.evt_params.sleep_record.sleep_history_del.echo_param.result     = result;
    protocol_send_evt(send_evt);
}


static void send_sleep_packs_sched_handler(void * p_context)
{
    uint32_t sleep_record_wake_utc[5];
    uint32_t sleep_record_sleep_utc[5];
    ASSERT(m_sleep_record_pack.packs_count < 5);
    get_valid_sleep_detail_recode_by_index(m_sleep_record_pack.index,sleep_record_wake_utc,sleep_record_sleep_utc);
    for(int i=0;i<m_sleep_record_pack.packs_count;i++)
    {
        send_sleep_record_detail(i,sleep_record_wake_utc[i],sleep_record_sleep_utc[i]);
        DBG0_EVT3("<*****sleep packs: %d %d %d *****>",i,sleep_record_wake_utc[i],sleep_record_sleep_utc[i]);
    }
    
}


uint32_t m_protocol_sleep_record_init(void)
{
    uint32_t status = NRF_SUCCESS;

    m_sleep_record_init = true;    
    memset((uint8_t *)&m_sleep_record_pack, 0, sizeof(m_sleep_record_pack));
    return status;
}



