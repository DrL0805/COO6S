/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**
 * @defgroup EVENT_BUS Event bus
 * @ingroup other
 * @{
 * @brief Details of the event bus mechanism and its functions.
 */
#ifndef __EVENT_BUS_H__
#define __EVENT_BUS_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_errors.h"

// Enable anonymous unions.
#if defined(__CC_ARM)
    #pragma push
    #pragma anon_unions
#elif defined(__ICCAR_)
    #pragma language=save
    #pragma language=extended
#elif defined(__GNUC__)
    // Anonymous unions are supported by default.
#endif

/**@brief Translate event type to event group. */
#define EVENT_GROUP(_event_type)     (event_group_t)((unsigned long)(_event_type) >> 8)

/**@brief Event groups. */
typedef enum
{
    EVT_GROUP_SYSTEM,       /**< System state events. */

    EVT_GROUP_BUTTON,

    EVT_GROUP_COMS,
    
    EVT_GROUP_TEST,

} event_group_t;

/**@brief Event types. */
typedef enum
{
    /* System Events */
    EVT_SYSTEM_STATE            = (EVT_GROUP_SYSTEM << 8),
    EVT_SYSTEM_BATTERY_LEVEL,
    EVT_SYSTEM_BT_CONN_STATE,
    EVT_SYSTEM_UTC_SECOND,
    EVT_SYSTEM_UTC_10SECOND,
    EVT_SYSTEM_UTC_MINUTE,
    EVT_SYSTEM_UTC_DAY,
    EVT_SYSTEM_STEPS_GOAL_FINISHED,
    EVT_SYSTEM_STEPS_CHANGED,
    EVT_SYSTEM_SHAKE_HAND,
    EVT_SYSTEM_LONG_SIT_TRIGGERED,
    EVT_SYSTEM_IDLE_ENTER,
    EVT_SYSTEM_IDLE_EXIT,
    EVT_SYSTEM_PRESSED_TO_POWERON,
    EVT_SYSTEM_PRESSED_TO_ENTER_NORMAL,
    EVT_SYSTEM_VIBRATOR_STATE,
    EVT_SYSTEM_LED_STATE,
    EVT_SYSTEM_MAG_ADJUST,
    EVT_SYSTEM_MAG_ADJUSTING,
    EVT_SYSTEM_MAG_COMPLETE_ADJUST,
    EVT_SYSTEM_MAG_COMPASS_ANGLE,
    EVT_SYSTEM_MAG_COMPASS_ENTER,     /*进入指北针模式*/
    EVT_SYSTEM_MAG_COMPASS_EXIT,
    /* BUTTON */
    EVT_BUTTON_PRESSED          = (EVT_GROUP_BUTTON << 8),

    /* EVT_GROUP_COMS */
    EVT_COMS_APP_RECV           = (EVT_GROUP_COMS   << 8),
    EVT_COMS_PROT_EXEC,
  

    /* EVT_GROUP_TEST */
    EVT_TEST_MODE_IN               =(EVT_GROUP_TEST << 8),
    EVT_TEST_MODE_OUT,
    EVT_TEST_BATT_VOL_START,
    EVT_TEST_BATT_VOL_END,
    EVT_TEST_MOVT_HAND_0_FORWARD,
    EVT_TEST_MOVT_HAND_1_FORWARD,
} event_type_t;

typedef struct
{
    event_type_t type;

    union {
        union {
            uint8_t      system_state;      // @system_state_e
            uint8_t      ble_conn_state;    // @system_ble_conn_state_event_e
            uint8_t      batt_level;        // 电池电量，0 ~ 100 %
            uint32_t     data;
        } system;

        struct {
            uint16_t     button_event;      // @button_event_e
        } button;

        union {
            uint32_t     app_recv_event;    // @app_recv_event_e
            uint32_t     prot_execution;    // @prot_execution_event_e
            uint32_t     coms_data;
        } coms;
        
       
        union test_t{
            uint32_t     test_event;        // @test_event_e
            uint32_t     test_data;
        } test;      
        
    };
} event_t;

#if defined(__CC_ARM)
    #pragma pop
#elif defined(__ICCAR_)
    #pragma language=restore
#elif defined(__GNUC__)
    // Anonymous unions are supported by default.
#endif

// EVT_SYSTEM_STATE system_state
enum system_state_e
{
    SYSTEM_STATE_ELETRIFY,                      // 上电
    SYSTEM_STATE_ELETRIFY_TEST,                 // test state
    SYSTEM_STATE_STORAGE,                       // 仓储状态，所有功能都失效
    SYSTEM_STATE_POWERON_FAIL,                  // 开机失败
    SYSTEM_STATE_NORMAL,                        // 正常状态，所有功能都有效
    SYSTEM_STATE_DND,                           // 勿扰状态，只有提醒功能失效，Do Not Disturb
    SYSTEM_STATE_GREEN_MODE,                    // 绿色模式，只有走时功能，其他所有关掉（蓝牙、运动记录等）
    SYSTEM_STATE_SAVE_POWER_MODE,               // 省电模式，关闭蓝牙和提醒
    SYSTEM_STATE_LOWPOWER,                      // 低电状态，只有走时功能(包括表针指示功能)有效
    SYSTEM_STATE_DFU,                           // DFU 状态，进 DFU 模式，准备 OTA 升级
    SYSTEM_STATE_RESUME_F_GREEN,                // 从绿色模式恢复
    SYSTEM_STATE_RESUME_F_SAVE,                 // 从省电模式恢复
    SYSTEM_STATE_RESET,                         // 复位状态，系统复位后进仓储模式
    SYSTEM_STATE_STORE_RESET_REASON,             // 
};

// EVT_SYSTEM_BT_CONN_STATE ble_conn_state
enum system_ble_conn_state_event_e
{
    BT_CONN_STATE_INIT,                         // 蓝牙初始化
    BT_CONN_STATE_DISABLED,                     // 关闭蓝牙
//    BT_CONN_STATE_FAKE_ADV,                     // 假广播（空闲状态关闭了蓝牙，用户查看还是广播状态）
    BT_CONN_STATE_DISCONNECTED,                 // 断开连接
    BT_CONN_STATE_ADVERTISING_BONDABLE,         // 可绑定广播
    BT_CONN_STATE_ADVERTISING,                  // 正常广播
    BT_CONN_STATE_CONNECTED,                    // 已连接
    BT_CONN_STATE_SECURED,                      // 已加密
};

// EVT_BUTTON_PRESSED button_event
enum button_event_e
{
    BUTTON_EVT_BT_EN_SWTICH,              // 切换 BLE 使能状态
    BUTTON_EVT_CHECK_BATT_LEVEL,
    BUTTON_EVT_CHECK_BT_STATE,                  // 检查蓝牙状态
    BUTTON_EVT_PHOTO_REQ,                       // 请求拍照
    BUTTON_EVT_FIND_PHONE,                      // 查找手机
    BUTTON_EVT_REJECT_CALLIN,                   // 拒接来电
    BUTTON_EVT_ACCEPT_CALLIN,                   // 接听来电
    BUTTON_EVT_INDICATE_CALLIN,                 // 指示来电
    BUTTON_EVT_INDICATE_WEEK,                   // 指示星期
    BUTTON_EVT_INDICATE_DAY,                    // 指示日期
    BUTTON_EVT_INDICATE_STEPS_GOAL,             // 指示运动完成度
    BUTTON_EVT_INDICATE_NEW_MSG,                // 指示新消息
    BUTTON_EVT_ENTER_STORAGE_STATE,             // 进入仓储状态
    BUTTON_EVT_AUTH_OK,                         // 授权成功
    BUTTON_EVT_WATCH_HAND_STEP_FORWARD,         // 正走一步
    BUTTON_EVT_WATCH_HAND_STEP_BACKWARD,        // 反走一步
    BUTTON_EVT_START_DFU,                       // 开始 DFU 
    BUTTON_EVT_BT_DISABLE,
    BUTTON_EVT_SYSTEM_RESUME_F_GREEN,           //从绿色模式恢复正常模式
    BUTTON_EVT_SYSTEM_RESUME_F_SAVE,            //从省电模式恢复正常模式   
    BUTTON_EVT_SYSTEM_RESET,

    BUTTON_EVT_CLEAR_RESET_INFO, // 20
    BUTTON_EVT_BTN0_SHORT_REL,
    BUTTON_EVT_BTN1_SHORT_REL,
    BUTTON_EVT_BTN2_SHORT_REL,
    
    BUTTON_EVT_COMPASS,    //指示指北标志
    BUTTON_EVT_ADJUST_COMPASS,
    BUTTON_EVT_EXIT_COMPASS,
    BUTTON_EVT_INVALID = 0xFFFF,
};

// EVT_COMS_APP_RECV app_recv_event
enum app_recv_event_e
{
    APP_RECV_EVT_LINKLOSS_ENABLED_CONFIG,       // 连接断开提醒使能配置
    APP_RECV_EVT_AUTH_ENTER_MODE,               // 进入授权模式
    APP_RECV_EVT_AUTH_EXIT_MODE,                // 退出授权模式
    APP_RECV_EVT_AUTH_FORCE_OK,                 // 强制授权成功
    APP_RECV_EVT_IOS_PAIRING_REQ,                // ios 请求配对
    APP_RECV_EVT_MSG_NEW,                       // 新消息
    APP_RECV_EVT_MSG_CANCEL,                    // 消息被取消
    APP_RECV_EVT_PHOTO_ENTER_MODE,              // 进入拍照模式
    APP_RECV_EVT_PHOTO_EXIT_MODE,               // 退出拍照模式
    APP_RECV_EVT_PHOTO_MODE_HEART,              // 拍照模式心跳信息
    APP_RECV_EVT_WATCH_HAND_ENTER_MODE,         // 进入校针模式
    APP_RECV_EVT_WATCH_HAND_SET_PARAM,          // 设置校针参数
    APP_RECV_EVT_WATCH_HAND_EXIT_MODE,          // 退出校针模式
    APP_RECV_EVT_WATCH_HAND_STEP_FORWARD,       // 表针前进(正转)一步
    APP_RECV_EVT_WATCH_HAND_STEP_BACKWARD,      // 表针后退(反转)一步
    APP_RECV_EVT_WATCH_HAND_STOP,               // 表针暂停
    APP_RECV_EVT_WATCH_HAND_RESUME,             // 表针恢复 
    APP_RECV_EVT_WATCH_HAND_CHECK,              // 开始校针
    APP_RECV_EVT_LONG_SIT_START,                // 久坐提醒功能启动
    APP_RECV_EVT_LONG_SIT_STOP,                 // 久坐提醒功能关闭
    APP_RECV_EVT_LONG_SIT_PARAM_UPDATE,         // 久坐提醒参数更新
    APP_RECV_EVT_DND_START,                     // 勿扰功能启动
    APP_RECV_EVT_DND_STOP,                      // 勿扰功能关闭
    APP_RECV_EVT_DND_PARAM_UPDATE,              // 勿扰参数更新
    APP_RECV_EVT_DAILY_SPORT_STEPS_GOAL,        // 运动目标设置
    APP_RECV_EVT_SPEC_EVENT_REMIND,             // 特殊事件提醒
    APP_RECV_EVT_UV_MEASURE_START,              // 开启UV测量
    APP_RECV_EVT_GET_BATT_VOL,                  // 获取电池电量
    APP_RECV_EVT_INCALL,
    APP_RECV_EVT_INVALID = 0xFFFF,
};

// EVT_COMS_PROT_EXCU
enum prot_execution_event_e
{
    PROT_EXEC_EVT_ALARM_TRIGGERED,              // 闹钟触发
    PROT_EXEC_EVT_LINKLOSS_TRIGGERED,           // 连接断开触发
};

enum test_event_e
{
    TEST_EVT_FLASH_READ_FAKE_DATA,              //从flash读出睡眠和计步数据
    TEST_EVT_FLASH_WRITE_FAKE_DATA,             //写入假的睡眠和计步数据
    TEST_EVT_WATCH_HAND_FORWARD,                         //指针正转
    TEST_EVT_WATCH_HAND_BACKWARD,                        //指针反转
  
    TEST_EVT_WAVEFORM_PARAM_1_UPDATE,
    TEST_EVT_WAVEFORM_PARAM_2_UPDATE,
    TEST_EVT_WAVEFORM_PARAM_3_UPDATE,
    TEST_EVT_WAVEFORM_PARAM_4_UPDATE,
};

/**@brief Event handler.
 *
 * @param[in]   p_event Pointer to the event structure.
 *
 * @return      true if further processing of the given event should be abandoned.
 */
typedef bool (*event_handler_t)(const event_t *p_event);

/**@brief Initialize event bus.
 *
 * @param[in]   event_handlers_table    Table with event handlers.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
ret_code_t event_bus_init(const event_handler_t *event_handlers_table);

/**@brief Send event.
 *
 * @param[in]   event_type  Event type.
 * @param[in]   ...         Event data.
 *
 * @note                    Event data elements are mapped one to one to data fields in the event structure.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
extern ret_code_t event_send(event_type_t event_type, ...);

/**@brief Print event bus statistics. */
void event_bus_print_stats(void);

#endif /** __EVENT_BUS_H__ */

/**
  @}
*/
