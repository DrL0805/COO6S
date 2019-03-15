/**
 * Copyright (c) 2012 - 2018, Nordic Semiconductor ASA
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
#include "sdk_common.h"
//#include "bsp_cfg.h"

extern uint8_t m_ble_max_data_len;

#if NRF_MODULE_ENABLED(BLE_LOWSPP)
#include "ble.h"
#include "m_ble_lowspp.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "string.h"
#include "app_scheduler.h"

#include "app_fifo.h"
#include "pack_fifo.h"
#include "m_protocol.h"

#include "resources.h"
#include "includes.h"

//#include "m_pack_send.h"

#if CONFIG_RTT_LOWSPP_ENABLED
#define LOWSAPP_Print(M, ...)	SEGGER_Print("LOWSPP", M, ##__VA_ARGS__)
#else
#define LOWSAPP_Print(M, ...)
#endif

//static uint8_t bt_test[256] = {0};


static app_fifo_t           m_recv_fifo;
static uint8_t              recv_buffer[M_COMS_BLE_LOWSPP_RECV_BUFFER_SIZE_MAX] = {0};
static pack_fifo_t          m_send_pack_fifo;
static pack_t               send_pack_buffer[M_COMS_BLE_LOWSPP_SEND_BUFFER_PACK_COUNT_MAX] = {0};
static bool                 m_send_busy = false;


__STATIC_INLINE void ble_lowspp_recv_handler(uint8_t chn, uint8_t * p_data, uint16_t length);
__STATIC_INLINE void ble_lowspp_send_handler(void);

//uint8_t  test_buf[256];


BLE_LOWSPP_DEF(m_lowspp, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */

#define BLE_UUID_LOWSPP_CH1_CHARACTERISTIC 0xA801               /**< The UUID of the TX Characteristic. */
#define BLE_UUID_LOWSPP_CH2_CHARACTERISTIC 0xA802               /**< The UUID of the RX Characteristic. */
#define BLE_UUID_LOWSPP_CH3_CHARACTERISTIC 0xA803               /**< The UUID of the RX Characteristic. */

#define BLE_LOWSPP_CH1_MAX_CHAR_LEN        BLE_LOWSPP_MAX_DATA_LEN /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_LOWSPP_CH2_MAX_CHAR_LEN        BLE_LOWSPP_MAX_DATA_LEN
#define BLE_LOWSPP_CH3_MAX_CHAR_LEN        BLE_LOWSPP_MAX_DATA_LEN
//#define BLE_NUS_MAX_TX_CHAR_LEN        BLE_LOWSPP_MAX_DATA_LEN /**< Maximum length of the TX Characteristic (in bytes).
#define LOWSPP_BASE_UUID                 {{0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}} /**< Used vendor specific UUID. */

#if (BLE_LOWSPP_CH1_ENABLED == 1) 
static uint8_t ch1_value_buf[BLE_LOWSPP_CH1_MAX_CHAR_LEN];
#endif

#if (BLE_LOWSPP_CH2_ENABLED == 1) 
static uint8_t ch2_value_buf[BLE_LOWSPP_CH2_MAX_CHAR_LEN];
#endif

#if (BLE_LOWSPP_CH3_ENABLED == 1) 
static uint8_t ch3_value_buf[BLE_LOWSPP_CH3_MAX_CHAR_LEN];
#endif

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_lowspp_t * p_lowspp, ble_evt_t const * p_ble_evt)
{
//    ret_code_t                      err_code;
//    ble_lowspp_evt_t                evt;
//    ble_gatts_value_t               gatts_val;
//    uint8_t                         cccd_value[2];
//    ble_lowspp_client_context_t     * p_client = NULL;

//    err_code = blcm_link_ctx_get(p_lowspp->p_link_ctx_storage,
//                                 p_ble_evt->evt.gap_evt.conn_handle,
//                                 (void *) &p_client);
//    if (err_code != NRF_SUCCESS)
//    {
//        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
//                      p_ble_evt->evt.gap_evt.conn_handle);
//    }
    
//    NRF_LOG_INFO("start cli1: %d\n\r", p_client->is_ch1_notification_enabled);
//    NRF_LOG_INFO("start cli2: %d\n\r", p_client->is_ch2_notification_enabled);
//    NRF_LOG_INFO("start cli3: %d\n\r", p_client->is_ch3_notification_enabled);

//    /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
//    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
//    gatts_val.p_value = cccd_value;
//    gatts_val.len     = sizeof(cccd_value);
//    gatts_val.offset  = 0;

//    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
//                                      p_lowspp->ch1_handles.cccd_handle,
//                                      &gatts_val);

//    if ((err_code == NRF_SUCCESS)     &&
//        (p_lowspp->ch1_data_handler != NULL) &&
//        ble_srv_is_notification_enabled(gatts_val.p_value))
//    {
//        if (p_client != NULL)
//        {
//            p_client->is_ch1_notification_enabled = true;
//        }

//        memset(&evt, 0, sizeof(ble_lowspp_evt_t));
//        evt.type        = BLE_LOWSPP_EVT_COMM_STARTED;
//        evt.p_lowspp    = p_lowspp;
//        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//        evt.p_link_ctx  = p_client;

//        p_lowspp->ch1_data_handler(&evt);
//    }
    
    
//    NRF_LOG_INFO("on ccCC\n\r");
    p_lowspp->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}
static void on_disconnect(ble_lowspp_t * p_lowspp, ble_evt_t const * p_ble_evt)
{
    ret_code_t                      err_code;
    ble_lowspp_client_context_t     * p_client = NULL;

    err_code = blcm_link_ctx_get(p_lowspp->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *) &p_client);
    
    if (err_code != NRF_SUCCESS)
    {
    
    }
//    NRF_LOG_INFO("stop cli1: %d\n\r", p_client->is_ch1_notification_enabled);
//    NRF_LOG_INFO("stop cli2: %d\n\r", p_client->is_ch2_notification_enabled);
//    NRF_LOG_INFO("stop cli3: %d\n\r", p_client->is_ch3_notification_enabled);
    
    #if (BLE_LOWSPP_CH1_ENABLED == 1) 
    p_client->is_ch1_notification_enabled = false;
    #endif
    
    #if (BLE_LOWSPP_CH2_ENABLED == 1) 
    p_client->is_ch2_notification_enabled = false;
    #endif
    
    #if (BLE_LOWSPP_CH3_ENABLED == 1) 
    p_client->is_ch3_notification_enabled = false;
    #endif
    
    p_lowspp->conn_handle = BLE_CONN_HANDLE_INVALID; 
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_lowspp_t * p_lowspp, ble_evt_t const * p_ble_evt)
{
    ret_code_t                          err_code;
    ble_lowspp_evt_t                    evt;
    ble_lowspp_client_context_t         * p_client;
    ble_gatts_evt_write_t const         * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    err_code = blcm_link_ctx_get(p_lowspp->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        LOWSAPP_Print("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
    }

    memset(&evt, 0, sizeof(ble_lowspp_evt_t));
    evt.p_lowspp    = p_lowspp;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    #if (BLE_LOWSPP_CH1_ENABLED == 1) 
    if ((p_evt_write->handle == p_lowspp->ch1_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_ch1_notification_enabled = true;
                evt.type                          = BLE_LOWSPP_EVT_COMM_STARTED;
            }
            else
            {
                p_client->is_ch1_notification_enabled = false;
                evt.type                          = BLE_LOWSPP_EVT_COMM_STOPPED;
            }

            if (p_lowspp->ch1_data_handler != NULL)
            {
                p_lowspp->ch1_data_handler(&evt);
            }

        }
        return;
    }
    else if ((p_evt_write->handle == p_lowspp->ch1_handles.value_handle) &&
             (p_lowspp->ch1_data_handler != NULL))
    {
        evt.type                  = BLE_LOWSPP_EVT_RX_DATA;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_lowspp->ch1_data_handler(&evt);
        return;
    }
    #endif
    
    #if (BLE_LOWSPP_CH2_ENABLED == 1) 
    if ((p_evt_write->handle == p_lowspp->ch2_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_ch2_notification_enabled = true;
                evt.type                          = BLE_LOWSPP_EVT_COMM_STARTED;
            }
            else
            {
                p_client->is_ch2_notification_enabled = false;
                evt.type                          = BLE_LOWSPP_EVT_COMM_STOPPED;
            }

            if (p_lowspp->ch2_data_handler != NULL)
            {
                p_lowspp->ch2_data_handler(&evt);
            }
        }
        return;
    }
    else if ((p_evt_write->handle == p_lowspp->ch2_handles.value_handle) &&
             (p_lowspp->ch2_data_handler != NULL))
    {
        evt.type                  = BLE_LOWSPP_EVT_RX_DATA;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_lowspp->ch2_data_handler(&evt);
        return;
    }
    #endif
    
    #if (BLE_LOWSPP_CH3_ENABLED == 1) 
    if ((p_evt_write->handle == p_lowspp->ch3_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_ch3_notification_enabled = true;
                evt.type                          = BLE_LOWSPP_EVT_COMM_STARTED;
            }
            else
            {
                p_client->is_ch3_notification_enabled = false;
                evt.type                          = BLE_LOWSPP_EVT_COMM_STOPPED;
            }

            if (p_lowspp->ch3_data_handler != NULL)
            {
                p_lowspp->ch3_data_handler(&evt);
            }

        }
    }
    else if ((p_evt_write->handle == p_lowspp->ch3_handles.value_handle) &&
             (p_lowspp->ch3_data_handler != NULL))
    {
        evt.type                  = BLE_LOWSPP_EVT_RX_DATA;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_lowspp->ch3_data_handler(&evt);
    }
    #endif
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
#if 0
static void on_hvx_tx_complete(ble_lowspp_t * p_lowspp, ble_evt_t const * p_ble_evt)
{
    ret_code_t                      err_code;
    ble_lowspp_evt_t                evt;
    ble_lowspp_client_context_t     * p_client;
    ble_gatts_evt_write_t const     * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;


    err_code = blcm_link_ctx_get(p_lowspp->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        LOWSAPP_Print("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
        return;
    }
	
    #if (BLE_LOWSPP_CH1_ENABLED == 1) 
    if ((p_client->is_ch1_notification_enabled) && (p_evt_write->handle == p_lowspp->ch1_handles.value_handle))
    {
        memset(&evt, 0, sizeof(ble_lowspp_evt_t));
        evt.type        = BLE_LOWSPP_EVT_TX_RDY;
        evt.p_lowspp       = p_lowspp;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        if (p_lowspp->ch1_data_handler != NULL)
            p_lowspp->ch1_data_handler(&evt);
    }
    #endif
    
    
    #if (BLE_LOWSPP_CH2_ENABLED == 1) 
    if ((p_client->is_ch2_notification_enabled) && (p_evt_write->handle == p_lowspp->ch2_handles.value_handle))
    {
        memset(&evt, 0, sizeof(ble_lowspp_evt_t));
        evt.type        = BLE_LOWSPP_EVT_TX_RDY;
        evt.p_lowspp       = p_lowspp;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        if (p_lowspp->ch2_data_handler != NULL)
            p_lowspp->ch2_data_handler(&evt);
    }
    #endif
    
    
    #if (BLE_LOWSPP_CH3_ENABLED == 1) 
    if ((p_client->is_ch3_notification_enabled) && (p_evt_write->handle == p_lowspp->ch3_handles.value_handle))
    {
        memset(&evt, 0, sizeof(ble_lowspp_evt_t));
        evt.type        = BLE_LOWSPP_EVT_TX_RDY;
        evt.p_lowspp       = p_lowspp;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        if (p_lowspp->ch3_data_handler != NULL)
            p_lowspp->ch3_data_handler(&evt);
    }
    #endif
}
#endif



/**@brief Function for adding RX characteristic.
 *
 * @param[in] p_nus       Nordic UART Service structure.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
#if (BLE_LOWSPP_CH1_ENABLED == 1) 
static uint32_t ch1_char_add(ble_lowspp_t * p_lowspp)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_md_t cccd_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_lowspp->uuid_type;
    ble_uuid.uuid = BLE_UUID_LOWSPP_CH1_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_LOWSPP_CH1_MAX_CHAR_LEN;
    attr_char_value.p_value   = ch1_value_buf;

    return sd_ble_gatts_characteristic_add(p_lowspp->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_lowspp->ch1_handles);
}

uint32_t ble_lowspp_ch1_data_send(ble_lowspp_t * p_lowspp,
                           uint8_t   * p_data,
                           uint16_t  length)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    ble_lowspp_client_context_t * p_client;

    VERIFY_PARAM_NOT_NULL(p_lowspp);

    err_code = blcm_link_ctx_get(p_lowspp->p_link_ctx_storage, p_lowspp->conn_handle, (void *) &p_client);
    VERIFY_SUCCESS(err_code);

    if ((p_lowspp->conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_ch1_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > /*BLE_LOWSPP_CH1_MAX_CHAR_LEN*/m_ble_max_data_len)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_lowspp->ch1_handles.value_handle;
    hvx_params.p_data = p_data;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

	#if CONFIG_RTT_LOWSPP_ENABLED
    LOG_DRAW_Print(0, "snd1: ");
    for (uint16_t i = 0; i < length; i++)
    {
        LOG_DRAW_Print(0, "%.2x ", p_data[i]);
    }
    LOG_DRAW_Print(0, "\n\r");
    #endif

    return sd_ble_gatts_hvx(p_lowspp->conn_handle, &hvx_params);
}

static void lowspp_ch1_data_handler(ble_lowspp_evt_t * p_evt)
{
    switch(p_evt->type)
    {
        case BLE_LOWSPP_EVT_TX_RDY:
            LOWSAPP_Print("CH1 BLE_LOWSPP_EVT_TX_RDY\n\r");
            break;
        
        case BLE_LOWSPP_EVT_RX_DATA:
            LOWSAPP_Print("CH1 BLE_LOWSPP_EVT_RX_DATA\n\r");
            {
                uint8_t     *p_data = (uint8_t *)p_evt->params.rx_data.p_data;
                uint16_t    length  = p_evt->params.rx_data.length;

                length = (length > 20) ? (20) : (length);
                
                ble_lowspp_recv_handler(1, p_data, length);
                #if CONFIG_RTT_LOWSPP_ENABLED
                LOG_DRAW_Print(0, "rcv1: ");
                for (uint16_t i = 0; i < length; i++)
                {
                    LOG_DRAW_Print(0, "%.2x ", p_data[i]);
                }
                LOG_DRAW_Print(0, "\n\r");
                #endif
            }
            break;
        
        case BLE_LOWSPP_EVT_COMM_STARTED:
            LOWSAPP_Print("CH1 BLE_LOWSPP_EVT_COMM_STARTED\n\r");
            break;
        
        case BLE_LOWSPP_EVT_COMM_STOPPED:
            LOWSAPP_Print("CH1 BLE_LOWSPP_EVT_COMM_STOPPED\n\r");
            break;
    }
}
#endif

#if (BLE_LOWSPP_CH2_ENABLED == 1) 
static uint32_t ch2_char_add(ble_lowspp_t * p_lowspp)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_md_t cccd_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_lowspp->uuid_type;
    ble_uuid.uuid = BLE_UUID_LOWSPP_CH2_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_LOWSPP_CH2_MAX_CHAR_LEN;
    attr_char_value.p_value   = ch2_value_buf;

    return sd_ble_gatts_characteristic_add(p_lowspp->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_lowspp->ch2_handles);
}

uint32_t ble_lowspp_ch2_data_send(ble_lowspp_t * p_lowspp,
                           uint8_t   * p_data,
                           uint16_t  length)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    ble_lowspp_client_context_t * p_client;

    VERIFY_PARAM_NOT_NULL(p_lowspp);

    err_code = blcm_link_ctx_get(p_lowspp->p_link_ctx_storage, p_lowspp->conn_handle, (void *) &p_client);
    VERIFY_SUCCESS(err_code);

    if ((p_lowspp->conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_ch2_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > /*BLE_LOWSPP_CH2_MAX_CHAR_LEN*/m_ble_max_data_len)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_lowspp->ch2_handles.value_handle;
    hvx_params.p_data = p_data;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

	#if CONFIG_RTT_DEBUG_ENABLED
    SEGGER_RTT_printf(0, "snd2: ");
    for (uint16_t i = 0; i < length; i++)
    {
        SEGGER_RTT_printf(0, "%.2x ", p_data[i]);
    }
    SEGGER_RTT_printf(0, "\n\r");
    #endif

    return sd_ble_gatts_hvx(p_lowspp->conn_handle, &hvx_params);
}

static void lowspp_ch2_data_handler(ble_lowspp_evt_t * p_evt)
{
    switch(p_evt->type)
    {
        case BLE_LOWSPP_EVT_TX_RDY:
            LOWSAPP_Print("CH2 BLE_LOWSPP_EVT_TX_RDY\n\r");
            break;
        
        case BLE_LOWSPP_EVT_RX_DATA:
            LOWSAPP_Print("CH2 BLE_LOWSPP_EVT_RX_DATA\n\r");
            {
                uint8_t     *p_data = (uint8_t *)p_evt->params.rx_data.p_data;
                uint16_t    length  = p_evt->params.rx_data.length;

                length = (length > 20) ? (20) : (length);
                
                ble_lowspp_recv_handler(2, p_data, length);
                #if CONFIG_RTT_DEBUG_ENABLED
                SEGGER_RTT_printf(0, "rcv2: ");
                for (uint16_t i = 0; i < length; i++)
                {
                    SEGGER_RTT_printf(0, "%.2x ", p_data[i]);
                }
                SEGGER_RTT_printf(0, "\n\r");
                #endif
            }
            break;
        
        case BLE_LOWSPP_EVT_COMM_STARTED:
            LOWSAPP_Print("CH2 BLE_LOWSPP_EVT_COMM_STARTED\n\r");
            break;
        
        case BLE_LOWSPP_EVT_COMM_STOPPED:
            LOWSAPP_Print("CH2 BLE_LOWSPP_EVT_COMM_STOPPED\n\r");
            break;
    }
}
#endif

#if (BLE_LOWSPP_CH3_ENABLED == 1) 
static uint32_t ch3_char_add(ble_lowspp_t * p_lowspp)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_md_t cccd_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.char_props.write         = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.char_props.notify        = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = &cccd_md;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_lowspp->uuid_type;
    ble_uuid.uuid = BLE_UUID_LOWSPP_CH3_CHARACTERISTIC;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_LOWSPP_CH3_MAX_CHAR_LEN;
    attr_char_value.p_value   = ch3_value_buf;

    return sd_ble_gatts_characteristic_add(p_lowspp->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_lowspp->ch3_handles);
}

uint32_t ble_lowspp_ch3_data_send(ble_lowspp_t * p_lowspp,
                           uint8_t   * p_data,
                           uint16_t  length)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    ble_lowspp_client_context_t * p_client;

    VERIFY_PARAM_NOT_NULL(p_lowspp);

    err_code = blcm_link_ctx_get(p_lowspp->p_link_ctx_storage, p_lowspp->conn_handle, (void *) &p_client);
    VERIFY_SUCCESS(err_code);

    if ((p_lowspp->conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (!p_client->is_ch3_notification_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (length > /*BLE_LOWSPP_CH3_MAX_CHAR_LEN*/m_ble_max_data_len)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_lowspp->ch3_handles.value_handle;
    hvx_params.p_data = p_data;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(p_lowspp->conn_handle, &hvx_params);
}

static void lowspp_ch3_data_handler(ble_lowspp_evt_t * p_evt)
{
    switch(p_evt->type)
    {
        case BLE_LOWSPP_EVT_TX_RDY:
            LOWSAPP_Print("CH3 BLE_LOWSPP_EVT_TX_RDY\n\r");
            break;
        
        case BLE_LOWSPP_EVT_RX_DATA:
        #if 1
            LOWSAPP_Print("CH3 BLE_LOWSPP_EVT_RX_DATA\n\r");
        {
            uint8_t     *p_data = (uint8_t *)p_evt->params.rx_data.p_data;
            uint16_t    length  = p_evt->params.rx_data.length;
            ble_lowspp_ch3_data_send(p_evt->p_lowspp, p_data, length);
            
//            if (strncmp((const char *)p_data, "BMP280", 6) == 0)
//            {
//                app_sched_event_put(NULL, NULL, bmp280_event);
//            }
//            
//            if (strncmp((const char *)p_data, "BM43", 4) == 0)
//            {
//                app_sched_event_put(NULL, NULL, bm43_event);
//            }
//            
//            if (strncmp((const char *)p_data, "SI7051", 6) == 0)
//            {
//                app_sched_event_put(NULL, NULL, si705x_event);
//            }

//            if (strncmp((const char *)p_data, "W25Q80", 6) == 0)
//            {
//                app_sched_event_put(NULL, NULL, w25q80_event);
//            }
//            
//            if (strncmp((const char *)p_data, "MC3610", 6) == 0)
//            {
//                app_sched_event_put(NULL, NULL, mc36xx_event);
//            }
//            
//            if (strncmp((const char *)p_data, "BMD101", 6) == 0)
//            {
//                app_sched_event_put(&p_data[6], 1, bm101_event);
//            }
//			
//			if (strncmp((const char *)p_data, "MIC", 3) == 0)
//			{
//				nrfx_err_t nrfx_pdm_start(void);
//				nrfx_pdm_start();
//			}
			
//			if (strncmp((const char *)p_data, "BT", 2) == 0)
//			{
//				ble_lowspp_ch3_data_send(p_evt->p_lowspp, test_buf, m_ble_max_data_len);
//			}
			
//			if (strncmp((const char *)p_data, "DIR", 3) == 0)
//			{
//				void flash_body_read_test(void);
//				flash_body_read_test();
//			}
        }
		#else

		LOWSAPP_Print("CH1 BLE_LOWSPP_EVT_RX_DATA\n\r");
        {
            uint8_t     *p_data = (uint8_t *)p_evt->params.rx_data.p_data;
            uint16_t    length  = p_evt->params.rx_data.length;

            length = (length > 20) ? (20) : (length);
            
            ble_lowspp_recv_handler(3, p_data, length);
            #ifdef PDEBUG
            SEGGER_RTT_printf(0, "rcv3: ");
            for (uint16_t i = 0; i < length; i++)
            {
                SEGGER_RTT_printf(0, "%.2x ", p_data[i]);
            }
            SEGGER_RTT_printf(0, "\n\r");
            #endif
        }

		#endif
        
            break;
        
        case BLE_LOWSPP_EVT_COMM_STARTED:
            LOWSAPP_Print("CH3 BLE_LOWSPP_EVT_COMM_STARTED\n\r");
            break;
        
        case BLE_LOWSPP_EVT_COMM_STOPPED:
            LOWSAPP_Print("CH3 BLE_LOWSPP_EVT_COMM_STOPPED\n\r");
            break;
    }
}
#endif

void ble_lowspp_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_lowspp_t * p_lowspp = (ble_lowspp_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        	pack_fifo_clear(&m_send_pack_fifo);
            m_send_busy = false;
            on_connect(p_lowspp, p_ble_evt);
            break;
        
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_lowspp, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_lowspp, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
//            LOWSAPP_Print("BLE_GATTS_EVT_HVN_TX_COMPLETE\n\r");
            m_send_busy = false;
            ble_lowspp_send_handler();	// 发射
            break;

        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_lowspp_init(ble_lowspp_t * p_lowspp, ble_lowspp_init_t const * p_lowspp_init)
{
    ret_code_t    err_code;
    ble_uuid_t    ble_uuid;
    ble_uuid128_t lowspp_base_uuid = LOWSPP_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_lowspp);
    VERIFY_PARAM_NOT_NULL(p_lowspp_init);

    // Initialize the service structure.
    p_lowspp->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    #if (BLE_LOWSPP_CH1_ENABLED == 1)
    p_lowspp->ch1_data_handler = p_lowspp_init->ch1_data_handler;
    #endif
    
    #if (BLE_LOWSPP_CH2_ENABLED == 1)
    p_lowspp->ch2_data_handler = p_lowspp_init->ch2_data_handler;
    #endif
    
    #if (BLE_LOWSPP_CH3_ENABLED == 1)
    p_lowspp->ch3_data_handler = p_lowspp_init->ch3_data_handler;
    #endif

    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&lowspp_base_uuid, &p_lowspp->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_lowspp->uuid_type;
    ble_uuid.uuid = BLE_UUID_LOWSPP_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_lowspp->service_handle);
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    VERIFY_SUCCESS(err_code);

    #if (BLE_LOWSPP_CH1_ENABLED == 1)
    // Add the RX Characteristic.
    err_code = ch1_char_add(p_lowspp);
    VERIFY_SUCCESS(err_code);
    #endif

    #if (BLE_LOWSPP_CH2_ENABLED == 1)
    // Add the TX Characteristic.
    err_code = ch2_char_add(p_lowspp);
    VERIFY_SUCCESS(err_code);
    #endif
    
    #if (BLE_LOWSPP_CH3_ENABLED == 1)
    // Add the TX Characteristic.
    err_code = ch3_char_add(p_lowspp);
    VERIFY_SUCCESS(err_code);
    #endif

    return NRF_SUCCESS;
}


void ble_lowspp_services_init(void)    
{
    uint32_t                err_code;
    ble_lowspp_init_t       lowspp_init;
	
	// 仅做测试
//	for (uint16_t i = 0; i < 256; i++)
//		test_buf[i] = i;

    // Initialize NUS.
    memset(&lowspp_init, 0, sizeof(lowspp_init));
 
    #if (BLE_LOWSPP_CH1_ENABLED == 1)
    lowspp_init.ch1_data_handler = lowspp_ch1_data_handler;
    #endif
    
    #if (BLE_LOWSPP_CH2_ENABLED == 1)
    lowspp_init.ch2_data_handler = lowspp_ch2_data_handler;
    #endif
    
    #if (BLE_LOWSPP_CH3_ENABLED == 1)
    lowspp_init.ch3_data_handler = lowspp_ch3_data_handler;
    #endif

    err_code = ble_lowspp_init(&m_lowspp, &lowspp_init);
    APP_ERROR_CHECK(err_code);


    err_code = app_fifo_init(&m_recv_fifo, recv_buffer, M_COMS_BLE_LOWSPP_RECV_BUFFER_SIZE_MAX);
    APP_ERROR_CHECK(err_code);
    
    err_code = pack_fifo_init(&m_send_pack_fifo, send_pack_buffer, M_COMS_BLE_LOWSPP_SEND_BUFFER_PACK_COUNT_MAX);
	APP_ERROR_CHECK(err_code);
}

//************************************************************************************************************
// 功能描述:   	处理(解析)接收到的数据
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
void rx_sched_event_handler(void *p_context)
{
    m_protocol_recv_process();
}


//************************************************************************************************************
// 功能描述:   	接收到的数据放到FIFO，再处理
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
__STATIC_INLINE void ble_lowspp_recv_handler(uint8_t chn, uint8_t * p_data, uint16_t length)
{
    //ASSERT((chn == 1) || (chn == 3));
    ASSERT(p_data);
    ASSERT(length);

    uint32_t        err_code;
    uint32_t        write_length = length;

    err_code = app_fifo_write(&m_recv_fifo, p_data, &write_length);

    if (err_code == NRF_SUCCESS)
    {
        if (write_length != length) // 剩余空间不足以保存新接收的数据时，也报没内存事件，此后的数据都将丢失
        {
        	//出错
        	LOWSAPP_Print("fifo write err: %d-%d\n\r", write_length, length);
        }
	}

	APP_ERROR_CHECK(app_isched_event_put(&g_fg_scheduler, rx_sched_event_handler, NULL));
}

//************************************************************************************************************
// 功能描述:   	从发送FIFO读取数据，命令从特征值1发送，数据从特征值2发送
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
static void ble_lowspp_send_handler(void)
{
//	  LOWSAPP_Print("ble_lowspp_send_handler\r\n");
//	if ((PACK_FIFO_LENGTH() == 0) || m_send_busy)
	if (m_send_busy)
	{
		return;
	}

	uint32_t	err_code;
	pack_t		pack = {0};

	while (true)
	{

		err_code = pack_fifo_peek(&m_send_pack_fifo, 0,&pack); // 取出包，不更新读指针

		if (err_code == NRF_SUCCESS)
		{
			if (pack.channel == SEND_CHANNEL_CMD)
			{
//				   LOWSAPP_Print("ble_lowspp_ch1_string_send\r\n");
#if 0
				LOG_DRAW_Print("ch1 send data: ");
				if( ! ((pack.pack_data[5] == 0x08)&&(pack.pack_data[5] == 0x10)&&(pack.pack_data[5] == 0x03)) )
				{
					for (uint8_t i = 0; i < 20; i++)
					{
						LOG_DRAW_Print("%02x ", pack.pack_data[i]);
						if ((i == 4) || (i == 7))
						{
							LOG_DRAW_Print("| ");
						}
					}
					LOG_DRAW_Print("\r\n");
				}
#endif
				err_code = ble_lowspp_ch1_data_send(&m_lowspp, pack.pack_data, pack.pack_len);
//                LOWSAPP_Print("ble_lowspp_ch1_data_send err_code : %d\r\n",err_code);
			}
			else if (pack.channel == SEND_CHANNEL_DATA)
			{
//				  LOWSAPP_Print("ble_lowspp_ch2_string_send\r\n");
#if 0
				LOG_DRAW_Print("ch2 send data: ");
				for (uint8_t i = 0; i < 20; i++)
				{
					LOG_DRAW_Print("%02x ", pack.pack_data[i]);
					if ((i == 4) || (i == 7))
					{
						LOG_DRAW_Print("| ");
					}
				}
				LOG_DRAW_Print("\r\n");
#endif
				err_code = ble_lowspp_ch2_data_send(&m_lowspp, pack.pack_data, pack.pack_len);
			}	   

			if (err_code == NRF_SUCCESS)
			{
				pack_fifo_get(&m_send_pack_fifo, &pack); // 更新读指针
			}
			else if ( (err_code == NRF_ERROR_BUSY) || (err_code == NRF_ERROR_RESOURCES) )
			{
				LOWSAPP_Print("Send hanlder: Soft device full!!!!!!!!!!!!!!!!!");
				m_send_busy = true;
                break;
			}
			else
			{
				LOWSAPP_Print("Send hanlder: error 0x%02x", err_code);
				m_send_busy = false;
				break;
			}
		}
		if (err_code != NRF_SUCCESS)
		{
			break;
		}
	}
}
//************************************************************************************************************
// 功能描述:   	把要发送的数据放入FIFO再发送
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
uint32_t m_coms_ble_lowspp_write(send_channel_t channel, uint8_t * p_data, uint16_t length)
{
	ASSERT(p_data);
	ASSERT(length);

	uint32_t	err_code;
	pack_t		pack = {0};

	if (!((channel > SEND_CHANNEL_FIRST) && (channel < SEND_CHANNEL_LAST)))
	{
		return NRF_ERROR_INVALID_PARAM;
	}

	if (m_lowspp.conn_handle == BLE_CONN_HANDLE_INVALID)
	{
		return NRF_ERROR_INVALID_STATE;
	}

	if ((channel == SEND_CHANNEL_CMD) || (channel == SEND_CHANNEL_DATA))
	{
		pack.channel = channel;

		if (length <= M_COMS_BLE_LOWSPP_PACK_MAX_SIZE)
		{
			pack.pack_len = length;
			memcpy(pack.pack_data, p_data, length);
			err_code = pack_fifo_put(&m_send_pack_fifo, pack);
		}
		else
		{
			err_code = NRF_ERROR_INVALID_LENGTH;
		}
	}
//	  LOWSAPP_Print("m_send_busy :%d  , err_code :%d ",m_send_busy,err_code);
	if ((!m_send_busy) && (err_code == NRF_SUCCESS))
	{
		ble_lowspp_send_handler();
	}
	else
	{
		if (err_code == NRF_ERROR_NO_MEM)
		{
			pack_fifo_clear(&m_send_pack_fifo);
			LOWSAPP_Print("Fifo full and cleared!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		}
	}

	return err_code;
}

//************************************************************************************************************
// 功能描述:   	从接收FIFO读取1个字节
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
uint32_t m_coms_ble_get_byte(uint8_t * p_byte)
{
	ASSERT(p_byte);
	
	return app_fifo_get(&m_recv_fifo, p_byte);
}

//************************************************************************************************************
// 功能描述:   	发送命令
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
uint32_t m_coms_ble_put_cmd(uint8_t * p_data, uint16_t length)
{
    return m_coms_ble_lowspp_write(SEND_CHANNEL_CMD, p_data, length);
}

//************************************************************************************************************
// 功能描述:   	发送数据
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
uint32_t m_coms_ble_put_data(uint8_t * p_data, uint16_t length)
{
    return m_coms_ble_lowspp_write(SEND_CHANNEL_DATA, p_data, length);
}



#endif // NRF_MODULE_ENABLED(BLE_NUS)

