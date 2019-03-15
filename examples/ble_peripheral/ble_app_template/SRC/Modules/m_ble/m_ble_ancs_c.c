//#include "ble_ancs_c.h"

#include "includes.h"

#if CONFIG_RTT_ANCS_C_ENABLED
#define ANCS_Print(M, ...)	SEGGER_Print("ANCS", M, ##__VA_ARGS__)
#else
#define ANCS_Print(M, ...)
#endif


#define  BLE_ANCS_NOTIF_REPORT_SINGLE	0

/************************************************
    需要添加如下几个文件：
        ancs_app_attr_get.c     .\components\ble\ble_services\ble_ancs_c
        ancs_attr_parser.c
        ancs_tx_buffer.c
        nrf_ble_ancs_c.c
        nrf_ble_gatts_c.c
		
		ANCS流程：先配对，确认配对后，查找ANCS服务
************************************************/

#if (BLE_ANCS_C_ENABLED == 1)

#define ATTR_DATA_SIZE                  BLE_ANCS_ATTR_DATA_MAX                       /**< Allocated size for attribute data. */
#define SECURITY_REQUEST_DELAY          APP_TIMER_TICKS(1500)                        /**< Delay after connection until security request is sent, if necessary (ticks). */
#define ATTR_REQUEST_DELAY              APP_TIMER_TICKS(200)  
#define NOTIF_SETUP_DELAY              APP_TIMER_TICKS(100) 

APP_TIMER_DEF(m_sec_req_timer_id);
//APP_TIMER_DEF(m_attr_req_timer_id);
APP_TIMER_DEF(m_notif_setup_timer_id);
BLE_ANCS_C_DEF(m_ancs_c);                                                           /**< Apple Notification Service Client instance. */
NRF_BLE_GATTS_C_DEF(m_gatts_c);                                                     /**< GATT Service client instance. Handles Service Changed indications from the peer. */
BLE_DB_DISCOVERY_DEF(m_db_disc);                                                    /**< DB Discovery module instance. */
 

#define ANCS_DATE_SIZE      16
#define ANCS_REPORT_SIZE    10
typedef struct 
{
   ble_ancs_c_evt_notif_t       notif;
   char                         date[ANCS_DATE_SIZE];        
}ancs_attr_t; 

typedef struct 
{
    uint8_t         read;
    uint8_t         write;
    ancs_attr_t     attr[ANCS_REPORT_SIZE];
}ancs_attr_report_t; 

/* 记录推送信息 */
ancs_attr_report_t  ancs_attr_report;
/* 获取推送信息的定时器是否已经启动，避免重复启动出错 */
//static bool m_attr_req_timer_run = false;

static ble_ancs_c_evt_notif_t m_notification_incall;                                /**< Local copy to keep track of the newest arriving notifications. */
static ble_ancs_c_evt_notif_t m_notification_latest;                                /**< Local copy to keep track of the newest arriving notifications. */
static ble_ancs_c_attr_t      m_notif_attr_latest;                                  /**< Local copy of the newest notification attribute. */
//static ble_ancs_c_attr_t      m_notif_attr_app_id_latest;                           /**< Local copy of the newest app attribute. */

uint16_t     m_cur_ancs_conn_handle = BLE_CONN_HANDLE_INVALID;                    /**< Handle of the current connection. */


static uint8_t m_attr_appid[ATTR_DATA_SIZE];                                        /**< Buffer to store attribute data. */
static uint8_t m_attr_title[ATTR_DATA_SIZE];                                        /**< Buffer to store attribute data. */
//static uint8_t m_attr_subtitle[ATTR_DATA_SIZE];                                     /**< Buffer to store attribute data. */
static uint8_t m_attr_message[ATTR_DATA_SIZE];                                      /**< Buffer to store attribute data. */
static uint8_t m_attr_message_size[ATTR_DATA_SIZE];                                 /**< Buffer to store attribute data. */
static uint8_t m_attr_date[ATTR_DATA_SIZE];                                         /**< Buffer to store attribute data. */
//static uint8_t m_attr_posaction[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */
//static uint8_t m_attr_negaction[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */
//static uint8_t m_attr_disp_name[ATTR_DATA_SIZE];                                    /**< Buffer to store attribute data. */

#if CONFIG_RTT_ANCS_C_ENABLED
/**@brief String literals for the iOS notification categories. used then printing to UART. */
static char const * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
{
    "Other",
    "Incoming Call",
    "Missed Call",
    "Voice Mail",
    "Social",
    "Schedule",
    "Email",
    "News",
    "Health And Fitness",
    "Business And Finance",
    "Location",
    "Entertainment"
};

/**@brief String literals for the iOS notification event types. Used then printing to UART. */
static char const * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
{
    "Added",
    "Modified",
    "Removed"
};

/**@brief String literals for the iOS notification attribute types. Used when printing to UART. */
static char const * lit_attrid[BLE_ANCS_NB_OF_NOTIF_ATTR] =
{
    "App Identifier",
    "Title",
    "Subtitle",
    "Message",
    "Message Size",
    "Date",
    "Positive Action Label",
    "Negative Action Label"
};

/**@brief String literals for the iOS notification attribute types. Used When printing to UART. */
static char const * lit_appid[BLE_ANCS_NB_OF_APP_ATTR] =
{
    "Display Name"
};
#endif
/* 记录推送的消息参数 */
//void ancs_notif_get(ble_ancs_c_evt_notif_t *p_notif)
//{
//    memcpy(&ancs_attr_report.attr[ancs_attr_report.write++].notif, p_notif, sizeof(ble_ancs_c_evt_notif_t));

//    if (ancs_attr_report.write >= ANCS_REPORT_SIZE)
//        ancs_attr_report.write = 0;


//    /* 即将获取推送信息 */
//    ble_ancs_c_attr_req_timer_start();
//}

/* 获取推送消息的内容 */
//uint8_t ancs_attr_get(uint8_t *str)
//{
//    uint8_t     index;

//    for (index = 0; index < ANCS_REPORT_SIZE; index++)
//    {
//        /* 查找是否是已经推送过的数据 */
//        if (0 == memcmp(&ancs_attr_report.attr[index].date, str, ANCS_DATE_SIZE))
//            break;
//    }

//    /* 收到的是新内容，存起来 */
//    if (index >= ANCS_REPORT_SIZE)
//    {
//        memcpy(&ancs_attr_report.attr[ancs_attr_report.read].date, str, ANCS_DATE_SIZE);

//        /* 记录新数据存放的位置偏移量 */
//        index = ancs_attr_report.read;
//    }
//    else
//    {
//        /* 无效的数据，无效的偏移量 */
//        index = ANCS_REPORT_SIZE;
//    }

//    /* 还有内容没有获取完毕 */
//    if (ancs_attr_report.read != ancs_attr_report.write)
//    {
//        //继续获取新的信息
//        ble_ancs_c_attr_req_timer_start();
//    }
//    return index;
//}


/*只判断来电通知*/
static void send_msg_event_from_notify(ble_ancs_c_evt_notif_t notify)
{
    ANCS_Print("send_msg_event_from_notify");
    bool                    is_valid_event  = true;
    uint32_t *              p_bitmap        = NULL;
    uint32_t                bitmap          = 0;
    enum app_recv_event_e   evt             = APP_RECV_EVT_INVALID;
    

    if (notify.evt_flags.pre_existing == 1)
    {
        ANCS_Print("Pre existing notify");
        return;
    }

    if (notify.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED) 
    {
        ANCS_Print("APP_RECV_EVT_MSG_NEW");
        p_bitmap = &system_database.new_msg_bitmap;
        
        evt = APP_RECV_EVT_MSG_NEW;
    }
    else if (notify.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)
    {
        ANCS_Print("BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED");
        p_bitmap = &system_database.cancel_msg_bitmap;
        evt = APP_RECV_EVT_MSG_CANCEL;
    }

    if (notify.category_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL) 
    {
        
        ANCS_Print("BLE_ANCS_CATEGORY_ID_INCOMING_CALL");
        SET_VALID_BIT(bitmap, MSG_BIT_CALLIN, system_database.msg_switch_bitmap);
        m_notification_incall = notify;
    }
    else if (notify.category_id == BLE_ANCS_CATEGORY_ID_MISSED_CALL) 
    {
        ANCS_Print("BLE_ANCS_CATEGORY_ID_MISSED_CALL");
//        SET_VALID_BIT(bitmap, MSG_BIT_MISS_CALL, system_database.msg_switch_bitmap);
    }
    else
    {
        is_valid_event = false;
    }

    ANCS_Print("system_database.msg_switch_bitmap %04x", system_database.msg_switch_bitmap);
    ANCS_Print("is_valid_event :%d  bitmap :%04x, evt :%d  dnd_cur_state: %d",is_valid_event,bitmap,evt,system_database.dnd_cur_state);
    if ((is_valid_event) && (bitmap) && (evt != APP_RECV_EVT_INVALID)&&(!system_database.dnd_cur_state))
    {
        *p_bitmap = bitmap;
        ANCS_Print("event_send: ancs notify event: bitmap: %04x, evt: %d", *p_bitmap, evt);
        APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, evt));
    }
}

// 分析ANCS的消息类型
static void send_msg_event_from_app_id(uint8_t * app_id_str)
{
    ANCS_Print("send_msg_event_from_app_id");
    ANCS_Print("send %s event", (uint32_t)app_id_str);
#if 1
    const uint8_t app_id_qq[] =             {"com.tencent.mqq"};           // tecent qq
    const uint8_t app_id_tim[] =            {"com.tencent.tim"};           // tecent tim
    const uint8_t app_id_weixin[] =         {"com.tencent.xin"};           // tencent weixin
    const uint8_t app_id_tcweibo[] =        {"com.tencent.WeiBo"};         // tecent weibo
    const uint8_t app_id_skype[] =          {"com.skype.tomskype"};        // skype
    const uint8_t app_id_sinaweibo[] =      {"com.sina.weibo"};            // sina weibo
    const uint8_t app_id_facebook[] =       {"com.facebook.Messenger"};    // facebook
    const uint8_t app_id_facebook2[] =      {"com.facebook.Facebook"};     // facebook2
    const uint8_t app_id_twitter[] =        {"com.atebits.Tweetie2"};      // twitter
    const uint8_t app_id_whatsapp[] =       {"net.whatsapp.WhatsApp"};     // whatsapp
    const uint8_t app_id_line[] =           {"jp.naver.line"};             // line
    const uint8_t app_id_message[] =        {"com.apple.MobileSMS"};       // message
//    const uint8_t app_id_callling[] =       {"com.apple.mobilephone"};     // phone


#endif
    uint32_t *              p_bitmap        = NULL;
    uint32_t                bitmap          = 0;
    enum app_recv_event_e   evt             = APP_RECV_EVT_INVALID;

    if (m_notification_latest.evt_flags.pre_existing == 1)
    {
        ANCS_Print("Pre existing app id");
        return;
    }

    if (m_notification_latest.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED) 
    {
        p_bitmap = &system_database.new_msg_bitmap;
        evt = APP_RECV_EVT_MSG_NEW;
    }
    else if (m_notification_latest.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)
    {
        p_bitmap = &system_database.cancel_msg_bitmap;
        evt = APP_RECV_EVT_MSG_CANCEL;
    }
    
   

    bitmap = 0;

    if (0 == memcmp(app_id_qq,           app_id_str, sizeof(app_id_qq)-1))          SET_VALID_BIT(bitmap, MSG_BIT_QQ, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_tim,          app_id_str, sizeof(app_id_tim)-1))         SET_VALID_BIT(bitmap, MSG_BIT_TIM, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_weixin,       app_id_str, sizeof(app_id_weixin)-1))      SET_VALID_BIT(bitmap, MSG_BIT_WECHAT, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_tcweibo,      app_id_str, sizeof(app_id_tcweibo)-1))     SET_VALID_BIT(bitmap, MSG_BIT_TC_WEIBO, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_skype,        app_id_str, sizeof(app_id_skype)-1))       SET_VALID_BIT(bitmap, MSG_BIT_SKYPE, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_sinaweibo,    app_id_str, sizeof(app_id_sinaweibo)-1))   SET_VALID_BIT(bitmap, MSG_BIT_XL_WEIBO, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_facebook,     app_id_str, sizeof(app_id_facebook)-1))    SET_VALID_BIT(bitmap, MSG_BIT_FACEBOOK, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_facebook2,    app_id_str, sizeof(app_id_facebook2)-1))   SET_VALID_BIT(bitmap, MSG_BIT_FACEBOOK, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_twitter,      app_id_str, sizeof(app_id_twitter)-1))     SET_VALID_BIT(bitmap, MSG_BIT_TWITTER, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_whatsapp,     app_id_str, sizeof(app_id_whatsapp)-1))    SET_VALID_BIT(bitmap, MSG_BIT_WHATSAPP, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_line,         app_id_str, sizeof(app_id_line)-1))        SET_VALID_BIT(bitmap, MSG_BIT_LINE, system_database.msg_switch_bitmap);
    if (0 == memcmp(app_id_message,      app_id_str, sizeof(app_id_message)-1))     SET_VALID_BIT(bitmap, MSG_BIT_MSG, system_database.msg_switch_bitmap);
//    if (0 == memcmp(app_id_callling,     app_id_str, sizeof(app_id_callling)-1))    SET_VALID_BIT(bitmap, MSG_BIT_CALLIN, system_database.msg_switch_bitmap);

    
    ANCS_Print("ancs app id event 1:  evt: %d, system_database.msg_switch_bitmap: %04x", evt, system_database.msg_switch_bitmap);
    ANCS_Print("bitmap :%04x  p_bitmap :%04x, dnd_cur_state: %d",bitmap,*p_bitmap,system_database.dnd_cur_state);
    if (bitmap && p_bitmap && (evt != APP_RECV_EVT_INVALID) &&(!system_database.dnd_cur_state))
    {
        *p_bitmap = bitmap;
        ANCS_Print("event_send : bitmap: %04x, evt: %d", bitmap, evt);
        APP_ERROR_CHECK(event_send(EVT_COMS_APP_RECV, evt));
    }
}


/**@brief Function for handling events from the GATT Servive client module.

   @param[in] p_evt GATT Service event.
*/
static void gatts_c_evt_handler(nrf_ble_gatts_c_evt_t * p_evt)
{
    ret_code_t ret;

    switch (p_evt->evt_type)
    {
        case NRF_BLE_GATTS_C_EVT_DISCOVERY_COMPLETE:
        {
            ANCS_Print("GATT Service and Service Changed characteristic found on server %d.", p_evt->conn_handle);

            ret = nrf_ble_gatts_c_handles_assign(&m_gatts_c,
                                                 p_evt->conn_handle,
                                                 &p_evt->params.srv_changed_char);
            APP_ERROR_CHECK(ret);

            ret = nrf_ble_gatts_c_enable_indication(&m_gatts_c, true);
            APP_ERROR_CHECK(ret);
        } break;

        case NRF_BLE_GATTS_C_EVT_DISCOVERY_FAILED:
            ANCS_Print("GATT Service or Service Changed characteristic not found on server.");
            break;

        case NRF_BLE_GATTS_C_EVT_DISCONN_COMPLETE:
            ANCS_Print("GATTS Service client disconnected connection handle %d.", p_evt->conn_handle);
            break;

        case NRF_BLE_GATTS_C_EVT_SRV_CHANGED:
            ANCS_Print("Service Changed indication received.");

            // Discover peer's services.
            ret = ble_db_discovery_start(&m_db_disc, p_evt->conn_handle);
            if(ret != NRF_ERROR_BUSY)
            {
                APP_ERROR_CHECK(ret);
            }
            break;

        default:
            break;
    }
}


/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_notification_setup(void * p_context)
{
    ret_code_t ret;

//    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    ret = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    ret = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(ret);

    ANCS_Print("Notifications Enabled.");
}


/**@brief Function for printing an iOS notification.
 *
 * @param[in] p_notif  Pointer to the iOS notification.
 */
static void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{
    ANCS_Print("\r\nNotification");
    ANCS_Print("Event:       %s", lit_eventid[p_notif->evt_id]);
    ANCS_Print("Category ID: %s", lit_catid[p_notif->category_id]);		// 消息类型
    ANCS_Print("Category Cnt:%u", p_notif->category_count);
    ANCS_Print("UID:         %u", p_notif->notif_uid);					// 身份证
	
    ANCS_Print("Flags:");
    if (p_notif->evt_flags.silent == 1)
    {
        ANCS_Print(" Silent");
    }
    if (p_notif->evt_flags.important == 1)
    {
        ANCS_Print(" Important");
    }
    if (p_notif->evt_flags.pre_existing == 1)
    {
        ANCS_Print(" Pre-existing");
    }
    if (p_notif->evt_flags.positive_action == 1)
    {
        ANCS_Print(" Positive Action");
    }
    if (p_notif->evt_flags.negative_action == 1)
    {
        ANCS_Print(" Negative Action");
    }
}


/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS notification attribute.
 */
static void notif_attr_print(ble_ancs_c_attr_t * p_attr)
{
    if (p_attr->attr_len != 0)
    {
        ANCS_Print("%s: %s", lit_attrid[p_attr->attr_id], p_attr->p_attr_data);
//        if (0 == memcmp(lit_attrid[p_attr->attr_id], "Date", 4))
//        {
//            uint8_t     index = ancs_attr_get(p_attr->p_attr_data);
//            if (index < ANCS_REPORT_SIZE)
//            {
//                ANCS_Print("Category ID: %s", lit_catid[ancs_attr_report.attr[index].notif.category_id]);
//                ANCS_Print("%s: %s", lit_attrid[p_attr->attr_id], ancs_attr_report.attr[index].date);
//            }
//        }
    }
    else if (p_attr->attr_len == 0)
    {
        ANCS_Print("%s: (N/A)", lit_attrid[p_attr->attr_id]);
    }
}


/**@brief Function for printing iOS notification attribute data.
 *
 * @param[in] p_attr Pointer to an iOS App attribute.
 */
static void app_attr_print(ble_ancs_c_attr_t * p_attr)
{
    if (p_attr->attr_len != 0)
    {
        ANCS_Print("%s: %s", lit_appid[p_attr->attr_id], p_attr->p_attr_data);
    }
    else if (p_attr->attr_len == 0)
    {
        ANCS_Print("%s: (N/A)", lit_appid[p_attr->attr_id]);
    }
}


/**@brief Function for printing out errors that originated from the Notification Provider (iOS).
 *
 * @param[in] err_code_np Error code received from NP.
 */
static void err_code_print(uint16_t err_code_np)
{
    switch (err_code_np)
    {
        case BLE_ANCS_NP_UNKNOWN_COMMAND:
            ANCS_Print("Error: Command ID was not recognized by the Notification Provider. ");
            break;

        case BLE_ANCS_NP_INVALID_COMMAND:
            ANCS_Print("Error: Command failed to be parsed on the Notification Provider. ");
            break;

        case BLE_ANCS_NP_INVALID_PARAMETER:
            ANCS_Print("Error: Parameter does not refer to an existing object on the Notification Provider. ");
            break;

        case BLE_ANCS_NP_ACTION_FAILED:
            ANCS_Print("Error: Perform Notification Action Failed on the Notification Provider. ");
            break;

        default:
            break;
    }
}

/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
static void sec_req_timeout_handler(void * p_context)
{
    ret_code_t           ret;
    pm_conn_sec_status_t status;

    if (m_cur_ancs_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        ret = pm_conn_sec_status_get(m_cur_ancs_conn_handle, &status);
        APP_ERROR_CHECK(ret);

        // If the link is still not secured by the peer, initiate security procedure.
        if (!status.encrypted)
        {
            ret = pm_conn_secure(m_cur_ancs_conn_handle, false);
            if (ret != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(ret);
            }
        }
    }
}


//callback by on_ble_peripheral_evt -> connected
void ble_ancs_c_sec_req_timer_start(uint16_t conn_handle)
{
    ret_code_t ret;
    
    m_cur_ancs_conn_handle = conn_handle;

    ret = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
    APP_ERROR_CHECK(ret);
}

//callback by on_ble_peripheral_evt -> disconnected
void ble_ancs_c_sec_req_timer_stop(uint16_t conn_handle)
{
    ret_code_t ret;

    m_cur_ancs_conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ret = app_timer_stop(m_sec_req_timer_id);
    APP_ERROR_CHECK(ret);
    
    if (conn_handle == m_ancs_c.conn_handle)
    {
        m_ancs_c.conn_handle = BLE_CONN_HANDLE_INVALID;
    }
}

//callback by ancs_c_evt_handler -> BLE_ANCS_C_EVT_DISCOVERY_COMPLETE
void apple_notification_start(void)
{
    ret_code_t ret; 

    ret = app_timer_start(m_notif_setup_timer_id, NOTIF_SETUP_DELAY, NULL);
    APP_ERROR_CHECK(ret);
}

/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void ancs_c_evt_handler(ble_ancs_c_evt_t * p_evt)
{
    ret_code_t ret = NRF_SUCCESS;

    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
            ANCS_Print("Apple Notification Center Service discovered on the server.");
            ret = nrf_ble_ancs_c_handles_assign(&m_ancs_c, p_evt->conn_handle, &p_evt->service);
            APP_ERROR_CHECK(ret);
            apple_notification_start();
            break;

        case BLE_ANCS_C_EVT_NOTIF:
            m_notification_latest = p_evt->notif;

//            ancs_notif_get(&m_notification_latest);
            notif_print(&m_notification_latest);
		
			if (1 != m_notification_latest.evt_flags.pre_existing)
			{
				// 只判断来电消息提醒，来电消息将不会运行后面的if
				send_msg_event_from_notify(m_notification_latest);
				// 新来的消息，既不是来电，也不是未接，进一步获取属性
				if ((m_notification_latest.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED) 		&&
					(m_notification_latest.category_id != BLE_ANCS_CATEGORY_ID_INCOMING_CALL)	&&
					(m_notification_latest.category_id != BLE_ANCS_CATEGORY_ID_MISSED_CALL))
				{
					ANCS_Print("nrf_ble_ancs_c_request_attrs. ");
					APP_ERROR_CHECK(nrf_ble_ancs_c_request_attrs(&m_ancs_c, &m_notification_latest));
				}
			}
//            ret = nrf_ble_ancs_c_request_attrs(&m_ancs_c, &m_notification_latest);  //add 2018-04-09 09:17
//            APP_ERROR_CHECK(ret);
            
            break;

        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
            m_notif_attr_latest = p_evt->attr;
			m_notification_latest = p_evt->notif;
            notif_attr_print(&m_notif_attr_latest);
            if (p_evt->attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
            {
//                m_notif_attr_app_id_latest = p_evt->attr;
				send_msg_event_from_app_id(m_notif_attr_latest.p_attr_data);
            }
            break;
        case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
            ANCS_Print("Apple Notification Center Service not discovered on the server.");
            break;

        case BLE_ANCS_C_EVT_APP_ATTRIBUTE:
            app_attr_print(&p_evt->attr);
            break;
        case BLE_ANCS_C_EVT_NP_ERROR:
            err_code_print(p_evt->err_code_np);
            break;
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_notification_error_handler(uint32_t nrf_error)
{
    ANCS_Print("[func: %s][%d]\n\r", __func__, nrf_error);
}

/**@brief Function for initializing ancs_c services that will be used by the application.
 */
void ble_ancs_c_services_init(void)
{
    ret_code_t             ret;
    ble_ancs_c_init_t      ancs_c_init;
//    nrf_ble_gatts_c_init_t gatts_c_init;
	
	ble_ancs_c_timer_init();

	/*
	*	SDK15 ANCS存在Bug：	第一次连接设备配对后，保持蓝牙不断开连接，此时ANCS工作正常；
	*						若某次断开连接，则接下来，IOS在若干秒后将自动断开，然后又自动连接，如此循环；
	*						官方给出的建议是注释下方 nrf_ble_gatts_c_init，或者把服务改变的数据保存，再次连接时使用，
	*						后者操作复杂且说明模糊，所以该程序未采纳
	*						nrf_ancs_perform_notif_action 该函数可接听或挂断来电
	*/
    // Init the GATTS client module.
//    memset(&gatts_c_init, 0, sizeof(gatts_c_init));

//    gatts_c_init.evt_handler = gatts_c_evt_handler;

//    ret = nrf_ble_gatts_c_init(&m_gatts_c, &gatts_c_init);
//    APP_ERROR_CHECK(ret);

    // Init the Apple Notification Center Service client module.
    memset(&ancs_c_init, 0, sizeof(ancs_c_init));

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER,
                                  m_attr_appid,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

//    ret = nrf_ble_ancs_c_app_attr_add(&m_ancs_c,
//                                      BLE_ANCS_APP_ATTR_ID_DISPLAY_NAME,
//                                      m_attr_disp_name,
//                                      sizeof(m_attr_disp_name));
//    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_TITLE,
                                  m_attr_title,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
                                  m_attr_message,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

//    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                  BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
//                                  m_attr_subtitle,
//                                  ATTR_DATA_SIZE);
//    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
                                  m_attr_message_size,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
                                  BLE_ANCS_NOTIF_ATTR_ID_DATE,
                                  m_attr_date,
                                  ATTR_DATA_SIZE);
    APP_ERROR_CHECK(ret);

//    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                  BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
//                                  m_attr_posaction,
//                                  ATTR_DATA_SIZE);
//    APP_ERROR_CHECK(ret);

//    ret = nrf_ble_ancs_c_attr_add(&m_ancs_c,
//                                  BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
//                                  m_attr_negaction,
//                                  ATTR_DATA_SIZE);
//    APP_ERROR_CHECK(ret);

    ancs_c_init.evt_handler   = ancs_c_evt_handler;
    ancs_c_init.error_handler = apple_notification_error_handler;

    ret = ble_ancs_c_init(&m_ancs_c, &ancs_c_init);
    APP_ERROR_CHECK(ret);
}

/**@brief Function for handling Database Discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective service instances.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
    nrf_ble_gatts_c_on_db_disc_evt(&m_gatts_c, p_evt);
}

void db_disc_start(uint16_t conn_handle)
{
    ret_code_t  ret;
    ret  = ble_db_discovery_start(&m_db_disc, conn_handle);
    if(ret != NRF_ERROR_BUSY)
    {
        APP_ERROR_CHECK(ret);
    }
}

void m_ancs_conn_handle_set(uint16_t conn_handle)
{
    
}

//callback by ble_ancs_c_attr_req_timer_init
//void attr_req_timeout_handler(void * p_context)
//{
//    ret_code_t      ret;

//    /* 请求推送数据 */
//    ret = nrf_ble_ancs_c_request_attrs(&m_ancs_c, &ancs_attr_report.attr[ancs_attr_report.read].notif);  //add 2018-04-09 09:17
//    APP_ERROR_CHECK(ret);

//    if (++ancs_attr_report.read >= ANCS_REPORT_SIZE)
//           ancs_attr_report.read = 0; 

//    m_attr_req_timer_run = false;
//}


//callback by ancs_c_evt_handler
void ble_ancs_c_attr_req_timer_start(void)
{
//    ret_code_t ret; 

//    if (true == m_attr_req_timer_run)
//        return;

//    ret = app_timer_start(m_attr_req_timer_id, ATTR_REQUEST_DELAY, NULL);
//    APP_ERROR_CHECK(ret);

//    m_attr_req_timer_run = true;
}

//callback by 
void ble_ancs_c_attr_req_timer_stop(void)
{
//    ret_code_t ret;

//    ret = app_timer_stop(m_attr_req_timer_id);
//    APP_ERROR_CHECK(ret);

//    m_attr_req_timer_run = false;
}


/* 无需 ANCS 处理拒接时，返回 false */
bool m_coms_ble_ancs_c_reject_callin(void)
{
    if (m_notification_incall.category_id != BLE_ANCS_CATEGORY_ID_INCOMING_CALL)
    {
        return false;
    }
    uint32_t ret = NRF_SUCCESS;

    if(m_notification_incall.evt_flags.negative_action == true)
    {
        ret = nrf_ancs_perform_notif_action(&m_ancs_c,
                                            m_notification_incall.notif_uid,
                                            ACTION_ID_NEGATIVE);
        APP_ERROR_CHECK(ret);
    }

    return true;
}


//callback by timers_init
void ble_ancs_c_timer_init(void)
{
    ret_code_t ret;

    // Create security request timer.
    ret = app_timer_create(&m_sec_req_timer_id,
                           APP_TIMER_MODE_SINGLE_SHOT,
                           sec_req_timeout_handler);
    APP_ERROR_CHECK(ret);
    
    
//    ret = app_timer_create(&m_attr_req_timer_id,
//                           APP_TIMER_MODE_SINGLE_SHOT,
//                           attr_req_timeout_handler);
//    APP_ERROR_CHECK(ret);
    
    ret = app_timer_create(&m_notif_setup_timer_id,
                           APP_TIMER_MODE_SINGLE_SHOT,
                           apple_notification_setup);
    APP_ERROR_CHECK(ret);  
}

#endif

