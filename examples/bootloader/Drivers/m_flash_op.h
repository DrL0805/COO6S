#ifndef __M_FLASH_OP_H__
#define __M_FLASH_OP_H__

#include <stdint.h>

//#define FLASH_OLD
//#define FLASH_NEW
//#define FLASH_TEST
#include "nrf52.h"
#include "sdk_errors.h"
#include "sr3_config.h"

#define MARK_STRING		"MOVT"

#define INDEX_LOW		0
#define INDEX_TOP		1
#define INDEX_MAX		2
#define IDLE_BYTE_DATA		(0xFF)
#define IDLE_2BYTE_DATA		(0xFFFF)
#define IDLE_3BYTE_DATA		(0xFFFFFF)
#define IDLE_4BYTE_DATA		(0xFFFFFFFF)

#define DIR_LIST_SIZE	(4)		// 最多保留6个有效目录

// 一个目录在flash占的空间大小
#define DIR_SIZE				sizeof(flash_op_dir_t)	

// 创建目录时，写入的数据大小；
// 结束地址在重新创建新目录时写入
// 是否删除标志在上传完数据后，听从APP处置
#define DIR_BEYOND_SIZE			(DIR_SIZE - 5)	

#define DIR_VALID				(0xFF)
#define DIR_INVALID				(0x00)

// 主要用于结束当前目录
typedef void (*pFun)(void);


//#define __OIO	volatile
#define __OIO	

/* 区分此次检测是上报朝廷，还是中饱私囊 */
typedef enum
{
	CHECK_UPLOAD,
	CHECK_STORE_FLASH,
}check_type_t;

// 采样的单位参数
typedef enum
{
	SAMPLE_UNIT_NONE = 0,
	SAMPLE_UNIT_1US,
	SAMPLE_UNIT_1MS,
	SAMPLE_UNIT_1S,
	SAMPLE_UNIT_10US,
	SAMPLE_UNIT_10MS,
	SAMPLE_UNIT_10S,
	SAMPLE_UNIT_MAX,
}sample_unit_t;



//// 数据类型
//typedef enum
//{
//	DATA_TYPE_STEPS = 0x00,
//	DATA_TYPE_SLEEP,
//	DATA_TYPE_BODY,
//	DATA_TYPE_PRESSURE,
//	DATA_TYPE_TH,
//	DATA_TYPE_MAX,
//}data_type_t;

// 数据存储结构类型，与数据类型不等，但是一一对应
typedef enum
{
	#if CONFIG_GSENSOR_ENABLED
	STORE_INDEX_STEPS,		// 计步数据在数组中的下标
	STORE_INDEX_SLEEP,		// 睡眠数据在数组中的下标
	#endif
	
	#if CONFIG_BODY_TEMPERATOR_ENABLED
	STORE_INDEX_BODY,
	#endif
	
	#if CONFIG_PRESSURE_ENABLED
	STORE_INDEX_PRESSURE,
	#endif
	
	#if CONFIG_TEMPERATURE_HUMIDITY_ENABLED
	STORE_INDEX_TH,
	#endif
	
	STORE_INDEX_MAX,
}store_type_t;



typedef enum
{
	DIR_FIND_FIRST = 0,
	DIR_FIND_NO_FIRST,
}dir_find_times_t;



typedef struct
{
	uint16_t	len;		// 数据的长度
	void		*data;		// 真实数据
}flash_data_t;


typedef struct
{
	__OIO uint16_t	sample_interval:12;	// 采样间隔					12bit -> 4 + 12 = 16bit = 2Byte
	__OIO uint16_t	sample_unit:4;		// 采样单位					4bit
}flash_op_sample_t;



typedef struct  // 16Byte
{
	__OIO uint32_t				utc; 				// 时间戳					4Byte
	
	__OIO flash_op_sample_t 	sample;				// 采样参数：单位与间隔		2Byte
	__OIO uint8_t				frame_len;			// 一帧数据的长度
	__OIO uint8_t				reserve;			// 预留						2Byte	
	
	__OIO uint32_t				start_addr:24;		// 数据段的首地址			3Byte
	__OIO uint32_t				valid:8;			// 包头是否有效，可能被删除	1Byte
	
	__OIO uint32_t				end_addr;			// 数据段的结束地址			4Byte
	
}flash_op_dir_t;

typedef struct  // 16Byte
{
	__OIO uint32_t				utc; 				// 时间戳	4Byte	
    
	__OIO flash_op_sample_t 	sample;				// 采样参数：单位与间隔
    __OIO uint16_t				dir_offset;			// 该目录所处的偏移地址
    
    
    __OIO uint32_t            	frame_len:8;
	__OIO uint32_t				start_addr:24;			// 数据段的首地址			3Byte
    
    __OIO uint32_t            	valid:8;
	__OIO uint32_t				end_addr:24;			// 数据段的结束地址			4Byte

}flash_dir_list_t;



typedef struct
{
	__OIO flash_op_dir_t		dir;			    // 当前目录表头
    
	__OIO uint32_t				write;				// 已经写到flash的某个地址	3Byte	
//	__OIO uint32_t				read;				// 已经读到flash的某个地址	4Byte
	__OIO uint32_t				origin;				// 表头所处内存块的首地址
	__OIO uint32_t				head;				// 缓存区地址头
	__OIO uint32_t				tail;				// 缓存区地址尾
	__OIO uint32_t				dir_offset;			// 下一个要写入的目录
    __OIO uint32_t              list_travel_offset:24;    // 下一个要遍历的目录    
    __OIO uint32_t              list_total:8;       // 目前已记录的目录数量
//	__OIO uint32_t				dir_end;		    // 表头有效数据所处的偏移地址
//	__OIO uint32_t				data_len;			// 当前目录已写入的数据长度，单位：帧

	__OIO flash_dir_list_t		list[DIR_LIST_SIZE];// 存放记录的有效目录
	__OIO uint32_t				data_offset;		// 数据写入的最后地址，保存在表头的偏移量
	__OIO uint16_t				frame_len;			// 当前一帧数据的长度
	__OIO flash_op_sample_t		sample;				// 当前采样参数
	
	__OIO uint8_t				init_flag;			// 是否已经初始化
	__OIO uint8_t				dir_ready;			// 是否已经新建好目录，写数据之前必须有目录
    __OIO uint8_t               reserve0;  			// 预留
	__OIO uint16_t				reserve1;			// 预留
//	__OIO dir_fifo_t			dir_fifo;

//	__OIO dir_record_t			dir_record_t[]
}flash_op_t;


extern flash_op_t	*flash_op[STORE_INDEX_MAX];
extern pFun 		DirClose[STORE_INDEX_MAX];

ret_code_t flash_dir_clear(flash_op_t *op, uint16_t offset);

ret_code_t flash_dir_find_no_first(flash_op_t *op);

ret_code_t flash_dir_list_insert(flash_op_t *op, flash_op_dir_t *dir, uint16_t offset);

ret_code_t flash_dir_list_read_all(flash_op_t *op, flash_op_dir_t *pdir, uint8_t *plen);

ret_code_t flash_dir_list_read_utc(flash_op_t *op, uint32_t utc);

ret_code_t flash_dir_list_offset_to_index(flash_op_t *op, uint32_t offset);

ret_code_t flash_dir_list_del_utc(flash_op_t *op, uint32_t utc);

ret_code_t flash_dir_list_del_all(flash_op_t *op);

ret_code_t flash_dir_list_num_get(flash_op_t *op);

ret_code_t flash_dir_list_get_size_single(flash_op_t *op, uint8_t index);

ret_code_t flash_dir_list_get_size_all(flash_op_t *op);

ret_code_t flash_dir_list_init(flash_op_t *op);

ret_code_t flash_table_find_finally_data_offset(flash_op_t *op);

ret_code_t flash_table_create(flash_op_t *op);

ret_code_t flash_table_insert(flash_op_t *op);

//ret_code_t flash_dir_list_endaddr_write(flash_op_t *op);

ret_code_t flash_dir_write_finaladdr(flash_op_t *op);

ret_code_t flash_dir_init(flash_op_t *op);

ret_code_t flash_dir_insert(flash_op_t *op);

ret_code_t flash_dir_find_first(flash_op_t *op);

ret_code_t flash_table_init(flash_op_t *op);

ret_code_t flash_op_init(flash_op_t *op);

ret_code_t flash_data_insert(flash_op_t *op, flash_data_t *flash_data);

ret_code_t flash_dir_list_travel(flash_op_t *op);

uint8_t	flash_arrayIndex_to_dataType(uint8_t index);

uint8_t	flash_dataType_to_arrayIndex(uint8_t type);

void DirCloseAll(void);

// 仅仅用于测试
ret_code_t flash_init(void);

void flash_body_write_test(void);

#endif
