
#ifndef __APP_TIME_SLOT_H__
#define __APP_TIME_SLOT_H__


#define TIME_SLOT_MINUTES(HOUR, MINUTE) (((HOUR) * 60) + (MINUTE))


typedef struct
{
    uint16_t begin;         // ������, ��ʼʱ��
    uint16_t end;           // ������, ����ʱ��
    uint16_t dnd_begin;     // ������, ���ſ�ʼʱ��(����ʱ�䷶ΧӦ�ڿ�ʼ������ʱ�䷶Χ֮��)
    uint16_t dnd_end;       // ������, ���Ž���ʱ��(����ʱ�䷶ΧӦ�ڿ�ʼ������ʱ�䷶Χ֮��)
}time_slot_minutes_t;

bool time_slot_is_minute_time_valid(time_slot_minutes_t time_slot, uint16_t cur_minutes);


#endif  /** __APP_TIME_SLOT_H__ */


