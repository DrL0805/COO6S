
#ifndef PROTOCOL_SENSOR_DATA_H__
#define PROTOCOL_SENSOR_DATA_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef struct
{
    uint16_t                             vis_measure_data;
    uint16_t                             ir_measure_data;
    uint16_t                             uv_measure_data;
}uv_measure_data_evt_t;

typedef union
{
    uv_measure_data_evt_t                measure_data;
    uint8_t                              uv_measure_index;
}prot_sensor_data_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_PACK_TRANS_H__ */


