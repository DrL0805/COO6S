#ifndef __M_DEBUG_H__
#define __M_DEBUG_H__

#include <stdio.h>
#include <string.h>
#include "SEGGER_RTT_Conf.h"
#include "SEGGER_RTT.h"
#include "sr3_config.h"



#define GPREGRET_ID		(0)
#define GPREGRET2_ID	(1)

#define MCU_MODE_DFU_BIT_MASK			0x01	//��DFU����
#define MCU_MODE_LOWPOWER_BIT_MASK		0x02	//�͵�ģʽ
#define MCU_MODE_CHARGE_BIT_MASK		0x04	//���ģʽ
#define MCU_MODE_STORAGE_BIT_MASK		0x08	//�ִ�ģʽ
#define MCU_MODE_NORMAL_BIT_MASK		0x10	//��������ģʽ


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
    SYS_STATE_STORAGE = 0,      //仓储模式
    SYS_STATE_LOWPOWER,         //电量过低
    SYS_STATE_BT_ON,            //蓝牙开�?
    SYS_STATE_BT_OFF,           //蓝牙关闭
}sys_state_t;

#endif

