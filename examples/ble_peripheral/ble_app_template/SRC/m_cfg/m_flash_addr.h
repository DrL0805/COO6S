#ifndef __M_FLASH_ADDR_H__
#define __M_FLASH_ADDR_H__

#define MAC_ADDR        0x00010000 // MAC读取地址
#define CORE_ID_ADDR    0x00010006 // 机芯读取地址
#define CUS_ADDR        0x00010086 // 成表sn读取地址
#define PROJ_ID_ADDR    0x00010107 // 项目编号读取地址

#define RESET_ADDR      0x00014000 // 保存复位信息(预留4KB == 0x1000)

#define SPORT_ORIGIN_ADDR				(RESET_ADDR + 4096)

#define SPORT_TIME_START_ADD         	(0x40000)	// 运动时间开始地址
#define SPORT_DIRECTORY_START_ADD       (0x41000)	// 运动目录开始地址
#define SPORT_DATA_START_ADD            (0x42000)	// 运动数据开始地址

#define SPORT_DIRECTORY_END_ADD         (0x41000 + 0x1000)	// 运动目录结束地址
#define SPORT_DATA_END_ADD              (0x42000 + 0x1000)	// 运动数据结束地址

#define SLEEP_TIME_START_ADD            (0x60000)		// 睡眠时间开始地址
#define SLEEP_DIRECTORY_START_ADD       (0x61000)		// 睡眠目录开始地址
#define SLEEP_DATA_START_ADD            (0x62000)		// 睡眠数据开始地址


#define SLEEP_DIRECTORY_END_ADD         (0x61000 + 0x1000)	// 睡眠目录结束地址
#define SLEEP_DATA_END_ADD              (0x62000 + 0x1000)	// 睡眠数据结束地址


/******************************自定义******************************/
#define BODY_DATA_SIZE					(8)
#define BODY_DATA_MAX_SIZE				(1536)	//1536 * 8 = 12KB，预留12KB
#define BODY_TEMP_ORIGIN_ADDR			(0x0)										// 关于体温数据的起始数据与结束地址在缓存区的备份地址
#define BODY_TEMP_DATA_HEAD_ADDR		(BODY_TEMP_ORIGIN_ADDR + 0x1000)				// 体温数据预留缓存区的起始地址
#define BODY_TEMP_DATA_TAIL_ADDR		(BODY_TEMP_DATA_HEAD_ADDR + 12 * 1024)	// 体温数据预留缓存区的结束地址，预留12KB


#define ENVIR_DATA_SIZE					(15)	
#define ENVIR_DATA_MAX_SIZE				(819)		//819 * 15 = 11.99KB 
#define ENVIR_ORIGIN_ADDR				((((BODY_TEMP_DATA_TAIL_ADDR - 1) >> 12) + 1) << 12)					// 关于环境参数的起始地址与结束地址在缓存区的备份地址
#define ENVIR_DATA_HEAD_ADDR			(ENVIR_ORIGIN_ADDR + 0x1000)					// 环境参数预留缓存区的起始地址
#define ENVIR_DATA_TAIL_ADDR			(ENVIR_DATA_HEAD_ADDR + 12 * 1024)		// 环境参数预留缓存区的结束地址，预留12KB


#endif
