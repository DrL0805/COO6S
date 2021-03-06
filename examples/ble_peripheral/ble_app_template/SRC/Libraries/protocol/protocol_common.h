
#ifndef PROTOCOL_COMMON_H__
#define PROTOCOL_COMMON_H__

#include <stdint.h>


#define BCD_TO_DEC(bcd) ( ((bcd) >> 4) * 10 + ((bcd) & 0x0f) )
#define DEC_TO_BCD(dec) ( (((dec) / 10) << 4) + (((dec) % 10) & 0x0f) )

typedef struct
{
    uint8_t hour;
    uint8_t minute;
}hour_minute_t;

typedef union
{
    uint16_t short_val;
    uint32_t word_val;
    uint8_t  bytes[4];
}bytes_t;

uint16_t change_endian_16(uint16_t val);

uint32_t change_endian_32(uint32_t val);


/**
 * @file
 * @brief 定义接口(包括接口类型、接口1、接口2)和事件标识符的功能，及他们之间的映射
 */

#define PROT_SET                                        0x01
#define PROT_GET                                        0x02
#define PROT_CALL                                       0x04
#define PROT_RET                                        0x08
#define PROT_ECHO                                       0x80

#define PROT_LINK                                       0x00     /**< 链路信息 */
#define PROT_BT_STATE                                   0x01
#define PROT_BT_CONN_PARAM                              0x02
#define PROT_BT_ADV_NAME                                0x03
#define PROT_BT_ADV_PROJ_NUM                            0x04
#define PROT_BT_CONN_INTV                               0x05
#define PROT_BT_FW_VERSION                              0x06
#define PROT_IOS_PAIR_REQ                               0x07
#define PROT_LINKLOSS_REMIND_SWITCH                     0x11

#define PROT_DEV_INFO                                   0x01     /**< 产品信息 */
#define PROT_PROD_NAME                                  0x01
#define PROT_PROD_VER                                   0x02
#define PROT_PROD_DATE                                  0x03
#define PROT_PROD_SOC                                   0x04
#define PROT_PROD_AUTH                                  0x05
#define PROT_PROD_SN                                    0x06
#define PROT_PROD_MANAGER_SN                            0x0F
#define PROT_PROD_MAC                                   0x11
#define PROT_PROD_PRO_NUM                               0x12
#define PROT_PROD_SOC_NEW                               0x14
#define PROT_PROD_BATT_VOL                              0x15
#define PROT_PROD_RESET_INFO                            0x20
#define PROT_DATE                                       0x08
#define PROT_SEC_CITY_DAT                               0x09
#define PROT_ALARM_CLK                                  0x0A

#define PROT_PROD_RESET_INFO                            0x20
#define PROT_PROD_KEY_RESET_INFO                        0x21
#define PROT_PROD_ERR_RESET_INFO                        0x22
#define PROT_PROD_HARD_RESET_INFO                       0x23
#define PROT_PROD_DF_RESET_INFO                         0x24



#define PROT_DAILY_SPORT_SCENE                          0x10     /**< 日常运动场景 */
#define PROT_SCENE_DS_MODE                              0x01
#define PROT_SCENE_DS_GOAL                              0x02
#define PROT_SCENE_DS_TOTAL_DAILY_STEPS                 0x03
#define PROT_SCENE_DS_HIS_STEPS                         0x04
#define PROT_SCENE_DS_HIS_STEPS_TRANS                   0x05


#define PROT_SLEEP_RECORD_SCENE                         0x14     /**< sleep record 场景 */
#define PROT_SCENE_SLEEP_GOAL                           0x02
#define PROT_SCENE_SLEEP_RECORD_CONTENT                 0x03
#define PROT_SCENE_SLEEP_RECORD_PKG                     0x04
#define PROT_SCENE_SLEEP_RECORD_DEL                     0x05

#define PROT_DEV_TEST                                   0x04     /**< device test mode  */
#define PROT_TEST_MODE_EN                               0x01
#define PROT_AUTO_TEST                                  0x02
#define PROT_SUBJECT_TEST                               0x03
#define PROT_TEST_LIST                                  0x04
#define PROT_LED_TEST                                   0x05
#define PROT_MOTOR_TEST                                 0x07
#define PROT_MOVT_TEST                                  0x08
#define PROT_KEY_TEST                                   0x09


#define PROT_INTERACT                                   0x03     /**< 交互控制 */

#define PROT_NEW_MSG                                    0x01
#define PROT_MSG_SWITCH                                 0x02
#define PROT_INCALL_RESP                                0x03
#define PROT_CANCEL_MSG                                 0x04

#define PROT_FIND_PHONE                                 0x11

#define PROT_PHOTO_MODE                                 0x21
#define PROT_PHOTO_REQ                                  0x22
#define PROT_PHOTO_HEART_BEAT                           0x23

#define PROT_WATCH_HAND_MODE                            0x31
#define PROT_WATCH_HAND_CTRL                            0x32
#define PROT_WATCH_HAND_PARAM                           0x33
#define PROT_WATCH_HAND_HEARTBEAT                       0x34
#define PROT_CALIBRATE_TIME_STATE                       0x35  
#define PROT_CALIBRATE_EXPECT_SECONDS                   0x36

#define PROT_MEDIA_PLAY_PAUSE                           0x41
#define PROT_MEDIA_VOLUME                               0x42
#define PROT_MEDIA_UP_DOWN                              0x43

#define PROT_LONG_SIT_SWITCH                            0x51
#define PROT_LONG_SIT_PARAM                             0x52

#define PROT_DND_SWITCH                                 0x61
#define PROT_DND_PARAM                                  0x62

#define PROT_VIBRATION_SCALE                            0x71


#define PROT_ENTER_GERRN_MODE                           0x81
#define PROT_SET_FUNCTION_SETTINGS                      0x92


#define PORT_SENSOR                                     0x05      /*传感器数据*/

#define PORT_SENSOR_DATA_UPDATE                         0x01
#define PORT_SENSOR_UV_MEASURE                          0x02
#define PORT_SENSOR_MEASURE_DATA_UR                     0x03




#define PROT_PACK_TRANS                                 0xF0     /**< 数据包传输 */
#define PROT_PACK_TRANS_CH1                             0x01 // 数据通道 1 
#define PROT_PACK_TRANS_GET_CONTENT_COUNT               0x10 // 获取目录数 
#define PROT_PACK_TRANS_GET_CONTENT_INFO                0x11 // 获取目录内容
#define PROT_PACK_TRANS_GET_PKG                         0x12 // 请求数据包 
#define PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT       0x20 // 自动上传目录数 
#define PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO        0x21 // 自动上传目录内容 
#define PROT_PACK_TRANS_DEL_ALL                         0x30 // 删除所有数据
#define PROT_PACK_TRANS_DEL_DATATYPE                    0x31 // 按数据类型删除数据 
#define PROT_PACK_TRANS_DEL_CONTENT_UTC                 0x32 // 按目录 UTC 删除数据 

/*
 * 协议事件 ID 分组，最多不能超过 63 个分组
*/
typedef enum
{
    PROT_EVT_ID_GROUP_BLE_LINK       = 1,
    PROT_EVT_ID_GROUP_DEV_INFO,
    PROT_EVT_ID_GROUP_DAILY_SPORT,
    PROT_EVT_ID_GROUP_SENSOR_DATA,
    PROT_EVT_ID_GROUP_INTERACTION,
    PROT_EVT_ID_GROUP_PACK_TRANS,
    PROT_EVT_ID_GROUP_SLEEP_RECORD,
    PROT_EVT_ID_GROUP_TEST_MODE,
}prot_evt_id_group_t;

/*
 * 协议事件 ID，每个分组最多不能超过 1024 个事件 ID
*/
typedef enum
{
    PROT_EVT_ID_FIRST                           = 0,
    PROT_EVT_ID_INVALID                         = PROT_EVT_ID_FIRST,

    PROT_EVT_ID_ACK,

    PROT_EVT_ID_SET_BLE_ADV_NAME                = (PROT_EVT_ID_GROUP_BLE_LINK << 10),       /* BLE 链路 */
    PROT_EVT_ID_SET_BLE_PROJ_NUM,
    PROT_EVT_ID_CALL_BT_CONN_INTV,
    PROT_EVT_ID_ECHO_BT_CONN_INTV,
    PROT_EVT_ID_GET_BT_FW_VERSION,
    PROT_EVT_ID_RET_BT_FW_VERSION,
    PROT_EVT_ID_ECHO_BT_FW_VERSION,
    PROT_EVT_ID_IOS_PAIR_REQ,
    PROT_EVT_ID_SET_LINKLOSS_REMIND_SWITCH,
    PROT_EVT_ID_GET_LINKLOSS_REMIND_SWITCH,
    PROT_EVT_ID_RET_LINKLOSS_REMIND_SWITCH,

    PROT_EVT_ID_GET_PROD_NAME                   = (PROT_EVT_ID_GROUP_DEV_INFO << 10),       /* 设备信息 */
    PROT_EVT_ID_RET_PROD_NAME,
    PROT_EVT_ID_GET_PROD_VER,
    PROT_EVT_ID_RET_PROD_VER,
    PROT_EVT_ID_GET_PROD_DATE,
    PROT_EVT_ID_RET_PROD_DATE,
    PROT_EVT_ID_GET_PROD_SOC,
    PROT_EVT_ID_RET_PROD_SOC,
    PROT_EVT_ID_GET_PROD_SOC_NEW,
    PROT_EVT_ID_RET_PROD_SOC_NEW,
    PROT_EVT_ID_ECHO_PROD_SOC_NEW,
    PROT_EVT_ID_GET_PROD_BATT_VOL,
    PROT_EVT_ID_RET_PROD_BATT_VOL,
    PROT_EVT_ID_ECHO_PROD_BATT_VOL,
    PROT_EVT_ID_GET_PROD_RESET_INFO,
    PROT_EVT_ID_RET_PROD_RESET_INFO,    
    PROT_EVT_ID_GET_PROD_ERR_RESET_INFO,
    PROT_EVT_ID_RET_PROD_ERR_RESET_INFO,  
    PROT_EVT_ID_GET_PROD_HARD_RESET_INFO,
    PROT_EVT_ID_RET_PROD_HARD_RESET_INFO,  
    PROT_EVT_ID_GET_PROD_DF_RESET_INFO,
    PROT_EVT_ID_RET_PROD_DF_RESET_INFO,  
    PROT_EVT_ID_GET_PROD_KEY_RESET_INFO,
    PROT_EVT_ID_RET_PROD_KEY_RESET_INFO,
    PROT_EVT_ID_CALL_PROD_AUTH,
    PROT_EVT_ID_SET_PROD_AUTH,
    PROT_EVT_ID_GET_PROD_SN,
    PROT_EVT_ID_ECHO_PROD_SN,
    PROT_EVT_ID_GET_PROD_MANAGER_SN,
    PROT_EVT_ID_ECHO_PROD_MANAGER_SN,
    PROT_EVT_ID_GET_PROD_MAC,
    PROT_EVT_ID_RET_PROD_MAC,
    PROT_EVT_ID_GET_PROD_PRO_NUM,
    PROT_EVT_ID_RET_PROD_PRO_NUM,
    PROT_EVT_ID_SET_DATE,
    PROT_EVT_ID_GET_DATE,
    PROT_EVT_ID_RET_DATE,
    PROT_EVT_ID_SET_SEC_CITY_DATE,
    PROT_EVT_ID_GET_SEC_CITY_DATE,
    PROT_EVT_ID_RET_SEC_CITY_DATE,
    PROT_EVT_ID_SET_ALARM_CLK,
    PROT_EVT_ID_GET_ALARM_CLK,
    PROT_EVT_ID_RET_ALARM_CLK,
    PROT_EVT_ID_ECHO_ALARM_CLK,
    

    PROT_EVT_ID_CALL_NEW_MSG                    = (PROT_EVT_ID_GROUP_INTERACTION << 10),    /* 交互控制 */
    PROT_EVT_ID_SET_MSG_SWTICH,
    PROT_EVT_ID_GET_MSG_SWTICH,
    PROT_EVT_ID_RET_MSG_SWTICH,
    PROT_EVT_ID_CALL_INCALL_RESP,
    PROT_EVT_ID_CALL_CANCEL_MSG,
    PROT_EVT_ID_CALL_FIND_PHONE,                /* 查找手机 */
    PROT_EVT_ID_CALL_PHOTO_MODE,                /* 拍照 */                 
    PROT_EVT_ID_CALL_PHOTO_REQ,
    PROT_EVT_ID_RET_PHOTO_REQ,
    PROT_EVT_ID_ECHO_PHOTO_HEARTBEAT,
    PROT_EVT_ID_CALL_WATCH_HAND_MODE,           /* 指针 */
    PROT_EVT_ID_CALL_WATCH_HAND_CTRL,
    PROT_EVT_ID_SET_WATCH_HAND_PARAM,
    PROT_EVT_ID_ECHO_CALIBRATE_TIME_STATE,
    PROT_EVT_ID_ECHO_CALIBRATE_EXPECT_SECONDS,
    PROT_EVT_ID_ECHO_WATCH_HAND_HEARTBEAT,
    PROT_EVT_ID_SET_LONG_SIT_SWITCH,            /* 久坐 */
    PROT_EVT_ID_GET_LONG_SIT_SWITCH,
    PROT_EVT_ID_RET_LONG_SIT_SWITCH,
    PROT_EVT_ID_SET_LONG_SIT_PARAM,
    PROT_EVT_ID_GET_LONG_SIT_PARAM,
    PROT_EVT_ID_RET_LONG_SIT_PARAM,
    PROT_EVT_ID_SET_DND_SWITCH,                 /* 勿扰 */
    PROT_EVT_ID_GET_DND_SWITCH,
    PROT_EVT_ID_RET_DND_SWITCH,
    PROT_EVT_ID_SET_DND_PARAM,
    PROT_EVT_ID_GET_DND_PARAM,
    PROT_EVT_ID_RET_DND_PARAM,
    PROT_EVT_ID_SET_VIBRATION_SCALE,            /* 反馈 */
    PROT_EVT_ID_GET_VIBRATION_SCALE,
    PROT_EVT_ID_RET_VIBRATION_SCALE,
    PROT_EVT_ID_CALL_ENTER_GERRN_MODE,
    PROT_EVT_ID_CALL_SPEC_EVENT_REMIND,          /*特殊事件提醒*/
    PROT_EVT_ID_SET_FUNCTION_SETTINGS,           /*功能设置*/
    PROT_EVT_ID_GET_FUNCTION_SETTINGS,           /*功能设置*/
    PROT_EVT_ID_RET_FUNCTION_SETTINGS,           /*功能设置*/
    

    PORT_EVT_ID_CALL_UV_MEASURE_START              = (PROT_EVT_ID_GROUP_SENSOR_DATA << 10),    /*传感器数据*/
    PORT_EVT_ID_ECHO_UV_MEASURE,
    PORT_EVT_ID_CALL_UV_MEASURE_INDEX_START,
    PORT_EVT_ID_ECHO_UV_MEASURE_INDEX,

    PROT_EVT_ID_CALL_SCENE_DS_MODE                 = (PROT_EVT_ID_GROUP_DAILY_SPORT << 10),    /* 日常运动 */
    PROT_EVT_ID_SET_DS_STEPS_GOAL,               
    PROT_EVT_ID_GET_DS_STEPS_GOAL,
    PROT_EVT_ID_RET_DS_STEPS_GOAL,
    PROT_EVT_ID_GET_DS_TOTAL_DAILY_STEPS,
    PROT_EVT_ID_RET_DS_TOTAL_DAILY_STEPS,
    PROT_EVT_ID_ECHO_DS_TOTAL_DAILY_STEPS,
    
    PROT_EVT_ID_CALL_TEST_MODE_EN                    =   (PROT_EVT_ID_GROUP_TEST_MODE << 10),    /* test mode*/                    
    PROT_EVT_ID_ECHO_TEST_MODE_EN,
    PROT_EVT_ID_CALL_AUTO_TEST ,
    PROT_EVT_ID_ECHO_AUTO_TEST ,
    PROT_EVT_ID_CALL_SUBJECT_TEST,
    PROT_EVT_ID_ECHO_SUBJECT_TEST,
    PROT_EVT_ID_GET_TEST_LIST  ,
    PROT_EVT_ID_RET_TEST_LIST  ,
    PROT_EVT_ID_CALL_LED_TEST  ,  
    PROT_EVT_ID_CALL_MOTOR_TEST , 
    PROT_EVT_ID_CALL_MOVT_TEST  ,
    PROT_EVT_ID_CALL_KEY_TEST   ,
    PROT_EVT_ID_ECHO_KEY_TEST,


    
    
    PROT_EVT_ID_SET_SLEEP_GOAL                 = (PROT_EVT_ID_GROUP_SLEEP_RECORD << 10),    /* sleep record*/           
    PROT_EVT_ID_GET_SLEEP_GOAL,
    PROT_EVT_ID_RET_SLEEP_GOAL,
    PROT_EVT_ID_GET_SLEEP_RECORD_COUNT,
    PROT_EVT_ID_RET_SLEEP_RECORD_COUNT,    
    PROT_EVT_ID_GET_SLEEP_RECORD_INFO,
    PROT_EVT_ID_RET_SLEEP_RECORD_INFO,
    PROT_EVT_ID_ECHO_SLEEP_RECORD_INFO,
    PROT_EVT_ID_CALL_SLEEP_RECORD_DEL,
    PROT_EVT_ID_ECHO_SLEEP_RECORD_DEL,
    
    


    PROT_EVT_ID_GET_CONTENT_COUNT              = (PROT_EVT_ID_GROUP_PACK_TRANS << 10),     /* 数据包上传 */
    PROT_EVT_ID_RET_CONTENT_COUNT,
    PROT_EVT_ID_GET_CONTENT_INFO,
    PROT_EVT_ID_RET_CONTENT_INFO,
    PROT_EVT_ID_GET_PKG,
    PROT_EVT_ID_RET_PKG,
    PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_COUNT,
    PROT_EVT_ID_RET_AUTO_UPDATE_CONTENT_COUNT,
    PROT_EVT_ID_ECHO_AUTO_UPDATE_CONTENT_COUNT,
    PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_INFO,
    PROT_EVT_ID_RET_AUTO_UPDATE_CONTENT_INFO,
    PROT_EVT_ID_ECHO_AUTO_UPDATE_CONTENT_INFO,
    PROT_EVT_ID_CALL_DEL_ALL,
    PROT_EVT_ID_ECHO_DEL_ALL,
    PROT_EVT_ID_CALL_DEL_DATATYPE,
    PROT_EVT_ID_ECHO_DEL_DATATYPE,
    PROT_EVT_ID_CALL_DEL_CONTENT_UTC,
    PROT_EVT_ID_ECHO_DEL_CONTENT_UTC,

}prot_evt_id_t;

/*
 * 将接口，映射成对应的事件 ID，@intf_evtid_map_t
*/
#define INTERFACE_AND_EVENT_ID_MAP_DEFINES                                                                                          \
{                                                                                                                                   \
{{PROT_SET,     PROT_LINK,                  PROT_BT_ADV_NAME},                          PROT_EVT_ID_SET_BLE_ADV_NAME},              \
{{PROT_ECHO,    PROT_LINK,                  PROT_IOS_PAIR_REQ},                         PROT_EVT_ID_IOS_PAIR_REQ},                  \
{{PROT_SET,     PROT_LINK,                  PROT_LINKLOSS_REMIND_SWITCH},               PROT_EVT_ID_SET_LINKLOSS_REMIND_SWITCH},    \
{{PROT_GET,     PROT_LINK,                  PROT_LINKLOSS_REMIND_SWITCH},               PROT_EVT_ID_GET_LINKLOSS_REMIND_SWITCH},    \
{{PROT_RET,     PROT_LINK,                  PROT_LINKLOSS_REMIND_SWITCH},               PROT_EVT_ID_RET_LINKLOSS_REMIND_SWITCH},    \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_NAME},                            PROT_EVT_ID_GET_PROD_NAME},                 \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_NAME},                            PROT_EVT_ID_RET_PROD_NAME},                 \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_VER},                             PROT_EVT_ID_GET_PROD_VER},                  \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_VER},                             PROT_EVT_ID_RET_PROD_VER},                  \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_DATE},                            PROT_EVT_ID_GET_PROD_DATE},                 \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_DATE},                            PROT_EVT_ID_RET_PROD_DATE},                 \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_SOC},                             PROT_EVT_ID_GET_PROD_SOC},                  \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_SOC},                             PROT_EVT_ID_RET_PROD_SOC},                  \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_SOC_NEW},                         PROT_EVT_ID_GET_PROD_SOC_NEW},              \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_SOC_NEW},                         PROT_EVT_ID_RET_PROD_SOC_NEW},              \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_SOC_NEW},                         PROT_EVT_ID_ECHO_PROD_SOC_NEW},             \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_BATT_VOL},                        PROT_EVT_ID_GET_PROD_BATT_VOL},              \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_BATT_VOL},                        PROT_EVT_ID_RET_PROD_BATT_VOL},              \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_BATT_VOL},                        PROT_EVT_ID_ECHO_PROD_BATT_VOL},             \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_RESET_INFO},                      PROT_EVT_ID_GET_PROD_RESET_INFO},           \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_RESET_INFO},                      PROT_EVT_ID_RET_PROD_RESET_INFO},           \
{{PROT_CALL,    PROT_DEV_INFO,              PROT_PROD_AUTH},                            PROT_EVT_ID_CALL_PROD_AUTH},                \
{{PROT_SET,     PROT_DEV_INFO,              PROT_PROD_AUTH},                            PROT_EVT_ID_SET_PROD_AUTH},                 \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_SN},                              PROT_EVT_ID_GET_PROD_SN},                   \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_SN},                              PROT_EVT_ID_ECHO_PROD_SN},                  \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_MANAGER_SN},                      PROT_EVT_ID_GET_PROD_MANAGER_SN},           \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_MANAGER_SN},                      PROT_EVT_ID_ECHO_PROD_MANAGER_SN},          \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_MAC},                             PROT_EVT_ID_GET_PROD_MAC},                  \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_MAC},                             PROT_EVT_ID_RET_PROD_MAC},                  \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_PRO_NUM},                         PROT_EVT_ID_GET_PROD_PRO_NUM},              \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_PRO_NUM},                         PROT_EVT_ID_RET_PROD_PRO_NUM},              \
{{PROT_SET,     PROT_DEV_INFO,              PROT_DATE},                                 PROT_EVT_ID_SET_DATE},                      \
{{PROT_GET,     PROT_DEV_INFO,              PROT_DATE},                                 PROT_EVT_ID_GET_DATE},                      \
{{PROT_RET,     PROT_DEV_INFO,              PROT_DATE},                                 PROT_EVT_ID_RET_DATE},                      \
{{PROT_SET,     PROT_DEV_INFO,              PROT_ALARM_CLK},                            PROT_EVT_ID_SET_ALARM_CLK},                 \
{{PROT_GET,     PROT_DEV_INFO,              PROT_ALARM_CLK},                            PROT_EVT_ID_GET_ALARM_CLK},                 \
{{PROT_RET,     PROT_DEV_INFO,              PROT_ALARM_CLK},                            PROT_EVT_ID_RET_ALARM_CLK},                 \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_ALARM_CLK},                            PROT_EVT_ID_ECHO_ALARM_CLK},                \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_RESET_INFO},                      PROT_EVT_ID_GET_PROD_RESET_INFO},           \
{{PROT_RET,     PROT_DEV_INFO,              PROT_PROD_RESET_INFO},                      PROT_EVT_ID_RET_PROD_RESET_INFO},           \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_ERR_RESET_INFO},                  PROT_EVT_ID_GET_PROD_ERR_RESET_INFO},         \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_ERR_RESET_INFO},                  PROT_EVT_ID_RET_PROD_ERR_RESET_INFO},         \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_HARD_RESET_INFO},                 PROT_EVT_ID_GET_PROD_HARD_RESET_INFO},           \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_HARD_RESET_INFO},                 PROT_EVT_ID_RET_PROD_HARD_RESET_INFO},           \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_DF_RESET_INFO},                   PROT_EVT_ID_GET_PROD_DF_RESET_INFO},           \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_DF_RESET_INFO},                   PROT_EVT_ID_RET_PROD_DF_RESET_INFO},           \
{{PROT_GET,     PROT_DEV_INFO,              PROT_PROD_KEY_RESET_INFO},                  PROT_EVT_ID_GET_PROD_KEY_RESET_INFO},       \
{{PROT_ECHO,    PROT_DEV_INFO,              PROT_PROD_KEY_RESET_INFO},                  PROT_EVT_ID_RET_PROD_KEY_RESET_INFO},       \
{{PROT_CALL,    PROT_DAILY_SPORT_SCENE,     PROT_SCENE_DS_MODE},                        PROT_EVT_ID_CALL_SCENE_DS_MODE},             \
{{PROT_SET,     PROT_DAILY_SPORT_SCENE,     PROT_SCENE_DS_GOAL},                        PROT_EVT_ID_SET_DS_STEPS_GOAL},             \
{{PROT_GET,     PROT_DAILY_SPORT_SCENE,     PROT_SCENE_DS_GOAL},                        PROT_EVT_ID_GET_DS_STEPS_GOAL},             \
{{PROT_RET,     PROT_DAILY_SPORT_SCENE,     PROT_SCENE_DS_GOAL},                        PROT_EVT_ID_RET_DS_STEPS_GOAL},             \
{{PROT_GET,     PROT_DAILY_SPORT_SCENE,     PROT_SCENE_DS_TOTAL_DAILY_STEPS},           PROT_EVT_ID_GET_DS_TOTAL_DAILY_STEPS},      \
{{PROT_RET,     PROT_DAILY_SPORT_SCENE,     PROT_SCENE_DS_TOTAL_DAILY_STEPS},           PROT_EVT_ID_RET_DS_TOTAL_DAILY_STEPS},      \
{{PROT_ECHO,    PROT_DAILY_SPORT_SCENE,     PROT_SCENE_DS_TOTAL_DAILY_STEPS},           PROT_EVT_ID_ECHO_DS_TOTAL_DAILY_STEPS},     \
{{PROT_SET,     PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_GOAL},                     PROT_EVT_ID_SET_SLEEP_GOAL},                \
{{PROT_GET,     PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_GOAL},                     PROT_EVT_ID_GET_SLEEP_GOAL},                \
{{PROT_RET,     PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_GOAL},                     PROT_EVT_ID_RET_SLEEP_GOAL},                \
{{PROT_GET,     PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_RECORD_CONTENT},           PROT_EVT_ID_GET_SLEEP_RECORD_COUNT},        \
{{PROT_RET,     PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_RECORD_CONTENT},           PROT_EVT_ID_RET_SLEEP_RECORD_COUNT},        \
{{PROT_GET,     PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_RECORD_PKG},               PROT_EVT_ID_GET_SLEEP_RECORD_INFO},        \
{{PROT_RET,     PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_RECORD_PKG},               PROT_EVT_ID_RET_SLEEP_RECORD_INFO},        \
{{PROT_ECHO,    PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_RECORD_PKG},               PROT_EVT_ID_ECHO_SLEEP_RECORD_INFO},      \
{{PROT_CALL,    PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_RECORD_DEL},               PROT_EVT_ID_CALL_SLEEP_RECORD_DEL},        \
{{PROT_ECHO,    PROT_SLEEP_RECORD_SCENE,    PROT_SCENE_SLEEP_RECORD_DEL},               PROT_EVT_ID_ECHO_SLEEP_RECORD_DEL},        \
{{PROT_CALL,    PROT_INTERACT,              PROT_NEW_MSG},                              PROT_EVT_ID_CALL_NEW_MSG},                  \
{{PROT_SET,     PROT_INTERACT,              PROT_MSG_SWITCH},                           PROT_EVT_ID_SET_MSG_SWTICH},                \
{{PROT_GET,     PROT_INTERACT,              PROT_MSG_SWITCH},                           PROT_EVT_ID_GET_MSG_SWTICH},                \
{{PROT_RET,     PROT_INTERACT,              PROT_MSG_SWITCH},                           PROT_EVT_ID_RET_MSG_SWTICH},                \
{{PROT_CALL,    PROT_INTERACT,              PROT_INCALL_RESP},                          PROT_EVT_ID_CALL_INCALL_RESP},              \
{{PROT_CALL,    PROT_INTERACT,              PROT_CANCEL_MSG},                           PROT_EVT_ID_CALL_CANCEL_MSG},               \
{{PROT_CALL,    PROT_INTERACT,              PROT_FIND_PHONE},                           PROT_EVT_ID_CALL_FIND_PHONE},               \
{{PROT_CALL,    PROT_INTERACT,              PROT_PHOTO_MODE},                           PROT_EVT_ID_CALL_PHOTO_MODE},               \
{{PROT_CALL,    PROT_INTERACT,              PROT_PHOTO_REQ},                            PROT_EVT_ID_CALL_PHOTO_REQ},                \
{{PROT_RET,     PROT_INTERACT,              PROT_PHOTO_REQ},                            PROT_EVT_ID_RET_PHOTO_REQ},                 \
{{PROT_ECHO,    PROT_INTERACT,              PROT_PHOTO_HEART_BEAT},                     PROT_EVT_ID_ECHO_PHOTO_HEARTBEAT},          \
{{PROT_CALL,    PROT_INTERACT,              PROT_WATCH_HAND_MODE},                      PROT_EVT_ID_CALL_WATCH_HAND_MODE},          \
{{PROT_CALL,    PROT_INTERACT,              PROT_WATCH_HAND_CTRL},                      PROT_EVT_ID_CALL_WATCH_HAND_CTRL},          \
{{PROT_CALL,    PROT_INTERACT,              PROT_WATCH_HAND_PARAM},                     PROT_EVT_ID_SET_WATCH_HAND_PARAM},          \
{{PROT_ECHO,    PROT_INTERACT,              PROT_CALIBRATE_TIME_STATE},                 PROT_EVT_ID_ECHO_CALIBRATE_TIME_STATE},     \
{{PROT_ECHO,    PROT_INTERACT,              PROT_CALIBRATE_EXPECT_SECONDS},             PROT_EVT_ID_ECHO_CALIBRATE_EXPECT_SECONDS}, \
{{PROT_ECHO,    PROT_INTERACT,              PROT_WATCH_HAND_HEARTBEAT},                 PROT_EVT_ID_ECHO_WATCH_HAND_HEARTBEAT},     \
{{PROT_SET,     PROT_INTERACT,              PROT_LONG_SIT_SWITCH},                      PROT_EVT_ID_SET_LONG_SIT_SWITCH},           \
{{PROT_GET,     PROT_INTERACT,              PROT_LONG_SIT_SWITCH},                      PROT_EVT_ID_GET_LONG_SIT_SWITCH},           \
{{PROT_RET,     PROT_INTERACT,              PROT_LONG_SIT_SWITCH},                      PROT_EVT_ID_RET_LONG_SIT_SWITCH},           \
{{PROT_SET,     PROT_INTERACT,              PROT_LONG_SIT_PARAM},                       PROT_EVT_ID_SET_LONG_SIT_PARAM},            \
{{PROT_GET,     PROT_INTERACT,              PROT_LONG_SIT_PARAM},                       PROT_EVT_ID_GET_LONG_SIT_PARAM},            \
{{PROT_RET,     PROT_INTERACT,              PROT_LONG_SIT_PARAM},                       PROT_EVT_ID_RET_LONG_SIT_PARAM},            \
{{PROT_SET,     PROT_INTERACT,              PROT_VIBRATION_SCALE},                      PROT_EVT_ID_SET_VIBRATION_SCALE},           \
{{PROT_GET,     PROT_INTERACT,              PROT_VIBRATION_SCALE},                      PROT_EVT_ID_GET_VIBRATION_SCALE},           \
{{PROT_RET,     PROT_INTERACT,              PROT_VIBRATION_SCALE},                      PROT_EVT_ID_RET_VIBRATION_SCALE},           \
{{PROT_CALL,    PROT_INTERACT,              PROT_ENTER_GERRN_MODE},                     PROT_EVT_ID_CALL_ENTER_GERRN_MODE},         \
{{PROT_SET,     PROT_INTERACT,              PROT_SET_FUNCTION_SETTINGS},                PROT_EVT_ID_SET_FUNCTION_SETTINGS},         \
{{PROT_GET,     PROT_INTERACT,              PROT_SET_FUNCTION_SETTINGS},                PROT_EVT_ID_GET_FUNCTION_SETTINGS},         \
{{PROT_RET,     PROT_INTERACT,              PROT_SET_FUNCTION_SETTINGS},                PROT_EVT_ID_RET_FUNCTION_SETTINGS},         \
{{PROT_SET,     PROT_INTERACT,              PROT_DND_SWITCH},                           PROT_EVT_ID_SET_DND_SWITCH},                \
{{PROT_GET,     PROT_INTERACT,              PROT_DND_SWITCH},                           PROT_EVT_ID_GET_DND_SWITCH},                \
{{PROT_RET,     PROT_INTERACT,              PROT_DND_SWITCH},                           PROT_EVT_ID_RET_DND_SWITCH},                \
{{PROT_SET,     PROT_INTERACT,              PROT_DND_PARAM},                            PROT_EVT_ID_SET_DND_PARAM},                 \
{{PROT_GET,     PROT_INTERACT,              PROT_DND_PARAM},                            PROT_EVT_ID_GET_DND_PARAM},                 \
{{PROT_RET,     PROT_INTERACT,              PROT_DND_PARAM},                            PROT_EVT_ID_RET_DND_PARAM},                 \
{{PROT_CALL,    PORT_SENSOR,                PORT_SENSOR_UV_MEASURE},                    PORT_EVT_ID_CALL_UV_MEASURE_START},         \
{{PROT_ECHO,    PORT_SENSOR,                PORT_SENSOR_UV_MEASURE},                    PORT_EVT_ID_ECHO_UV_MEASURE},               \
{{PROT_CALL,    PORT_SENSOR,                PORT_SENSOR_MEASURE_DATA_UR},               PORT_EVT_ID_CALL_UV_MEASURE_INDEX_START},   \
{{PROT_ECHO,    PORT_SENSOR,                PORT_SENSOR_MEASURE_DATA_UR},               PORT_EVT_ID_ECHO_UV_MEASURE_INDEX},         \
{{PROT_GET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_GET_CONTENT_COUNT},         PROT_EVT_ID_GET_CONTENT_COUNT},             \
{{PROT_RET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_GET_CONTENT_COUNT},         PROT_EVT_ID_RET_CONTENT_COUNT},             \
{{PROT_GET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_GET_CONTENT_INFO},          PROT_EVT_ID_GET_CONTENT_INFO},              \
{{PROT_RET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_GET_CONTENT_INFO},          PROT_EVT_ID_RET_CONTENT_INFO},              \
{{PROT_GET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_GET_PKG},                   PROT_EVT_ID_GET_PKG},                       \
{{PROT_RET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_GET_PKG},                   PROT_EVT_ID_RET_PKG},                       \
{{PROT_CALL,    PROT_PACK_TRANS,            PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT}, PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_COUNT},\
{{PROT_RET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT}, PROT_EVT_ID_RET_AUTO_UPDATE_CONTENT_COUNT}, \
{{PROT_ECHO,    PROT_PACK_TRANS,            PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_COUNT}, PROT_EVT_ID_ECHO_AUTO_UPDATE_CONTENT_COUNT},\
{{PROT_CALL,    PROT_PACK_TRANS,            PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO},  PROT_EVT_ID_CALL_AUTO_UPDATE_CONTENT_INFO}, \
{{PROT_RET,     PROT_PACK_TRANS,            PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO},  PROT_EVT_ID_RET_AUTO_UPDATE_CONTENT_INFO},  \
{{PROT_ECHO,    PROT_PACK_TRANS,            PROT_PACK_TRANS_AUTO_UPDATE_CONTENT_INFO},  PROT_EVT_ID_ECHO_AUTO_UPDATE_CONTENT_INFO}, \
{{PROT_CALL,    PROT_PACK_TRANS,            PROT_PACK_TRANS_DEL_ALL},                   PROT_EVT_ID_CALL_DEL_ALL},                  \
{{PROT_ECHO,    PROT_PACK_TRANS,            PROT_PACK_TRANS_DEL_ALL},                   PROT_EVT_ID_ECHO_DEL_ALL},                  \
{{PROT_CALL,    PROT_PACK_TRANS,            PROT_PACK_TRANS_DEL_DATATYPE},              PROT_EVT_ID_CALL_DEL_DATATYPE},             \
{{PROT_ECHO,    PROT_PACK_TRANS,            PROT_PACK_TRANS_DEL_DATATYPE},              PROT_EVT_ID_ECHO_DEL_DATATYPE},             \
{{PROT_CALL,    PROT_PACK_TRANS,            PROT_PACK_TRANS_DEL_CONTENT_UTC},           PROT_EVT_ID_CALL_DEL_CONTENT_UTC},          \
{{PROT_ECHO,    PROT_PACK_TRANS,            PROT_PACK_TRANS_DEL_CONTENT_UTC},           PROT_EVT_ID_ECHO_DEL_CONTENT_UTC},          \
{{PROT_CALL,    PROT_DEV_TEST,              PROT_TEST_MODE_EN},                         PROT_EVT_ID_CALL_TEST_MODE_EN},             \
{{PROT_RET,     PROT_DEV_TEST,              PROT_TEST_MODE_EN},                         PROT_EVT_ID_ECHO_TEST_MODE_EN},             \
{{PROT_CALL,    PROT_DEV_TEST,              PROT_AUTO_TEST},                            PROT_EVT_ID_CALL_AUTO_TEST},                \
{{PROT_ECHO,     PROT_DEV_TEST,              PROT_AUTO_TEST},                           PROT_EVT_ID_ECHO_AUTO_TEST},                \
{{PROT_CALL,    PROT_DEV_TEST,              PROT_SUBJECT_TEST},                         PROT_EVT_ID_CALL_SUBJECT_TEST},             \
{{PROT_ECHO,    PROT_DEV_TEST,              PROT_SUBJECT_TEST},                         PROT_EVT_ID_ECHO_SUBJECT_TEST},             \
{{PROT_GET,     PROT_DEV_TEST,              PROT_TEST_LIST},                            PROT_EVT_ID_GET_TEST_LIST},                 \
{{PROT_RET,     PROT_DEV_TEST,              PROT_TEST_LIST},                            PROT_EVT_ID_RET_TEST_LIST},                 \
{{PROT_CALL,    PROT_DEV_TEST,              PROT_LED_TEST},                             PROT_EVT_ID_CALL_LED_TEST},                  \
{{PROT_CALL,    PROT_DEV_TEST,              PROT_MOTOR_TEST},                           PROT_EVT_ID_CALL_MOTOR_TEST},                \
{{PROT_CALL,    PROT_DEV_TEST,              PROT_MOVT_TEST},                            PROT_EVT_ID_CALL_MOVT_TEST},                 \
{{PROT_CALL,    PROT_DEV_TEST,              PROT_KEY_TEST},                             PROT_EVT_ID_CALL_KEY_TEST},                  \
{{PROT_ECHO,    PROT_DEV_TEST,              PROT_KEY_TEST},                             PROT_EVT_ID_ECHO_KEY_TEST},                  \
}

#endif /* PROTOCOL_COMMON_H__ */


