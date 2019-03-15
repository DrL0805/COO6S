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
    EVT_SYSTEM_MAG_COMPASS_ENTER,     /*����ָ����ģʽ*/
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
            uint8_t      batt_level;        // ��ص�����0 ~ 100 %
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
    SYSTEM_STATE_ELETRIFY,                      // �ϵ�
    SYSTEM_STATE_ELETRIFY_TEST,                 // test state
    SYSTEM_STATE_STORAGE,                       // �ִ�״̬�����й��ܶ�ʧЧ
    SYSTEM_STATE_POWERON_FAIL,                  // ����ʧ��
    SYSTEM_STATE_NORMAL,                        // ����״̬�����й��ܶ���Ч
    SYSTEM_STATE_DND,                           // ����״̬��ֻ�����ѹ���ʧЧ��Do Not Disturb
    SYSTEM_STATE_GREEN_MODE,                    // ��ɫģʽ��ֻ����ʱ���ܣ��������йص����������˶���¼�ȣ�
    SYSTEM_STATE_SAVE_POWER_MODE,               // ʡ��ģʽ���ر�����������
    SYSTEM_STATE_LOWPOWER,                      // �͵�״̬��ֻ����ʱ����(��������ָʾ����)��Ч
    SYSTEM_STATE_DFU,                           // DFU ״̬���� DFU ģʽ��׼�� OTA ����
    SYSTEM_STATE_RESUME_F_GREEN,                // ����ɫģʽ�ָ�
    SYSTEM_STATE_RESUME_F_SAVE,                 // ��ʡ��ģʽ�ָ�
    SYSTEM_STATE_RESET,                         // ��λ״̬��ϵͳ��λ����ִ�ģʽ
    SYSTEM_STATE_STORE_RESET_REASON,             // 
};

// EVT_SYSTEM_BT_CONN_STATE ble_conn_state
enum system_ble_conn_state_event_e
{
    BT_CONN_STATE_INIT,                         // ������ʼ��
    BT_CONN_STATE_DISABLED,                     // �ر�����
//    BT_CONN_STATE_FAKE_ADV,                     // �ٹ㲥������״̬�ر����������û��鿴���ǹ㲥״̬��
    BT_CONN_STATE_DISCONNECTED,                 // �Ͽ�����
    BT_CONN_STATE_ADVERTISING_BONDABLE,         // �ɰ󶨹㲥
    BT_CONN_STATE_ADVERTISING,                  // �����㲥
    BT_CONN_STATE_CONNECTED,                    // ������
    BT_CONN_STATE_SECURED,                      // �Ѽ���
};

// EVT_BUTTON_PRESSED button_event
enum button_event_e
{
    BUTTON_EVT_BT_EN_SWTICH,              // �л� BLE ʹ��״̬
    BUTTON_EVT_CHECK_BATT_LEVEL,
    BUTTON_EVT_CHECK_BT_STATE,                  // �������״̬
    BUTTON_EVT_PHOTO_REQ,                       // ��������
    BUTTON_EVT_FIND_PHONE,                      // �����ֻ�
    BUTTON_EVT_REJECT_CALLIN,                   // �ܽ�����
    BUTTON_EVT_ACCEPT_CALLIN,                   // ��������
    BUTTON_EVT_INDICATE_CALLIN,                 // ָʾ����
    BUTTON_EVT_INDICATE_WEEK,                   // ָʾ����
    BUTTON_EVT_INDICATE_DAY,                    // ָʾ����
    BUTTON_EVT_INDICATE_STEPS_GOAL,             // ָʾ�˶���ɶ�
    BUTTON_EVT_INDICATE_NEW_MSG,                // ָʾ����Ϣ
    BUTTON_EVT_ENTER_STORAGE_STATE,             // ����ִ�״̬
    BUTTON_EVT_AUTH_OK,                         // ��Ȩ�ɹ�
    BUTTON_EVT_WATCH_HAND_STEP_FORWARD,         // ����һ��
    BUTTON_EVT_WATCH_HAND_STEP_BACKWARD,        // ����һ��
    BUTTON_EVT_START_DFU,                       // ��ʼ DFU 
    BUTTON_EVT_BT_DISABLE,
    BUTTON_EVT_SYSTEM_RESUME_F_GREEN,           //����ɫģʽ�ָ�����ģʽ
    BUTTON_EVT_SYSTEM_RESUME_F_SAVE,            //��ʡ��ģʽ�ָ�����ģʽ   
    BUTTON_EVT_SYSTEM_RESET,

    BUTTON_EVT_CLEAR_RESET_INFO, // 20
    BUTTON_EVT_BTN0_SHORT_REL,
    BUTTON_EVT_BTN1_SHORT_REL,
    BUTTON_EVT_BTN2_SHORT_REL,
    
    BUTTON_EVT_COMPASS,    //ָʾָ����־
    BUTTON_EVT_ADJUST_COMPASS,
    BUTTON_EVT_EXIT_COMPASS,
    BUTTON_EVT_INVALID = 0xFFFF,
};

// EVT_COMS_APP_RECV app_recv_event
enum app_recv_event_e
{
    APP_RECV_EVT_LINKLOSS_ENABLED_CONFIG,       // ���ӶϿ�����ʹ������
    APP_RECV_EVT_AUTH_ENTER_MODE,               // ������Ȩģʽ
    APP_RECV_EVT_AUTH_EXIT_MODE,                // �˳���Ȩģʽ
    APP_RECV_EVT_AUTH_FORCE_OK,                 // ǿ����Ȩ�ɹ�
    APP_RECV_EVT_IOS_PAIRING_REQ,                // ios �������
    APP_RECV_EVT_MSG_NEW,                       // ����Ϣ
    APP_RECV_EVT_MSG_CANCEL,                    // ��Ϣ��ȡ��
    APP_RECV_EVT_PHOTO_ENTER_MODE,              // ��������ģʽ
    APP_RECV_EVT_PHOTO_EXIT_MODE,               // �˳�����ģʽ
    APP_RECV_EVT_PHOTO_MODE_HEART,              // ����ģʽ������Ϣ
    APP_RECV_EVT_WATCH_HAND_ENTER_MODE,         // ����У��ģʽ
    APP_RECV_EVT_WATCH_HAND_SET_PARAM,          // ����У�����
    APP_RECV_EVT_WATCH_HAND_EXIT_MODE,          // �˳�У��ģʽ
    APP_RECV_EVT_WATCH_HAND_STEP_FORWARD,       // ����ǰ��(��ת)һ��
    APP_RECV_EVT_WATCH_HAND_STEP_BACKWARD,      // �������(��ת)һ��
    APP_RECV_EVT_WATCH_HAND_STOP,               // ������ͣ
    APP_RECV_EVT_WATCH_HAND_RESUME,             // ����ָ� 
    APP_RECV_EVT_WATCH_HAND_CHECK,              // ��ʼУ��
    APP_RECV_EVT_LONG_SIT_START,                // �������ѹ�������
    APP_RECV_EVT_LONG_SIT_STOP,                 // �������ѹ��ܹر�
    APP_RECV_EVT_LONG_SIT_PARAM_UPDATE,         // �������Ѳ�������
    APP_RECV_EVT_DND_START,                     // ���Ź�������
    APP_RECV_EVT_DND_STOP,                      // ���Ź��ܹر�
    APP_RECV_EVT_DND_PARAM_UPDATE,              // ���Ų�������
    APP_RECV_EVT_DAILY_SPORT_STEPS_GOAL,        // �˶�Ŀ������
    APP_RECV_EVT_SPEC_EVENT_REMIND,             // �����¼�����
    APP_RECV_EVT_UV_MEASURE_START,              // ����UV����
    APP_RECV_EVT_GET_BATT_VOL,                  // ��ȡ��ص���
    APP_RECV_EVT_INCALL,
    APP_RECV_EVT_INVALID = 0xFFFF,
};

// EVT_COMS_PROT_EXCU
enum prot_execution_event_e
{
    PROT_EXEC_EVT_ALARM_TRIGGERED,              // ���Ӵ���
    PROT_EXEC_EVT_LINKLOSS_TRIGGERED,           // ���ӶϿ�����
};

enum test_event_e
{
    TEST_EVT_FLASH_READ_FAKE_DATA,              //��flash����˯�ߺͼƲ�����
    TEST_EVT_FLASH_WRITE_FAKE_DATA,             //д��ٵ�˯�ߺͼƲ�����
    TEST_EVT_WATCH_HAND_FORWARD,                         //ָ����ת
    TEST_EVT_WATCH_HAND_BACKWARD,                        //ָ�뷴ת
  
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
