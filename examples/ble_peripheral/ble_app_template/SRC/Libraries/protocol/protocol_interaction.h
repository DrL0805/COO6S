
#ifndef PROTOCOL_INTERACTION_H__
#define PROTOCOL_INTERACTION_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef struct 
{
    uint32_t msg_bitmap;
}prot_new_msg_evt_t;

typedef struct 
{
    uint32_t msg_switch_bitmap;
}prot_msg_switch_evt_t;

typedef struct 
{
    uint8_t cmd;                // 0, 拒接来电; 1, 接听来电
}prot_incall_resp_evt_t;

typedef union 
{
    uint32_t msg_bitmap;
}prot_cancel_msg_evt_t;

typedef union 
{
    uint8_t nothing;
}prot_find_phone_evt_t;

typedef struct 
{
    uint8_t call_enter_exit;    // 1，进入拍照; 0，退出拍照
}prot_photo_mode_evt_t;

typedef union 
{
    uint8_t ret_result;         // 拍照请求的结果: 1，拍照异常; 0，成功拍照
}prot_photo_req_evt_t;

typedef union 
{
    uint8_t nothing;
}prot_photo_heartbeat_evt_t;

typedef struct
{
    uint8_t enter_exit;         // 1，进入拍照; 0，退出拍照
}prot_wh_mode_evt_t;

typedef struct
{
    uint8_t watch_hand_id;      // 1~255  BYTE  指针 ID，每个指针 ID 对应一种类型的指针
    uint8_t cmd;                // 0 恢复行针, 1 暂停行针, 2 指针正转 1 步, 3 指针反转 1 步, 4 使指针进入校针状态
}prot_wh_ctrl_evt_t;

typedef struct
{
    uint8_t  watch_hand_id;      // 1~255  BYTE  指针 ID，每个指针 ID 对应一种类型的指针
    uint16_t watch_hand_pos;     // 指针物理位置
}prot_wh_param_evt_t;

typedef union
{
    uint8_t nothing;
}prot_wh_heartbeat_evt_t;

typedef union
{
    uint8_t remind_enabled;
}prot_long_sit_switch_evt_t;

typedef struct
{
    hour_minute_t   valid_time_begin;               // ???ё?Ч??ì????
    hour_minute_t   valid_time_end;                 // ???ё?Ч??ì????
    hour_minute_t   dnd_time_begin;                 // ???ё????ì????
    hour_minute_t   dnd_time_end;                   // ???ё????ì????
    uint16_t        intv;                           // ???ё??
}prot_long_sit_param_evt_t;

typedef union
{
    uint8_t remind_enabled;
}prot_dnd_switch_evt_t;

typedef struct
{
    hour_minute_t   begin_time;             // ??????
    hour_minute_t   end_time;               // ??????
}prot_dnd_param_evt_t;




typedef struct
{
    uint16_t expect_seconds;
}prot_calibrate_expect_seconds_evt_t;

typedef enum
{
    START_CALIBRATE_TIME = 0,
    COMPLETE_CALIBRATE_TIME,
}calibrate_time_state_t;


typedef struct
{
    calibrate_time_state_t calibrate_time_state;
}prot_calibrate_time_state_evt_t;

typedef struct
{
    uint8_t callin_scale;
    uint8_t msg_scale;
    uint8_t alarm_scale;
    uint8_t long_sit_scale;
    uint8_t step_goal_finish_scale;
}prot_vibration_scale_evt_t;

typedef struct
{
    uint8_t  vib_times;
    uint8_t  vib_freq;
    uint8_t  led_times;
    uint8_t  led_freq;
}prot_spec_event_remind_evt_t;


typedef struct
{
    uint8_t green_mode;
}prot_green_mode_evt_t;

typedef struct
{
    uint32_t settings;
}prot_function_settings_evt_t;



typedef union
{
    prot_new_msg_evt_t          new_msg;
    prot_msg_switch_evt_t       msg_switch;
    prot_incall_resp_evt_t      incall_resp;
    prot_cancel_msg_evt_t       cancel_msg;
    prot_find_phone_evt_t       find_phone;
    prot_photo_mode_evt_t       photo_mode;
    prot_photo_req_evt_t        photo_req;
    prot_photo_heartbeat_evt_t  photo_heartbeat;
    prot_wh_mode_evt_t          watch_hand_mode;
    prot_wh_ctrl_evt_t          watch_hand_ctrl;
    prot_wh_param_evt_t         watch_hand_param;
    prot_calibrate_time_state_evt_t        calibrate_time_state;
    prot_calibrate_expect_seconds_evt_t    calibrate_expect_seconds;
    prot_wh_heartbeat_evt_t     watch_hand_heartbeat;
    prot_long_sit_switch_evt_t  long_sit_switch;
    prot_long_sit_param_evt_t   long_sit_param;
    prot_dnd_switch_evt_t       dnd_switch;
    prot_dnd_param_evt_t        dnd_param;
    prot_vibration_scale_evt_t  vibration_scale;
    prot_spec_event_remind_evt_t spec_event_remind;
    prot_green_mode_evt_t         prot_green_mode;
    prot_function_settings_evt_t  function_settings;
    
}prot_interact_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_INTERACTION_H__ */


