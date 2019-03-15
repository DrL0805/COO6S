
#ifndef __APP_ALARM_CLOCK_H__
#define __APP_ALARM_CLOCK_H__

#define APP_ALARM_CLOCK_COUNT_MAX      3
#include "stdint.h"

#pragma pack(1)

typedef struct
{
    uint8_t valid :    4;               // ������Ч�ԣ�Ϊ 0 ��ʾ�������ѱ�ɾ����1 Ϊ��Ч
    uint8_t enabled :  4;               // ���ӿ��أ�Ϊ 0 ��ʾ�ر����ӣ�1 Ϊ��
}ac_valid_t;

typedef struct
{
    uint8_t weeks :    7;               // �������ڱ�bit 6 ~ bit0 �ֱ��ʾ����һ��������
    uint8_t repeat:    1;               // �����ظ���Ϊ 0 ��ʾΪ�������ӣ�Ϊ 1 �����ݺ������ڱ�����ý�������
}ac_week_t;

typedef struct
{
	uint8_t             num;            // �������
    ac_valid_t          valid;          // ������Ч��
    uint8_t             hour;           // ���Ӵ���ʱ��: Сʱ
    uint8_t             minute;         // ���Ӵ���ʱ��: ����
    ac_week_t           week;           // ���Ӵ���ʱ��: ���ڱ�
}app_alarm_clock_param_t;

#pragma pack()


/* ���Ӵ���ʱ�������ָ��Ӧ��ָ��һ���ܹ�ʵʱ���µ���ֵ */
typedef struct
{
    uint8_t             *p_week;       // 0 ~ 6 ��ʾ�����յ�������
    uint8_t             *p_hour;       // 24 Сʱ�ƣ��� 0 ʱ��ʼ
    uint8_t             *P_minute;     // ���ӣ��� 0 ��ʼ
}app_alarm_clock_trigger_time_t;

typedef struct
{
    app_alarm_clock_param_t                *p_param_list;
    app_alarm_clock_trigger_time_t          trigger_time;
}app_alarm_clock_config_t;

typedef enum
{
    APP_ALARM_CLOCK_EVT_TRIGGERED,  // �����Ѿ�����
}app_alarm_clock_evt_id_t;

typedef struct
{
    app_alarm_clock_evt_id_t evt_id;
    struct
    {
        uint8_t              alarm_clock_index; // �������
    }param;
}app_alarm_clock_evt_t;

typedef void(*app_alarm_clock_evt_handler_t)(app_alarm_clock_evt_t * p_evt);


uint32_t app_alarm_clock_init(app_alarm_clock_config_t * config, app_alarm_clock_evt_handler_t handler);

uint32_t app_alarm_clock_delete(uint8_t alarm_clock_num);

uint32_t app_alarm_clock_enable(void);

uint32_t app_alarm_clock_disable(void);

uint32_t app_alarm_clock_minute_process(void);


#endif  /** __APP_ALARM_CLOCK_H__ */


