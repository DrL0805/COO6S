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

#define DIR_LIST_SIZE	(4)		// ��ౣ��6����ЧĿ¼

// һ��Ŀ¼��flashռ�Ŀռ��С
#define DIR_SIZE				sizeof(flash_op_dir_t)	

// ����Ŀ¼ʱ��д������ݴ�С��
// ������ַ�����´�����Ŀ¼ʱд��
// �Ƿ�ɾ����־���ϴ������ݺ�����APP����
#define DIR_BEYOND_SIZE			(DIR_SIZE - 5)	

#define DIR_VALID				(0xFF)
#define DIR_INVALID				(0x00)

// ��Ҫ���ڽ�����ǰĿ¼
typedef void (*pFun)(void);


//#define __OIO	volatile
#define __OIO	

/* ���ִ˴μ�����ϱ���͢�������б�˽�� */
typedef enum
{
	CHECK_UPLOAD,
	CHECK_STORE_FLASH,
}check_type_t;

// �����ĵ�λ����
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



//// ��������
//typedef enum
//{
//	DATA_TYPE_STEPS = 0x00,
//	DATA_TYPE_SLEEP,
//	DATA_TYPE_BODY,
//	DATA_TYPE_PRESSURE,
//	DATA_TYPE_TH,
//	DATA_TYPE_MAX,
//}data_type_t;

// ���ݴ洢�ṹ���ͣ����������Ͳ��ȣ�����һһ��Ӧ
typedef enum
{
	#if CONFIG_GSENSOR_ENABLED
	STORE_INDEX_STEPS,		// �Ʋ������������е��±�
	STORE_INDEX_SLEEP,		// ˯�������������е��±�
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
	uint16_t	len;		// ���ݵĳ���
	void		*data;		// ��ʵ����
}flash_data_t;


typedef struct
{
	__OIO uint16_t	sample_interval:12;	// �������					12bit -> 4 + 12 = 16bit = 2Byte
	__OIO uint16_t	sample_unit:4;		// ������λ					4bit
}flash_op_sample_t;



typedef struct  // 16Byte
{
	__OIO uint32_t				utc; 				// ʱ���					4Byte
	
	__OIO flash_op_sample_t 	sample;				// ������������λ����		2Byte
	__OIO uint8_t				frame_len;			// һ֡���ݵĳ���
	__OIO uint8_t				reserve;			// Ԥ��						2Byte	
	
	__OIO uint32_t				start_addr:24;		// ���ݶε��׵�ַ			3Byte
	__OIO uint32_t				valid:8;			// ��ͷ�Ƿ���Ч�����ܱ�ɾ��	1Byte
	
	__OIO uint32_t				end_addr;			// ���ݶεĽ�����ַ			4Byte
	
}flash_op_dir_t;

typedef struct  // 16Byte
{
	__OIO uint32_t				utc; 				// ʱ���	4Byte	
    
	__OIO flash_op_sample_t 	sample;				// ������������λ����
    __OIO uint16_t				dir_offset;			// ��Ŀ¼������ƫ�Ƶ�ַ
    
    
    __OIO uint32_t            	frame_len:8;
	__OIO uint32_t				start_addr:24;			// ���ݶε��׵�ַ			3Byte
    
    __OIO uint32_t            	valid:8;
	__OIO uint32_t				end_addr:24;			// ���ݶεĽ�����ַ			4Byte

}flash_dir_list_t;



typedef struct
{
	__OIO flash_op_dir_t		dir;			    // ��ǰĿ¼��ͷ
    
	__OIO uint32_t				write;				// �Ѿ�д��flash��ĳ����ַ	3Byte	
//	__OIO uint32_t				read;				// �Ѿ�����flash��ĳ����ַ	4Byte
	__OIO uint32_t				origin;				// ��ͷ�����ڴ����׵�ַ
	__OIO uint32_t				head;				// ��������ַͷ
	__OIO uint32_t				tail;				// ��������ַβ
	__OIO uint32_t				dir_offset;			// ��һ��Ҫд���Ŀ¼
    __OIO uint32_t              list_travel_offset:24;    // ��һ��Ҫ������Ŀ¼    
    __OIO uint32_t              list_total:8;       // Ŀǰ�Ѽ�¼��Ŀ¼����
//	__OIO uint32_t				dir_end;		    // ��ͷ��Ч����������ƫ�Ƶ�ַ
//	__OIO uint32_t				data_len;			// ��ǰĿ¼��д������ݳ��ȣ���λ��֡

	__OIO flash_dir_list_t		list[DIR_LIST_SIZE];// ��ż�¼����ЧĿ¼
	__OIO uint32_t				data_offset;		// ����д�������ַ�������ڱ�ͷ��ƫ����
	__OIO uint16_t				frame_len;			// ��ǰһ֡���ݵĳ���
	__OIO flash_op_sample_t		sample;				// ��ǰ��������
	
	__OIO uint8_t				init_flag;			// �Ƿ��Ѿ���ʼ��
	__OIO uint8_t				dir_ready;			// �Ƿ��Ѿ��½���Ŀ¼��д����֮ǰ������Ŀ¼
    __OIO uint8_t               reserve0;  			// Ԥ��
	__OIO uint16_t				reserve1;			// Ԥ��
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

// �������ڲ���
ret_code_t flash_init(void);

void flash_body_write_test(void);

#endif
