#include "m_pack_send.h"
#include "protocol_pack_trans.h"
#include "event_bus.h"
#include "m_database.h"
#include "includes.h"

#if CONFIG_RTT_PACK_SEND_ENABLED
#define PACKS_Print(M, ...)	SEGGER_Print("PACK_SEND", M, ##__VA_ARGS__)
#else
#define PACKS_Print(M, ...)
#endif

pack_send_t		pack_send;

uint32_t m_coms_ble_put_data(uint8_t * p_data, uint16_t length);

//************************************************************************************************************
// ��������:   	ѭ���������ݵ�׼������
// ��������:    type		->		��������
//				index		->		Ŀ¼�б��е��±�
//				offset		->		������֡��ƫ����
// ����ֵ  :	�������
//************************************************************************************************************
ret_code_t	pack_data_init(uint8_t type, uint8_t	index, uint16_t	offset)
{
	uint32_t		len;
	flash_op_t		*op = NULL;
	
	uint8_t			store_index;
	
	store_index = flash_dataType_to_arrayIndex(type);
	
	// �������ʹ����˳�
	if (store_index >= STORE_INDEX_MAX)
		return NRF_ERROR_INVALID_PARAM;
	
	op = flash_op[store_index];
    
    pack_send.pkg_type = type;


	pack_send.read = op->list[index].start_addr;
	pack_send.head = op->head;
	pack_send.tail = op->tail;
	pack_send.frame_len = op->list[index].frame_len;
	
	if (op->list[index].end_addr > op->list[index].start_addr)
		pack_send.nframes = (op->list[index].end_addr - op->list[index].start_addr) / pack_send.frame_len;
	else
		pack_send.nframes = ((op->list[index].end_addr - op->head) + (op->tail - op->list[index].start_addr)) / pack_send.frame_len;
	
	if (offset >= pack_send.nframes)
		return NRF_ERROR_INVALID_PARAM;
	
	// ��ʣ�¶���֡��Ҫ����
	pack_send.nframes -= offset;
	
	// �����ָ��λ��
	len = op->list[index].start_addr + offset * pack_send.frame_len;
	if (len >= op->tail)
	{
		pack_send.read = len - op->tail + op->head;
	}
	else
	{
		pack_send.read = len;
	}
	
	// ����ţ��ۼ�
	pack_send.pkg_sn = 0x0000;
	
	pack_send.data.read_pos = 0;	// ��2��ֵ��Ӧ�����
	pack_send.data.write_pos = 0;
	
	pack_send.data.bytes_max = PACK_SEND_BUFFER_MAX - (PACK_SEND_BUFFER_MAX % pack_send.frame_len);
	
	// ��ʼ����������ȡ���ݲ�����
	pack_send_loop();
	
	return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	��Flash��ȡ��֡���ݵ���������׼������
// ��������:    ��
// ����ֵ  :	�������
//************************************************************************************************************
ret_code_t	pack_data_from_flash(void)
{
	if ((pack_send.nframes) && (pack_send.data.read_pos == pack_send.data.write_pos))
	{

		// ��flash�����ݵ�������
		uint32_t	len = pack_send.nframes * pack_send.frame_len;
		uint32_t	remain;
		
		// �˴�Ӧ�ö�ȡ�����ݳ���
		if (len > pack_send.data.bytes_max)
			len = pack_send.data.bytes_max;
		
		
		remain = pack_send.tail - pack_send.read;
		if (remain >= len)
		{
			remain = len;
			w25x80_read(pack_send.read, pack_send.data.buf, remain);
			pack_send.read += remain;
			
		}
		else
		{
			w25x80_read(pack_send.read, pack_send.data.buf, remain);
			
			w25x80_read(pack_send.head, pack_send.data.buf, len - remain);
			pack_send.read = pack_send.head + len - remain;
		}
		
		pack_send.data.write_pos = len;
		pack_send.data.read_pos = 0;
		
		// ʣ��֡��
		pack_send.nframes -= (len / pack_send.frame_len);
		
	}
	return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	ѭ�����ͣ���Flash��ȡ���ݣ�ֱ��û����Ч����
// ��������:    ��
// ����ֵ  :	�������
//************************************************************************************************************
ret_code_t	pack_send_loop(void)
{
    uint32_t    err_code = NRF_SUCCESS;
	
	if (system_database.bt_state != BT_CONN_STATE_CONNECTED)
	{
		pack_send.data.read_pos = 0;
		pack_send.data.write_pos = 0;
		pack_send.nframes = 0;
		PACKS_Print("BT Disconnect\n\r");
		return NRF_ERROR_INVALID_STATE;
	}
    
    // �ȴ�Flash�����ݶ�����
	pack_data_from_flash();
	
	if (pack_send.data.read_pos != pack_send.data.write_pos)
	{
		uint8_t		pack[20];
		uint8_t		index = 0;
		
		// ��������ʣ�����ݳ���
		uint16_t	copy_bytes = pack_send.data.write_pos - pack_send.data.read_pos;		
		uint8_t		copy_frame = PACK_FRAME_REMAIN_SIZE / pack_send.frame_len;
		
		if (copy_bytes < pack_send.frame_len)
		{
			// ��������
			return NRF_ERROR_INVALID_PARAM;
		}	
		
		// ���м�֡����
		copy_bytes /= pack_send.frame_len;
		
		// ȡ��С��
		if (copy_frame >  copy_bytes)
		{
			copy_frame = copy_bytes;
		}
	
		
		// Ӧ�ÿ��������ݳ���
		copy_bytes = copy_frame * pack_send.frame_len;
		
		pack[index++] = pack_send.pkg_sn >> 8;
		pack[index++] = pack_send.pkg_sn & 0xFF;
		pack[index++] = pack_send.pkg_type >> 8;
		pack[index++] = pack_send.pkg_type & 0xFF;
		
//		PACKS_Print("pack assign %d:%d\n\r", index, copy_bytes);
//		PACKS_Print("rw[%d-%d]\n\r", pack_send.data.read_pos, pack_send.data.write_pos);
		
		// ��ʣ16�ֽ����ڴ�������
		memcpy(&pack[index], &pack_send.data.buf[pack_send.data.read_pos], copy_bytes);
		index += copy_bytes;
		
		memset(&pack[index], 0xFF, 20 - index);
		
		// �����ݷ���FIFO���ȴ�����
		err_code = m_coms_ble_put_data(pack, 20);//pack_fifo_put(&m_send_pack_fifo, pack);
		
        // ���ͳɹ�����ָ����SN�����
		if (NRF_SUCCESS == err_code)
		{
			pack_send.data.read_pos += copy_bytes;
			pack_send.pkg_sn++;
		}
	}
    
    return err_code;
}


