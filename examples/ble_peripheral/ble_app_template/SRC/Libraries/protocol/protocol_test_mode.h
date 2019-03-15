
#ifndef PROTOCOL_TEST_MODE_H__
#define PROTOCOL_TEST_MODE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)



typedef union
{
    struct
    {  
        uint8_t test_mode_en;
    }call_param;
    struct
    {
        uint8_t test_mode_en;
        uint8_t result;
    }ret_param;
}prot_test_mode_en_evt_t;


typedef union
{
    struct
    {  
        uint8_t auto_test_en;
    }call_param;
    struct
    {
        uint8_t test_item;
        uint8_t test_submodule;
        uint8_t result;
    }echo_param;
}prot_test_mode_auto_test_evt_t;



typedef union
{
    struct
    {  
        uint8_t sub_test_en;
        uint8_t test_item;
        uint8_t test_submodule;
    }call_param;
    struct
    {
        uint8_t test_item;
        uint8_t test_submodule;
        uint8_t test_result;        
    }echo_param;
}prot_test_mode_sub_test_evt_t;


typedef union
{
    struct
    {
        uint8_t pack_index;
        uint8_t test_list[10];        
    }ret_param;
}prot_test_mode_test_list_evt_t;

typedef union
{
    struct
    {
        uint8_t led_num;
        uint8_t state_switch;
        uint8_t switch_time;        
    }call_param;
}prot_test_mode_test_led_evt_t;

typedef union
{
    struct
    {
        uint8_t motor_en;      
    }call_param;
}prot_test_mode_test_motor_evt_t;

typedef union
{
    struct
    {
        uint8_t movt_id;
        uint8_t movt_action;
        uint8_t movt_speed;
        uint16_t movt_steps;       
    }call_param;
}prot_test_mode_test_movt_evt_t;

typedef union
{
    struct
    {
        uint8_t key_id;
        uint32_t key_times;       
    }echo_param;
}prot_test_mode_test_key_evt_t;



typedef union
{
    prot_test_mode_en_evt_t                test_mode_en;    
    prot_test_mode_auto_test_evt_t         test_mode_auto_test;
    prot_test_mode_sub_test_evt_t          test_mode_sub_test;  
    prot_test_mode_test_list_evt_t         test_mode_test_list;
    prot_test_mode_test_led_evt_t          test_mode_test_led;
    prot_test_mode_test_motor_evt_t        test_mode_test_motor;
    prot_test_mode_test_movt_evt_t         test_mode_test_movt;
    prot_test_mode_test_key_evt_t          test_mode_test_key;
}prot_test_mode_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_SCENE_DAILY_SPORT_H__ */


