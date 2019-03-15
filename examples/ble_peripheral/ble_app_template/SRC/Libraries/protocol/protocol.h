
#ifndef PROTOCOL_H__
#define PROTOCOL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    
#include "protocol_common.h"
#include "protocol_link.h"
#include "protocol_dev_info.h"
#include "protocol_scene_daily_sport.h"
#include "protocol_interaction.h"
#include "protocol_pack_trans.h"
#include "protocol_sensor_data.h"
#include "protocol_scene_sleep_record.h"
#include "protocol_test_mode.h"
#define FRAME_SIZE_MAX                  (20)

#pragma pack(1)

typedef enum
{
    ACK_STATE_SUCCESS           = 0,
    ACK_STATE_ERROR_INTF        = 1,
    ACK_STATE_ERROR_INTF_TYPE   = 2,
    ACK_STATE_ERROR_INTF_PARAM  = 3,
    ACK_STATE_ERROR_CHECKSUM    = 4,
    ACK_STATE_INVALID,
}ack_state_t;

typedef struct
{
    uint8_t                             state;                                  // @ ack_state_t
}prot_ack_evt_t;

typedef struct
{
    uint16_t                            evt_id;                                 // 事件 ID, 每个接口对应一个事件 ID，@prot_evt_id_t
    uint16_t                            evt_len;                                // 事件长度 = 事件参数的长度 = 接口帧中:载荷的长度 - 3
}prot_evt_hdr_t;

typedef struct
{
    prot_evt_hdr_t                      header;
    union
    {
        prot_ack_evt_t                  ack;
        prot_link_evt_t                 link;
        prot_dev_info_evt_t             dev_info;
        prot_scene_daily_sport_evt_t    scene_daily_sport;
        prot_interact_evt_t             interact;
        prot_pack_trans_evt_t           pack_trans;
        prot_scene_sleep_record_evt_t   sleep_record;
        prot_test_mode_evt_t            test_mode;
		prot_sensor_data_evt_t          sensor_data;
        uint8_t                         data[FRAME_SIZE_MAX - 9];
    }evt_params;
}protocol_evt_t;

#pragma pack()

typedef uint32_t (*protocol_input_handler_t)(uint8_t *);

typedef void (*protocol_output_handler_t)(protocol_evt_t *);

typedef uint32_t (*protocol_send_channel_handler_t)(uint8_t *, uint16_t);

typedef struct
{
    protocol_input_handler_t   input_func;
    protocol_output_handler_t  output_func;
}protocol_recv_handler_t;

typedef struct
{
    protocol_send_channel_handler_t     cmd_channel_func;
    protocol_send_channel_handler_t     data_channel_func;
}protocol_send_handler_t;

void protocol_init(protocol_recv_handler_t recv_handler, protocol_send_handler_t send_handler);

void protocol_send_evt(protocol_evt_t evt);

uint32_t protocol_send_data(uint8_t * p_data_buf, uint8_t data_len);

void protocol_recv_process(void);


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_H__ */


