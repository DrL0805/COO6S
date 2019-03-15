


#include "sdk_common.h"
#include "app_time_slot.h"

/* �ж�ʱ���Ƿ�����Чʱ����� */
bool time_slot_is_minute_time_valid(time_slot_minutes_t time_slot, uint16_t cur_minutes)
{
    bool valid_time = false;

    if (time_slot.begin <= time_slot.end)
    {
        if ((cur_minutes >= time_slot.begin) && (cur_minutes <= time_slot.end)) // ��Чʱ�����
        {
            valid_time = true;
        }
    }
    else
    {
        if (! ((cur_minutes >= time_slot.end) && (cur_minutes <= time_slot.begin)) ) // ��Чʱ�����
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
        if ((cur_minutes >= time_slot.dnd_begin) && (cur_minutes <= time_slot.dnd_end)) // ����ʱ�����
        {
            return false;
        }
    }
    else
    {
        if (!((cur_minutes >= time_slot.dnd_end) && (cur_minutes <= time_slot.dnd_begin))) // ����ʱ�����
        {
            return false;
        }
    }

    /* ��Чʱ����ڣ�����ʱ����� */
    return true;
}






































