#ifndef __M_DEBUG_H__
#define __M_DEBUG_H__

#include <stdio.h>
#include <string.h>
#include "SEGGER_RTT_Conf.h"
#include "SEGGER_RTT.h"
#include "sr3_config.h"



#define GPREGRET_ID		(0)
#define GPREGRET2_ID	(1)

#define MCU_MODE_DFU_BIT_MASK			0x01	//从DFU开机
#define MCU_MODE_LOWPOWER_BIT_MASK		0x02	//低电模式
#define MCU_MODE_CHARGE_BIT_MASK		0x04	//充电模式
#define MCU_MODE_STORAGE_BIT_MASK		0x08	//仓储模式
#define MCU_MODE_NORMAL_BIT_MASK		0x10	//正常工作模式


#ifndef BOOTLOADER 
#include "m_database.h"

extern system_database_t system_database;
#endif


#if CONFIG_RTT_DEBUG_ENABLED

#ifndef NRF_LOG_USES_RTT
#define NRF_LOG_USES_RTT	1
#endif


#define LOG_DRAW_Print(...)     SEGGER_RTT_printf(0, ##__VA_ARGS__)


#define SHORT_FILE 	strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : (__FILE__)

#ifdef TICKS_PRINT
//#define SEGGER_Print(N, M, ...)		do {	\
//	SEGGER_RTT_printf(0, "[%.4d-%.2d-%.2d %.2d:%.2d:%.2d][%s:%d (%s) ] " M " \n\r", system_database.utc.year, system_database.utc.month, system_database.utc.day,	\
//								system_database.utc.hour, system_database.utc.minute, system_database.utc.second, SHORT_FILE, __LINE__, __func__, ##__VA_ARGS__);	\
//} while(0)

#if 1
#define SEGGER_Print(N, M, ...)		do {	\
	SEGGER_RTT_printf(0, "[%s:%d] " M " \n\r", __func__, __LINE__, ##__VA_ARGS__);	\
} while(0)

#else
#define SEGGER_Print(N, M, ...)		do {	\
	SEGGER_RTT_printf(0, "[%.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d][%s:%d] " M " \n\r", system_database.utc.year, system_database.utc.month, system_database.utc.day,	\
								system_database.utc.hour, system_database.utc.minute, system_database.utc.second, ((NRF_RTC1->COUNTER & 0x7FFF) * 1000) >> 15, __func__, __LINE__, ##__VA_ARGS__);	\
} while(0)
#endif

#else
#define SEGGER_Print(N, M, ...)		do {	\
	SEGGER_RTT_printf(0, "[%s:%d (%s) ] " M " \n\r", SHORT_FILE, __LINE__, __func__, ##__VA_ARGS__);	\
} while(0)
#endif




#else
#define SEGGER_Print(...)
#define LOG_DRAW_Print(...)
#endif

__STATIC_INLINE void m_power_gpregret_set(uint8_t id, uint8_t mask)
{
	
	sd_power_gpregret_clr(id, 0xffffffff);
	sd_power_gpregret_set(id, mask);
	LOG_DRAW_Print("GPREGRET111: %d, 0x%x\r\n", id, NRF_POWER->GPREGRET2);
}

__STATIC_INLINE void m_power_gpregret_insert(uint8_t id, uint8_t mask)
{
	sd_power_gpregret_set(id, mask);
	LOG_DRAW_Print("GPREGRET222: %d, 0x%x\n\r", id, NRF_POWER->GPREGRET2);
}


typedef enum
{
    SYS_STATE_STORAGE = 0,      //浠撳偍妯″紡
    SYS_STATE_LOWPOWER,         //鐢甸噺杩囦綆
    SYS_STATE_BT_ON,            //钃濈墮寮�鍚?
    SYS_STATE_BT_OFF,           //钃濈墮鍏抽棴
}sys_state_t;

#endif

