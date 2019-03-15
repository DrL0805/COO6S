


#include "sdk_common.h"
#include "app_time_slot.h"

/* 判断时间是否在有效时间段内 */
bool time_slot_is_minute_time_valid(time_slot_minutes_t time_slot, uint16_t cur_minutes)
{
    bool valid_time = false;

    if (time_slot.begin <= time_slot.end)
    {
        if ((cur_minutes >= time_slot.begin) && (cur_minutes <= time_slot.end)) // 有效时间段内
        {
            valid_time = true;
        }
    }
    else
    {
        if (! ((cur_minutes >= time_slot.end) && (cur_minutes <= time_slot.begin)) ) // 有效时间段内
        {
            valid_time = true;
        }
    }

    if (!valid_time)
    {
        return false;
    }

    if (time_slot.dnd_begin <= time_slot.dnd_end)
    {
        if ((cur_minutes >= time_slot.dnd_begin) && (cur_minutes <= time_slot.dnd_end)) // 勿扰时间段内
        {
            return false;
        }
    }
    else
    {
        if (!((cur_minutes >= time_slot.dnd_end) && (cur_minutes <= time_slot.dnd_begin))) // 勿扰时间段内
        {
            return false;
        }
    }

    /* 有效时间段内，勿扰时间段外 */
    return true;
}






































