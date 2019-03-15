
#ifndef __APP_ALARM_CLOCK_H__
#define __APP_ALARM_CLOCK_H__

#define APP_ALARM_CLOCK_COUNT_MAX      3
#include "stdint.h"

#pragma pack(1)

typedef struct
{
    uint8_t valid :    4;               // 闹钟有效性，为 0 表示该闹钟已被删除；1 为有效
    uint8_t enabled :  4;               // 闹钟开关，为 0 表示关闭闹钟；1 为开
}ac_valid_t;

typedef struct
{
    uint8_t weeks :    7;               // 闹钟星期表，bit 6 ~ bit0 分别表示星期一到星期天
    uint8_t repeat:    1;               // 闹钟重复，为 0 表示为单次闹钟，为 1 则依据后面星期表的配置进行提醒
}ac_week_t;

typedef struct
{
	uint8_t             num;            // 闹钟序号
    ac_valid_t          valid;          // 闹钟有效性
    uint8_t             hour;           // 闹钟触发时间: 小时
    uint8_t             minute;         // 闹钟触发时间: 分钟
    ac_week_t           week;           // 闹钟触发时间: 星期表
}app_alarm_clock_param_t;

#pragma pack()


/* 闹钟触发时间参数，指针应该指向一个能够实时更新的数值 */
typedef struct
{
    uint8_t             *p_week;       // 0 ~ 6 表示星期日到星期六
    uint8_t             *p_hour;       // 24 小时制，从 0 时开始
    uint8_t             *P_minute;     // 分钟，从 0 开始
}app_alarm_clock_trigger_time_t;

typedef struct
{
    app_alarm_clock_param_t                *p_param_list;
    app_alarm_clock_trigger_time_t          trigger_time;
}app_alarm_clock_config_t;

typedef enum
{
    APP_ALARM_CLOCK_EVT_TRIGGERED,  // 闹钟已经触发
}app_alarm_clock_evt_id_t;

typedef struct
{
    app_alarm_clock_evt_id_t evt_id;
    struct
    {
        uint8_t              alarm_clock_index; // 闹钟序号
    }param;
}app_alarm_clock_evt_t;

typedef void(*app_alarm_clock_evt_handler_t)(app_alarm_clock_evt_t * p_evt);


uint32_t app_alarm_clock_init(app_alarm_clock_config_t * config, app_alarm_clock_evt_handler_t handler);

uint32_t app_alarm_clock_delete(uint8_t alarm_clock_num);

uint32_t app_alarm_clock_enable(void);

uint32_t app_alarm_clock_disable(void);

uint32_t app_alarm_clock_minute_process(void);


#endif  /** __APP_ALARM_CLOCK_H__ */


