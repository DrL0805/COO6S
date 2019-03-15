
#ifndef PROTOCOL_SENSOR_TRANS_H__
#define PROTOCOL_SENSOR_TRANS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

//#define DATA_TYPE_STEPS                             0x0000
//#define DATA_TYPE_SLEEP                             0x0001
//#define DATA_TYPE_END                               0x0002

#define DATA_INTV_1US                               1
#define DATA_INTV_1MS                               2
#define DATA_INTV_1S                                3
#define DATA_INTV_10US                              4
#define DATA_INTV_10MS                              5
#define DATA_INTV_10S                               6
#define DATA_INTV_1MINUTE                           7
#define DATA_INTV(VALUE, UNIT)                      (((UNIT) << 12) + ((VALUE) & 0x0FFF))

#define DATAREQ_RESULT_SUCCESS                      0
#define DATAREQ_RESULT_INVALID_DATATYPE             1
#define DATAREQ_RESULT_INVALID_CONTENT_INDEX        2
#define DATAREQ_RESULT_INVALID_PACK_INDEX           3
#define DATAREQ_RESULT_INTERNAL_ERROR               4


#define DELETE_SUCCESS                              0                   // 删除成功
#define DELETE_ERROR_INTERNAL                       1                   // 删除失败(内部错误)
#define DELETE_NO_PERMISSION                        2                   // 删除失败(没有权限)

typedef union
{
	uint16_t		temp;
}prot_ir_temp_evt_t;

typedef union
{
	uint16_t		temp;
}prot_body_temp_evt_t;

typedef union
{
	uint32_t					data;
	struct
	{
		int16_t		temp;
		uint16_t	press;
	} pressure;
}prot_pressure_temp_evt_t;

typedef union
{
	prot_ir_temp_evt_t						ir_data;
	prot_body_temp_evt_t					body_data;
	prot_pressure_temp_evt_t					pressure_data;
}prot_sensor_trans_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_PACK_TRANS_H__ */


