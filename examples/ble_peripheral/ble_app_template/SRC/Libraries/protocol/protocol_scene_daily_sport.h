
#ifndef PROTOCOL_SCENE_DAILY_SPORT_H__
#define PROTOCOL_SCENE_DAILY_SPORT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef union
{
    uint8_t mode;
}prot_scene_ds_mode_evt_t;

typedef union
{
    uint16_t set_goal;
    uint16_t ret_goal;
}prot_scene_ds_goal_evt_t;

typedef union
{
    uint8_t nothing;
}prot_scene_ds_his_steps_evt_t;

typedef union
{
    uint8_t nothing;
}prot_scene_ds_his_steps_trans_evt_t;

typedef union
{
    prot_scene_ds_mode_evt_t                ds_mode;
    prot_scene_ds_goal_evt_t                ds_goal;
    uint16_t                                total_daily_steps;
    prot_scene_ds_his_steps_evt_t           his_steps;
    prot_scene_ds_his_steps_trans_evt_t     his_steps_trans;
}prot_scene_daily_sport_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_SCENE_DAILY_SPORT_H__ */


