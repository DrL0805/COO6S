
#include <stdint.h>             /* C 库头文件 */
#include <stdbool.h>

#include "nrf_log.h"

#include "sr3_config.h"       /* 资源配置、底层资源头文件 */
#include "nrf_assert.h"
#include "app_timer.h"
#include "app_debug.h"
#include "app_error.h"
#include "protocol.h"
#include "AD_DataStore.h"

#include "app_isched.h"
#include "resources.h"

#include "event_bus.h"

#include "m_system_state.h"
#include "m_database.h"
#include "m_protocol_pack_trans.h"

#include "includes.h"

#if CONFIG_RTT_PROTOCOL_PACK_ENABLED
#define P_PackTrans_Print(M, ...)	SEGGER_Print("PackTran", M, ##__VA_ARGS__)
#else
#define P_PackTrans_Print(M, ...)
#endif




#define INVALID_CONTENT_INDEX       0xFFFF
#define INVALID_DATA                0xFF
#define INVALID_DATA_TYPE           0xFFFF
#define INVALID_PACK_INDEX          0xFFFF

typedef struct
{
    uint16_t data_type;
    uint32_t utc_second;
}delete_by_utc_param_t;


typedef struct
{
    bool     busy;
    uint16_t data_type;
    uint32_t utc_second;
    uint16_t pack_index;
}send_pack_ctrl_block_t;

typedef struct
{
    bool     busy;
    uint16_t data_type;
    uint16_t packs_count;
    uint16_t content_index;
    uint16_t pack_index;
}send_pack_block_t;


typedef struct
{
    bool     busy;
    uint16_t data_type;
    uint16_t content_count;
}send_content_info_ctrl_block_t;

APP_TIMER_DEF(x_timeout_timer_id);

static bool m_pack_trans_init = false;
static bool m_is_ble_connected = false;

static delete_by_utc_param_t s_delete_by_utc = {0};
//static send_pack_ctrl_block_t s_send_pack_cb;
static send_content_info_ctrl_block_t s_send_content_info_cb;

static send_pack_block_t s_m_send_pack_cb;



#if 0

__STATIC_INLINE void send_x(void)
{
/*
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_X;
    send_evt.header.evt_len = x;
    send_evt.evt_params.pack_trans = x;

    protocol_send_evt(send_evt);
*/
}
#endif

static void x_timeout_timer_handler(void * p_context)
{

}

__STATIC_INLINE uint16_t get_contentcount_of_datatype(uint16_t datatype)//得到某数据类型的目录条数
{
    uint16_t count = GetDirectoryCount(datatype);

    P_PackTrans_Print("get_contentcount_of_datatype: datatype: %d, count: %d", datatype, count);

    return count;
}

__STATIC_INLINE uint32_t get_data_len_of_content(uint16_t datatype, uint16_t content_index)
{
    uint32_t len = GetDirectoryDataLen(content_index, datatype);

    P_PackTrans_Print("get_data_len_of_content: datatype: %d, content_index: %d, len: %d", datatype, content_index, len);

    return len;
}

__STATIC_INLINE uint32_t get_data_len_of_datatype(uint16_t datatype, uint16_t content_count)
{
    uint32_t data_len = 0;

    for (uint16_t content_index = 0; content_index < content_count; content_index++)
    {
        data_len += get_data_len_of_content(datatype, content_index);
    }

    P_PackTrans_Print("get_data_len_of_datatype: datatype: %d, content_count: %d, data_len: %d", datatype, content_count, data_len);

    return data_len;
}




__STATIC_INLINE uint32_t get_utc_second_of_content(uint16_t datatype, uint16_t content_index)
{
    uint32_t second = 0;
       
   second = GetDirectoryUTC_Time(content_index, datatype);
    
    return second;
}

__STATIC_INLINE uint16_t get_content_index_from_utcsecond(uint16_t datatype, uint32_t utc_second)
{
    //DBG0_EVT0("get_content_index_from_utcsecond");
    uint16_t index = 0;
    uint32_t get_second = 0;
    uint16_t content_count = get_contentcount_of_datatype(datatype);
    for (index = 0; index < content_count; index++)
    {
        
        get_second = get_utc_second_of_content(datatype, index);

        if (get_second == utc_second)
        {
            return index;
        }
    }

    //DBG0_EVT3("get_content_index_from_utcsecond: datatype: %d, utc_second: %d, index: %d", datatype, utc_second, INVALID_CONTENT_INDEX);

    return INVALID_CONTENT_INDEX;
}

__STATIC_INLINE uint16_t get_pack_count_of_content(uint16_t datatype, uint16_t content_index)
{
    uint32_t content_datalen = get_data_len_of_content(datatype, content_index);
    uint16_t pack_count = CEIL_DIV(content_datalen, 16);

    //DBG0_EVT3("get_pack_count_of_content: datatype: %d, content_index: %d, pack_count: %d", datatype, content_index, pack_count);

    return pack_count;
}

__STATIC_INLINE void get_pack_head(uint8_t * p_data_buf, uint16_t datatype, uint16_t pack_index)
{
    memset(p_data_buf, INVALID_DATA, 20);
    p_data_buf[0] = (pack_index >> 8) & 0xff;
    p_data_buf[1] = (pack_index >> 0) & 0xff;
    p_data_buf[2] = (datatype   >> 8) & 0xff;
    p_data_buf[3] = (datatype   >> 0) & 0xff;
}

//static uint8_t send_packs(uint16_t data_type, uint32_t utc_second, uint16_t pack_index)
static uint8_t send_packs(uint16_t data_type, uint16_t content_index, uint16_t packs_count,uint16_t pack_index)
{

    if (!m_is_ble_connected) return DATAREQ_RESULT_INTERNAL_ERROR;
    uint8_t  data_buf[20]  = {0};
    uint32_t err_code = NRF_SUCCESS;



    if (pack_index != 0) // 指定上传某个数据包
    {
        get_pack_head(data_buf, data_type, 0);
//        //DBG0_EVT3("[GetSleepAndStepData] pack_index: %d, content_index: %d, data_type: %d.", pack_index, content_index, data_type);
        GetSleepAndStepData(pack_index, content_index, data_type, &data_buf[4]);

        err_code = protocol_send_data(data_buf, 20);
#if 1
                NRF_LOG_RAW_INFO("GetSleepAndStepData: ");
                for (uint8_t i = 0; i < 20; i++)
                {
                    NRF_LOG_RAW_INFO("%02x ", data_buf[i]);
                    if ((i == 3))
                    {
                        NRF_LOG_RAW_INFO("| ");
                    }
                }
                NRF_LOG_RAW_INFO("\r\n");
#endif
        if (NRF_ERROR_NO_MEM == err_code)
        {
            //DBG0_EVT0("Send one pack error: no memory\r\n");
            return DATAREQ_RESULT_INTERNAL_ERROR;
        }

        return DATAREQ_RESULT_SUCCESS;
    }

    for (uint16_t i = 0; i < packs_count; i++)
    {
        get_pack_head(data_buf, data_type, i);
//        //DBG0_EVT3("[GetSleepAndStepData] pack_index: %d, content_index: %d, data_type: %d.", pack_index, content_index, data_type);

        err_code = GetSleepAndStepData(i, content_index, data_type, &data_buf[4]);
        //DBG0_EVT0("[GetSleepAndStepData err_code : %d",err_code);
        err_code = protocol_send_data(data_buf, 20);
#if 1
                NRF_LOG_RAW_INFO("GetSleepAndStepData: ");
                for (uint8_t i = 0; i < 20; i++)
                {
                    NRF_LOG_RAW_INFO("%02x ", data_buf[i]);
                    if ((i == 3))
                    {
                        NRF_LOG_RAW_INFO("| ");
                    }
                }
                NRF_LOG_RAW_INFO("\r\n");
#endif

        if (NRF_ERROR_NO_MEM == err_code)
        {
            //DBG0_EVT0("Send packs error: no memory!\r\n");
            return DATAREQ_RESULT_INTERNAL_ERROR;
        }
    }

    s_m_send_pack_cb.busy = false;

    return DATAREQ_RESULT_SUCCESS;
}

static void send_packs_sched_handler(void * p_context)
{

    if (DATAREQ_RESULT_SUCCESS != send_packs(s_m_send_pack_cb.data_type, s_m_send_pack_cb.content_index, s_m_send_pack_cb.packs_count,s_m_send_pack_cb.pack_index))
    {
        s_m_send_pack_cb.busy = false;
    }
}

static void auto_send_content_count_sched_handler(void * p_context)
{
    
    if (!m_is_ble_connected) return;
    
    //DBG0_EVT0("auto_send_content_count_sched_handler\r\n");

    prot_auto_update_content_count_evt_t aucc_param;
    protocol_evt_t send_evt;
    uint16_t saved_content_count = 0;
    uint32_t saved_all_data_len = 0;

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_AUTO_UPDATE_CONTENT_COUNT;
    send_evt.header.evt_len = 10;

    /* 0, 步数数据 */
    saved_content_count    = get_contentcount_of_datatype(DATA_TYPE_STEPS);
    saved_all_data_len     = get_data_len_of_datatype(DATA_TYPE_STEPS, saved_content_count);

    aucc_param.echo_param.datatype_index                    = __REV16((uint16_t)0);
    aucc_param.echo_param.data_type                         = __REV16(DATA_TYPE_STEPS);
    aucc_param.echo_param.saved_content_count_of_datatype   = __REV16(saved_content_count);
    aucc_param.echo_param.saved_all_data_len                = __REV(saved_all_data_len);

    send_evt.evt_params.pack_trans.auto_update_content_count = aucc_param;

    protocol_send_evt(send_evt);

    /* 1, 睡眠数据 */

    saved_content_count    = get_contentcount_of_datatype(DATA_TYPE_SLEEP);
    saved_all_data_len     = get_data_len_of_datatype(DATA_TYPE_SLEEP, saved_content_count);

    aucc_param.echo_param.datatype_index                    = __REV16((uint16_t)1);
    aucc_param.echo_param.data_type                         = __REV16(DATA_TYPE_SLEEP);
    aucc_param.echo_param.saved_content_count_of_datatype   = __REV16(saved_content_count);
    aucc_param.echo_param.saved_all_data_len                = __REV(saved_all_data_len);

    send_evt.evt_params.pack_trans.auto_update_content_count = aucc_param;

    protocol_send_evt(send_evt);
    
}

static void auto_send_content_info(uint16_t data_type, uint16_t content_count)
{
    
    if (!m_is_ble_connected)
    {
        s_send_content_info_cb.busy = false;
        return;
    }

    prot_auto_update_content_info_evt_t auci_param;
    protocol_evt_t send_evt;
    uint16_t       index = 0;
    uint16_t       saved_content_count    = get_contentcount_of_datatype(DATA_TYPE_STEPS);

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_AUTO_UPDATE_CONTENT_INFO;
    send_evt.header.evt_len = 11;

    for (index = 0; index < content_count; index++)
    {
        auci_param.echo_param.content_index = __REV16(index);
        auci_param.echo_param.utc_second    = __REV(get_utc_second_of_content(data_type, index));
        auci_param.echo_param.data_len      = __REV16(get_data_len_of_content(data_type, index));
        auci_param.echo_param.data_intv     = __REV16(DATA_INTV(30, DATA_INTV_10S));;
        auci_param.echo_param.data_unit_len = 2;
        
        send_evt.evt_params.pack_trans.auto_update_content_info = auci_param;
        protocol_send_evt(send_evt);
    }

    s_send_content_info_cb.busy = false;
    
}

static void send_content_info_shced_handler(void * p_context)
{
    auto_send_content_info(s_send_content_info_cb.data_type, s_send_content_info_cb.content_count);
}

static void delete_all_sched_handler(void * p_context)
{
    
    protocol_evt_t send_evt;

    //DBG1_EVT0("Delete all");

    DeleteDataTpye(SLEEP_DATA);
    DeleteDataTpye(SPORT_DATA);

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_DEL_ALL;
    send_evt.header.evt_len = 1;
    send_evt.evt_params.pack_trans.del_all.echo_result = DELETE_SUCCESS;

    protocol_send_evt(send_evt);
    
}

static void delete_by_datatype_sched_handler(void * p_context)
{
    
    uint8_t datatype = *((uint8_t *)p_context);
    protocol_evt_t send_evt;

    P_PackTrans_Print("Delete datatype %d", datatype);

    DeleteDataTpye(datatype);

    if (datatype == SPORT_DATA)
    {
        datatype = 0;
    }
    else if (datatype == SLEEP_DATA)
    {
        datatype = 1;
    }

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_DEL_DATATYPE;
    send_evt.header.evt_len = 3;
    send_evt.evt_params.pack_trans.del_datatype.echo_param.data_type = __REV16(datatype);
    send_evt.evt_params.pack_trans.del_datatype.echo_param.result = DELETE_SUCCESS;

    protocol_send_evt(send_evt);

}

static void delete_by_utc_sched_handler(void * p_context)
{
    
    protocol_evt_t send_evt;

//    P_PackTrans_Print("Delete datatype %d utc %d", s_delete_by_utc.data_type, s_delete_by_utc.utc_second);

    DeleteDataCount(s_delete_by_utc.data_type, s_delete_by_utc.utc_second);

    if (s_delete_by_utc.data_type == SPORT_DATA)
    {
        s_delete_by_utc.data_type = 0;
    }
    else if (s_delete_by_utc.data_type == SLEEP_DATA)
    {
        s_delete_by_utc.data_type = 1;
    }

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_DEL_CONTENT_UTC;
    send_evt.header.evt_len = 7;
    send_evt.evt_params.pack_trans.del_content_utc.echo_param.data_type     = __REV16(s_delete_by_utc.data_type);
    send_evt.evt_params.pack_trans.del_content_utc.echo_param.utc_second    = __REV(s_delete_by_utc.utc_second);
    send_evt.evt_params.pack_trans.del_content_utc.echo_param.result        = DELETE_SUCCESS;

    memset((uint8_t *)&s_delete_by_utc, 0, sizeof(delete_by_utc_param_t));

    protocol_send_evt(send_evt);
    

}


bool m_protocol_pack_trans_prot_evt_execution(protocol_evt_t * p_evt)
{
    
    ASSERT(m_pack_trans_init);

    if ((p_evt->header.evt_id >> 10) != PROT_EVT_ID_GROUP_PACK_TRANS)
    {
        return false; // 不属于 pack_trans 的协议，退出
    }

    uint8_t ack_state = (uint8_t)ACK_STATE_ERROR_INTF; // 默认返回找不到接口的帧错误
    uint16_t data_type;
    uint16_t pack_index;
    uint16_t content_count;
    uint16_t content_index;
    uint32_t all_data_len;
    uint32_t utc_second;


    switch (p_evt->header.evt_id)
    {
        case PROT_EVT_ID_GET_CONTENT_COUNT:  //02 f0 10
            P_PackTrans_Print("\r\n[Get content count 02 f0 10]");
            data_type = __REV16(p_evt->evt_params.pack_trans.content_count.get_data_type);
            
            p_evt->header.evt_id = PROT_EVT_ID_RET_CONTENT_COUNT;
            p_evt->header.evt_len = 8;
        
            content_count = get_contentcount_of_datatype(data_type);
            P_PackTrans_Print("content_count: datatype: %d, count: %d", data_type, content_count);
            
            if (content_count > 9)
            {
                content_count = 9;
            }
            all_data_len  = get_data_len_of_datatype(data_type, content_count);

            p_evt->evt_params.pack_trans.content_count.ret_param.data_type      = __REV16(data_type);
            p_evt->evt_params.pack_trans.content_count.ret_param.content_count  = __REV16(content_count);
            p_evt->evt_params.pack_trans.content_count.ret_param.all_data_len   = __REV(all_data_len);

            ack_state = ACK_STATE_INVALID;
            break;

        case PROT_EVT_ID_GET_CONTENT_INFO:
            P_PackTrans_Print("\r\n[Get content information 02 f0 11]"); //02 f0 11
            data_type       = __REV16(p_evt->evt_params.pack_trans.content_info.get_param.data_type);
            content_index   = __REV16(p_evt->evt_params.pack_trans.content_info.get_param.content_index);
            
            p_evt->header.evt_id = PROT_EVT_ID_RET_CONTENT_INFO;
            p_evt->header.evt_len = 9;

            p_evt->evt_params.pack_trans.content_info.ret_param.utc_second    = __REV(get_utc_second_of_content(data_type, content_index));
            p_evt->evt_params.pack_trans.content_info.ret_param.data_len      = __REV16(get_data_len_of_content(data_type, content_index));
            p_evt->evt_params.pack_trans.content_info.ret_param.data_intv     = __REV16(DATA_INTV(30, DATA_INTV_10S));
            p_evt->evt_params.pack_trans.content_info.ret_param.data_unit_len = 2;

            ack_state = ACK_STATE_INVALID;

            break;

        case PROT_EVT_ID_GET_PKG:  
        {
            P_PackTrans_Print("\r\n[Get packages 02 f0 12]");

            data_type       = __REV16(p_evt->evt_params.pack_trans.package_req.get_param.data_type);
            utc_second      = __REV(p_evt->evt_params.pack_trans.package_req.get_param.utc_second);
            pack_index      = __REV16(p_evt->evt_params.pack_trans.package_req.get_param.pack_index);
            
            p_evt->header.evt_id = PROT_EVT_ID_RET_PKG;  //08 f0 12
            p_evt->header.evt_len = 1; 
            p_evt->evt_params.pack_trans.package_req.ret_result = DATAREQ_RESULT_SUCCESS;
            ack_state = ACK_STATE_INVALID;
            
            /*--------------*/
            uint16_t content_index = get_content_index_from_utcsecond(data_type, utc_second);
            uint16_t packs_count   = get_pack_count_of_content(data_type, content_index);
            
            /* 请求数据包命令的参数检查 */
            if (data_type >= DATA_TYPE_END)             p_evt->evt_params.pack_trans.package_req.ret_result = DATAREQ_RESULT_INVALID_DATATYPE;
            if (pack_index >= packs_count)              p_evt->evt_params.pack_trans.package_req.ret_result = DATAREQ_RESULT_INVALID_PACK_INDEX;
            if (INVALID_CONTENT_INDEX == content_index) p_evt->evt_params.pack_trans.package_req.ret_result = DATAREQ_RESULT_INVALID_CONTENT_INDEX;           
            /*--------------*/
            
            P_PackTrans_Print("s_send_pack_cb.busy: %d",s_m_send_pack_cb.busy);
            P_PackTrans_Print("\r\n[Get packages]: data_type :%d,utc_second :%d ,pack_index : %d\r\n",data_type,utc_second,pack_index);
            if ((!s_m_send_pack_cb.busy) && (p_evt->evt_params.pack_trans.package_req.ret_result == DATAREQ_RESULT_SUCCESS))
            {

                s_m_send_pack_cb.busy          = true;
                s_m_send_pack_cb.data_type     = data_type;
                s_m_send_pack_cb.packs_count   = packs_count;
                s_m_send_pack_cb.content_index = content_index;
                s_m_send_pack_cb.pack_index    = pack_index;
                
                APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, send_packs_sched_handler, NULL));
            }
            break;
        }

        case PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_COUNT:
            //P_PackTrans_Print("\r\n[Auto update content count]");
            p_evt->header.evt_id = PROT_EVT_ID_RET_AUTO_UPDATE_CONTENT_COUNT;
            p_evt->header.evt_len = 1;

            p_evt->evt_params.pack_trans.auto_update_content_count.ret_datatype_count = 2;

            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, auto_send_content_count_sched_handler, NULL));

            ack_state = ACK_STATE_INVALID;

            break;

        case PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_INFO:
            //P_PackTrans_Print("\r\n[Auto update content information]");
            data_type       = __REV16(p_evt->evt_params.pack_trans.auto_update_content_info.call_datatype);

            p_evt->header.evt_id = PROT_EVT_ID_RET_AUTO_UPDATE_CONTENT_INFO;
            p_evt->header.evt_len = 2;

            content_count = get_contentcount_of_datatype(data_type);
            p_evt->evt_params.pack_trans.auto_update_content_info.ret_content_count = __REV16(content_count);

            if (!s_send_content_info_cb.busy)
            {
                s_send_content_info_cb.content_count = content_count;
                s_send_content_info_cb.data_type = data_type;
                APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, send_content_info_shced_handler, NULL));
            }

            ack_state = ACK_STATE_INVALID;

            break;

        case PROT_EVT_ID_CALL_DEL_ALL:
            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, delete_all_sched_handler, NULL));

            ack_state = ACK_STATE_SUCCESS;

            break;

        case PROT_EVT_ID_CALL_DEL_DATATYPE:
            data_type       = __REV16(p_evt->evt_params.pack_trans.del_datatype.call_datatype);

            if (data_type == 0)
            {
                data_type = SPORT_DATA;
            }
            else if (data_type == 1)
            {
                data_type = SLEEP_DATA;
            }
            else
            {
                ack_state = ACK_STATE_ERROR_INTF_PARAM;
                break;
            }

            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, delete_by_datatype_sched_handler, &data_type));

            ack_state = ACK_STATE_SUCCESS;

            break;

        case PROT_EVT_ID_CALL_DEL_CONTENT_UTC:
        {
            memset((uint8_t *)&s_delete_by_utc, 0, sizeof(delete_by_utc_param_t));

            s_delete_by_utc.data_type       = __REV16(p_evt->evt_params.pack_trans.del_content_utc.call_param.data_type);
            s_delete_by_utc.utc_second      = __REV(p_evt->evt_params.pack_trans.del_content_utc.call_param.utc_second);
        
            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, delete_by_utc_sched_handler, NULL));

            ack_state = ACK_STATE_SUCCESS;

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

uint32_t m_protocol_pack_trans_init(void)
{
    uint32_t status = NRF_SUCCESS;

    status = app_timer_create(&x_timeout_timer_id, APP_TIMER_MODE_SINGLE_SHOT, x_timeout_timer_handler);

    m_pack_trans_init = true;

    
    memset((uint8_t *)&s_m_send_pack_cb, 0, sizeof(send_pack_block_t));

    memset((uint8_t *)&s_send_content_info_cb, 0, sizeof(send_content_info_ctrl_block_t));

    return status;
}

bool m_protocol_pack_trans_event_handler(const event_t * p_event)
{
    
    ASSERT(m_pack_trans_init);


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
                s_m_send_pack_cb.busy =false;
                s_send_content_info_cb.busy =false;
            }
            break;
    }
    
    return false;
}


