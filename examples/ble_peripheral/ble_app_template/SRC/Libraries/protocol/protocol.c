

#include "sdk_common.h"
#include "protocol_dox_config.h"

#include "nrf_assert.h"
#include "protocol.h"

#include "app_debug.h"

#include "includes.h"


#if CONFIG_RTT_PORTO_ENABLED
#define PROTO_Print(M, ...)    SEGGER_Print("PROTO", M, ##__VA_ARGS__)
#else
#define PROTO_Print(M, ...)
#endif

#define FRAME_HEAD                      0x23
#define FRAME_VERSION                   0x01
#define FRAME_ROUTE_SEND                0x13
#define FRAME_ROUTE_RECV                0x31

#pragma pack(1)

typedef struct
{
    uint8_t head;
    uint8_t version;
    uint8_t payload_len;
    uint8_t routing;
    uint8_t flow_ctrl;
}frame_hdr_t;

typedef struct
{
    uint8_t intf_type; // @intf_type_t
    uint8_t intf_1;
    uint8_t intf_2;
}frame_intf_t;

/*
 * 接口帧格式
 */
typedef struct
{
    frame_hdr_t             header;
    frame_intf_t            intf;
    uint8_t                 p_and_c[]; // params and checksum
}prot_intf_frame_t;

/*
 * 通用 ACK 帧格式
 */
typedef struct
{
    frame_hdr_t             header;
    uint8_t                 state;
    uint8_t                 checksum;
}prot_ack_frame_t;

typedef enum
{
    FRAME_TYPE_FIRST            = 0U,
    FRAME_TYPE_CMD,
    FRAME_TYPE_ACK,
}frame_type_t;

typedef struct
{
    frame_type_t            frame_type;
    union
    {
        prot_intf_frame_t   intf_frame;
        prot_ack_frame_t    ack_frame;
        uint8_t             data[FRAME_SIZE_MAX];
    }frame_data;
}protocol_frame_t;

#pragma pack()

typedef enum
{
    RECV_OP_GET_FRAME,
    RECV_OP_GET_EVT,
    RECV_OP_EVT_DISPATCH,
    RECV_OP_ACK_PROCESS,
}recv_operation_t;

typedef recv_operation_t (*recv_op_step_t)(void);

typedef struct
{
    frame_intf_t    intf;
    prot_evt_id_t   evt_id;
}intf_evtid_map_t;

recv_operation_t recv_op_get_frame(void);
recv_operation_t recv_op_get_evt(void);
recv_operation_t recv_op_evt_dispatch(void);
recv_operation_t recv_op_ack_process(void);

#define RECV_OPERATION_DEFINES {    \
    recv_op_get_frame,              \
    recv_op_get_evt,                \
    recv_op_evt_dispatch,           \
    recv_op_ack_process             \
}

static const recv_op_step_t         recv_operations[] = RECV_OPERATION_DEFINES;
static const intf_evtid_map_t       intf_evtid_maps[] = INTERFACE_AND_EVENT_ID_MAP_DEFINES;
static protocol_frame_t             m_recv_frame;
#pragma pack(1)
static protocol_evt_t               m_prot_evt;
#pragma pack()
static protocol_recv_handler_t      m_recv_handler;
static protocol_send_handler_t      m_send_handler;

static uint8_t checksum(uint8_t *buf, uint8_t buf_size)
{
    if ((buf == NULL) || (buf_size == 0))
    {
        return 0;
    }

    uint8_t i;
    uint8_t ret = 0;

    for (i = 0; i < buf_size; i++)
    {
        ret += buf[i] ^ i;
    }

    return ret;
}

static uint32_t get_intf_from_evtid(uint16_t evt_id, frame_intf_t *intf)
{
    if (intf == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    uint16_t i;
    bool     is_evt_id_valid = false;

    for (i = 0; i < ARRAY_SIZE(intf_evtid_maps); i++)
    {
        if (evt_id == intf_evtid_maps[i].evt_id)
        {
            *intf = intf_evtid_maps[i].intf;
            is_evt_id_valid = true;
        }
    }

    if (is_evt_id_valid)
    {
        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_NOT_FOUND;
    }
}

recv_operation_t recv_op_get_frame(void)
{
    bool    is_valid_frame = false;
    uint8_t data = 0;
    uint8_t frame_index = 0;
    uint8_t *recv_frame = m_recv_frame.frame_data.data;
    uint8_t payload_len = 0;

    memset(recv_frame, 0, FRAME_SIZE_MAX);

    while (NRF_SUCCESS == m_recv_handler.input_func(&data))
    {
        if (frame_index == 0)
        {
            if (data == FRAME_HEAD)
            {
                recv_frame[frame_index++] = data;
                
            }

            continue;
        }

        recv_frame[frame_index++] = data;

        payload_len = m_recv_frame.frame_data.intf_frame.header.payload_len;

        if ((payload_len + 6) == frame_index)
        {
            if (1)//checksum(recv_frame, frame_index-1) == recv_frame[frame_index-1])
            {
                if (payload_len == 1)
                {
                    m_recv_frame.frame_type = FRAME_TYPE_ACK;
                }
                else if (payload_len >= 3)
                {
                    m_recv_frame.frame_type = FRAME_TYPE_CMD;
                }
#if CONFIG_RTT_PORTO_ENABLED
                LOG_DRAW_Print("Recv op get frame: ");
                for (uint8_t i = 0; i < frame_index; i++)
                {
                    LOG_DRAW_Print("%02x ", m_recv_frame.frame_data.data[i]);
                    if ((i == 4) || (i == 7) || (i == (frame_index - 2)))
                    {
                        LOG_DRAW_Print("| ");
                    }
                }
                LOG_DRAW_Print("\r\n");
#endif
                is_valid_frame = true;
                break;
            }
            else
            {
                PROTO_Print("Recv op get frame: Checksum error!!!!!!!!!!!!!!\r\n");
            }
        }
    }

    if (is_valid_frame)
    {
        return RECV_OP_GET_EVT;
    }
    else
    {
        return RECV_OP_GET_FRAME;
    }
}

recv_operation_t recv_op_get_evt(void)
{
    if (m_recv_frame.frame_type == FRAME_TYPE_ACK)
    {
        m_prot_evt.header.evt_id = PROT_EVT_ID_ACK;
        m_prot_evt.header.evt_len = 0;
        return RECV_OP_EVT_DISPATCH;
    }

    bool         is_valid_evt = false;
    uint8_t      evt_len = 0;
    frame_intf_t intf = m_recv_frame.frame_data.intf_frame.intf;

    for (uint16_t i = 0; i < ARRAY_SIZE(intf_evtid_maps); i++)
    {
        if (  (intf.intf_type == intf_evtid_maps[i].intf.intf_type)
           && (intf.intf_1    == intf_evtid_maps[i].intf.intf_1)
           && (intf.intf_2    == intf_evtid_maps[i].intf.intf_2) )
        {
            m_prot_evt.header.evt_id  = intf_evtid_maps[i].evt_id;

            evt_len = m_recv_frame.frame_data.intf_frame.header.payload_len - 3;
            m_prot_evt.header.evt_len = evt_len;
            PROTO_Print("evt_len: %d\r\n",evt_len);

            if (evt_len)
            {
                memcpy(m_prot_evt.evt_params.data, m_recv_frame.frame_data.intf_frame.p_and_c, evt_len);
            }

            is_valid_evt = true;
            break;
        }
    }

    if (is_valid_evt)
    {
        return RECV_OP_EVT_DISPATCH;
    }
    else
    {
        return RECV_OP_GET_FRAME;
    }
}

recv_operation_t recv_op_evt_dispatch(void)
{
    if (PROT_EVT_ID_ACK == m_prot_evt.header.evt_id)
    {
        return RECV_OP_GET_FRAME;
    }

    if (m_recv_handler.output_func)
    {
        m_recv_handler.output_func(&m_prot_evt); // m_prot_evt 作为输入、输出参数

        if (m_prot_evt.header.evt_id != PROT_EVT_ID_INVALID)
        {
            return RECV_OP_ACK_PROCESS;
        }
    }

    return RECV_OP_GET_FRAME;

}

recv_operation_t recv_op_ack_process(void)
{
    if (m_prot_evt.header.evt_id == PROT_EVT_ID_INVALID)
    {
        return RECV_OP_GET_FRAME;
    }

    protocol_send_evt(m_prot_evt);

    return RECV_OP_GET_FRAME;
}

uint16_t change_endian_16(uint16_t val)
{
    bytes_t val_in;
    bytes_t val_out;

    val_in.short_val = val;
    val_out.bytes[0] = val_in.bytes[1];
    val_out.bytes[1] = val_in.bytes[0];

    return val_out.short_val;
}

uint32_t change_endian_32(uint32_t val)
{
    bytes_t val_in;
    bytes_t val_out;

    val_in.word_val  = val;
    val_out.bytes[0] = val_in.bytes[3];
    val_out.bytes[1] = val_in.bytes[2];
    val_out.bytes[2] = val_in.bytes[1];
    val_out.bytes[3] = val_in.bytes[0];

    return val_out.word_val;
}


void protocol_recv_process(void)
{
    static recv_operation_t next_op = RECV_OP_GET_FRAME;
    recv_operation_t        cur_op;
    uint8_t                 loop_cnt = 0;
    uint32_t                err_code = NRF_SUCCESS;

    do
    {
        cur_op = next_op;
        next_op = recv_operations[cur_op]();
        loop_cnt++;
        if (loop_cnt > 24)
        {
            err_code = NRF_ERROR_TIMEOUT;
            break;
        }
    }while (cur_op != next_op);

    APP_ERROR_CHECK(err_code);
}

uint32_t protocol_send_data(uint8_t * p_data_buf, uint8_t data_len)
{
    if (p_data_buf == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
   
    return m_send_handler.data_channel_func(p_data_buf, data_len);  //m_coms_ble_put_data();
}


void protocol_send_evt(protocol_evt_t evt)
{
    static uint8_t  flow_cnt = 0;
    uint8_t         send_buf[FRAME_SIZE_MAX]= {0};
    uint8_t         buf_len = 0;

    if (evt.header.evt_id == PROT_EVT_ID_INVALID)
    {
        return;
    }

    if (evt.header.evt_id == PROT_EVT_ID_ACK)
    {
        send_buf[buf_len++] = FRAME_HEAD;
        send_buf[buf_len++] = FRAME_VERSION;
        send_buf[buf_len++] = 1;
        send_buf[buf_len++] = FRAME_ROUTE_SEND;
        send_buf[buf_len++] = m_recv_frame.frame_data.intf_frame.header.flow_ctrl;
        send_buf[buf_len++] = evt.evt_params.ack.state;
        send_buf[buf_len++] = checksum(send_buf, buf_len);
    }
    else
    {
        frame_intf_t intf;
        uint8_t      flow = 0;

        if (NRF_SUCCESS != get_intf_from_evtid(evt.header.evt_id, &intf))
        {
            return;
        }

        if ((intf.intf_type == PROT_SET) || (intf.intf_type == PROT_GET) || (intf.intf_type == PROT_CALL))
        {
            flow = flow_cnt++;
        }
        else if (intf.intf_type == PROT_RET)
        {
            flow = m_recv_frame.frame_data.intf_frame.header.flow_ctrl;
        }

        send_buf[buf_len++] = FRAME_HEAD;
        send_buf[buf_len++] = FRAME_VERSION;
        send_buf[buf_len++] = evt.header.evt_len + 3;
        send_buf[buf_len++] = FRAME_ROUTE_SEND;
        send_buf[buf_len++] = flow;
        send_buf[buf_len++] = intf.intf_type;
        send_buf[buf_len++] = intf.intf_1;
        send_buf[buf_len++] = intf.intf_2;

        if (evt.header.evt_len)
        {
            memcpy(&send_buf[buf_len], evt.evt_params.data, evt.header.evt_len);
            buf_len += evt.header.evt_len;
        }

        send_buf[buf_len++] = checksum(send_buf, buf_len);
    }

#if 0
    NRF_LOG_RAW_INFO("Protocol send: ");
    NRF_LOG_RAW_HEXDUMP_INFO(send_buf, buf_len);
#endif
    uint32_t err_code = m_send_handler.cmd_channel_func(send_buf, buf_len);

    if (err_code != NRF_SUCCESS)
    {
        PROTO_Print("protocol: Send evt error %d", err_code);
    }
}

void protocol_init(protocol_recv_handler_t recv_handler, protocol_send_handler_t send_handler)
{
    ASSERT(recv_handler.input_func);
    ASSERT(recv_handler.output_func);
    ASSERT(send_handler.cmd_channel_func);
    ASSERT(send_handler.data_channel_func);   

    m_recv_handler.input_func = recv_handler.input_func;
    m_recv_handler.output_func = recv_handler.output_func;
     
    m_send_handler.cmd_channel_func = send_handler.cmd_channel_func;
    m_send_handler.data_channel_func = send_handler.data_channel_func;
}



