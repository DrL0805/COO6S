
#ifndef DRV_VIBRATOR_H__
#define DRV_VIBRATOR_H__

#include <stdbool.h>
#include <stdint.h>
#include "nrf_drv_common.h"
#include "sdk_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DRV_VIBRATOR_EVT_STARTED,
    DRV_VIBRATOR_EVT_DONE,
}drv_vibrator_evt_type_t;

typedef struct 
{
    uint16_t on;                // ms
    uint16_t off;               // ms
    uint16_t flash_count;
    uint16_t interval;          // ms
    uint16_t count;
}drv_vibrator_config_t;

typedef void(*drv_vibrator_event_handler_t)(drv_vibrator_evt_type_t evt, uint16_t time);


uint32_t drv_vibrator_init(drv_vibrator_event_handler_t handler);

uint32_t drv_vibrator_uninit(void);

uint32_t drv_vibrator_start(drv_vibrator_config_t p_config);

uint32_t drv_vibrator_stop(void);


#ifdef __cplusplus
}
#endif

#endif // DRV_VIBRATOR_H__

