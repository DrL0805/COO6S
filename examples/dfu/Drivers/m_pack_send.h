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
	uint8_t			buf[PACK_SEND_BUFFER_MAX];	// ������
	uint16_t		read_pos;					// ��ָ��
	uint16_t		write_pos;					// дָ�� 
	uint16_t		bytes_max;					// ���������������֡��
}pack_data_t;

typedef struct
{
	pack_data_t		data;	// ������
	
	uint32_t		head;		// ���ݶε���������
	uint32_t		tail;		// ���ݶε���������
	uint32_t		read;		// �����ݵ���ʼ��
	uint16_t		nframes;	// ��Ҫ��ȡ���ݵ�֡��
	uint16_t		frame_len;	// ÿ֡���ݵĳ���
	uint16_t		pkg_sn;		// ���к�
	uint16_t		pkg_type;	// ��������
	
}pack_send_t;

//************************************************************************************************************
// ��������:   	ѭ���������ݵ�׼������
// ��������:    type		->		��������
//				index		->		Ŀ¼�б��е��±�
//				offset		->		������֡��ƫ����
// ����ֵ  :	�������
//************************************************************************************************************
ret_code_t	pack_data_init(uint8_t type, uint8_t	index, uint16_t	offset);

//************************************************************************************************************
// ��������:   	��Flash��ȡ��֡���ݵ���������׼������
// ��������:    ��
// ����ֵ  :	�������
//************************************************************************************************************
ret_code_t	pack_data_from_flash(void);


//************************************************************************************************************
// ��������:   	ѭ�����ͣ���Flash��ȡ���ݣ�ֱ��û����Ч����
// ��������:    ��
// ����ֵ  :	�������
//************************************************************************************************************
ret_code_t	pack_send_loop(void);

#endif



