
#ifndef DRV_WH_H__
#define DRV_WH_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf_drv_common.h"
#include "sdk_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRV_WH_FORWARD_STEP_INTV            18                  // ???????????λ? ms
#define DRV_WH_BACKWARD_STEP_INTV           35                  // ???????????λ? ms

#define DRV_WH_FORWARD_STEP_WAIT_INTV       12                   //推针波形之后增加一个间隔，防止两个波形间隔太短  18 > (5.3 + 12) > 1000/60



typedef enum
{
    DRV_WH_EVT_STARTED          = 0U,
    DRV_WH_EVT_STEP_FORWARD,
    DRV_WH_EVT_STEP_BACKWARD,
    DRV_WH_EVT_DONE,
}drv_wh_evt_type_t;



typedef enum
{
    DRV_WH_DIR_IDLE             = 0U,
    DRV_WH_DIR_FORWARD          = 1U,
    DRV_WH_DIR_BACKWARD         = 2U,
}drv_wh_dir_t;

typedef struct 
{
    uint8_t         watch_hand_id;
    uint16_t        steps;
    uint8_t         direction;          // @ drv_wh_dir_t
}drv_wh_config_t;

typedef void(*drv_wh_event_handler_t)(uint8_t watch_hand_id, drv_wh_evt_type_t);

uint32_t drv_wh_init(drv_wh_event_handler_t handler);

uint32_t drv_wh_start(drv_wh_config_t p_config);

uint32_t drv_wh_stop(uint8_t watch_hand_id);


bool drv_wh_get_active_pin(uint8_t);
void drv_wh_config_active_pin(bool ,bool);
#ifdef __cplusplus
}
#endif

#endif // DRV_WH_H__

