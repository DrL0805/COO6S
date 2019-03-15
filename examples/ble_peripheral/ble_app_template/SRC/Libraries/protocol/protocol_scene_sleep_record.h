
#ifndef PROTOCOL_SCENE_SLEEP_RECORD_H__
#define PROTOCOL_SCENE_SLEEP_RECORD_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)



typedef union
{
    uint8_t sleep_histrory_index_num;
}prot_scene_sleep_history_index_evt_t;


typedef union
{
    uint8_t sleep_histrory_index;
    uint8_t wake_num;
    uint32_t sleep_start;
    uint32_t sleep_stop;
}prot_scene_sleep_history_record_evt_t;

typedef union
{
    uint8_t sleep_histrory_index;
    uint8_t detail_index;
    uint32_t wake_time;
    uint32_t sleep_time;
}prot_scene_sleep_history_detail_record_evt_t;

typedef union
{
    struct
    {  
        uint8_t pack_index;
    }get_param;
    struct
    {
        uint8_t wake_times;
        uint32_t sleep_utc_start;
        uint32_t sleep_utc_stop;
    }ret_param;
    struct
    {
        uint8_t detail_index;
        uint32_t wake_up_utc;
        uint32_t sleep_utc;
    }echo_param;
}prot_scene_sleep_history_pkg_evt_t;

typedef union
{
    struct
    {
        uint32_t record_utc;
    }call_param;
    struct
    {
        uint32_t record_utc;
        uint8_t result;
    }echo_param;
}prot_scene_sleep_history_del_evt_t;


typedef union
{
    prot_scene_sleep_history_index_evt_t                sleep_history_index;    
    prot_scene_sleep_history_record_evt_t               sleep_history_record;
    prot_scene_sleep_history_pkg_evt_t                  sleep_history_pkg;  
    prot_scene_sleep_history_del_evt_t                  sleep_history_del;
}prot_scene_sleep_record_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_SCENE_DAILY_SPORT_H__ */


