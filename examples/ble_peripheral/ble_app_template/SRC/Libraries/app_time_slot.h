
#ifndef __APP_TIME_SLOT_H__
#define __APP_TIME_SLOT_H__


#define TIME_SLOT_MINUTES(HOUR, MINUTE) (((HOUR) * 60) + (MINUTE))


typedef struct
{
    uint16_t begin;         // 分钟数, 开始时间
    uint16_t end;           // 分钟数, 结束时间
    uint16_t dnd_begin;     // 分钟数, 勿扰开始时间(勿扰时间范围应在开始、结束时间范围之内)
    uint16_t dnd_end;       // 分钟数, 勿扰结束时间(勿扰时间范围应在开始、结束时间范围之内)
}time_slot_minutes_t;

bool time_slot_is_minute_time_valid(time_slot_minutes_t time_slot, uint16_t cur_minutes);


#endif  /** __APP_TIME_SLOT_H__ */


