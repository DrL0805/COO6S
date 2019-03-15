#ifndef __M_PACK_SEND_H__
#define __M_PACK_SEND_H__

#include <stdint.h>
#include "nrf52.h"
#include "sdk_errors.h"
#include "pack_fifo.h"
#include "m_drv_w25x80.h"
#include "m_flash_op.h"
#include "string.h"

#define PACK_FRAME_REMAIN_SIZE	(16)		
#define PACK_SEND_BUFFER_MAX	(512)

#define BODY_DATA		(0x0008)
#define ENVIR_DATA		(0x0009)

typedef struct
{
	uint8_t			buf[PACK_SEND_BUFFER_MAX];	// 缓存区
	uint16_t		read_pos;					// 读指针
	uint16_t		write_pos;					// 写指针 
	uint16_t		bytes_max;					// 缓存区可容纳最大帧数
}pack_data_t;

typedef struct
{
	pack_data_t		data;	// 缓存区
	
	uint32_t		head;		// 数据段的数据上限
	uint32_t		tail;		// 数据段的数据下限
	uint32_t		read;		// 读数据的起始点
	uint16_t		nframes;	// 需要读取数据的帧数
	uint16_t		frame_len;	// 每帧数据的长度
	uint16_t		pkg_sn;		// 序列号
	uint16_t		pkg_type;	// 数据类型
	
}pack_send_t;

//************************************************************************************************************
// 功能描述:   	循环发送数据的准备工作
// 函数参数:    type		->		数据类型
//				index		->		目录列表中的下标
//				offset		->		数据区帧数偏移量
// 返回值  :	操作结果
//************************************************************************************************************
ret_code_t	pack_data_init(uint8_t type, uint8_t	index, uint16_t	offset);

//************************************************************************************************************
// 功能描述:   	从Flash读取多帧数据到缓存区，准备发送
// 函数参数:    无
// 返回值  :	操作结果
//************************************************************************************************************
ret_code_t	pack_data_from_flash(void);


//************************************************************************************************************
// 功能描述:   	循环发送，从Flash读取数据，直到没有有效数据
// 函数参数:    无
// 返回值  :	操作结果
//************************************************************************************************************
ret_code_t	pack_send_loop(void);

#endif



