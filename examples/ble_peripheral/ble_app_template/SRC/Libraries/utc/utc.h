
#ifndef UTC_H__
#define UTC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define BEGIN_YEAR          1970
#define BEGIN_WEEK          4       // 1970 年 1 月 1 号是星期四


typedef enum
{
    UTC_EVT_FIRST           = 0U,
    UTC_EVT_SECOND,
    UTC_EVT_MINUTE,
    UTC_EVT_HOUR,
    UTC_EVT_DAY,
    UTC_EVT_END,
}utc_evt_t;

//typedef struct
//{
//	uint32_t	body_utc;	// 创建目录的时间间隔
//}utc_alert_t;

typedef void (*utc_evt_handler_t)(utc_evt_t);

#pragma pack(1)

typedef struct
{
    uint8_t  week;      // 0~6: 星期天~星期六
    uint8_t  second;
    uint8_t  minute;
    uint8_t  hour;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;
}utc_time_t;

#pragma pack()


utc_time_t utc_get_cur_time(void);

uint32_t utc_get_cur_seconds(void);

void utc_set_cur_time(utc_time_t utc);

void utc_set_cur_seconds(uint32_t seconds);

uint32_t utc_init(utc_evt_handler_t handler);

uint32_t utc_enabled(void);

uint32_t utc_disabled(void);

utc_time_t second_to_utc(uint32_t sec);

uint32_t utc_to_second(utc_time_t utc);

uint8_t	second_to_week(uint32_t sec);

#ifdef __cplusplus
}
#endif

#endif /* UTC_H__ */


