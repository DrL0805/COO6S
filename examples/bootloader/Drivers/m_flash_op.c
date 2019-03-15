#include "m_flash_op.h"
#include "m_drv_w25x80.h"

#include "m_flash_addr.h"
#include "m_database.h"
#include "includes.h"

#include "AD_DataStore.h"

/* �Ƿ���Ҫ����Ŀ¼�б� */
#define FLASH_DIR_LIST_TRAVEL	false
/* �Ƿ���Ҫ��ӡ�ú�������ʱ�䣬��λ0.1ms */
#define FLASH_RTC_PRINT			true

#if CONFIG_RTT_FLASH_OP_ENABLED
#define FLASH_Print(M, ...)    SEGGER_Print("FLASH_OP", M, ##__VA_ARGS__)
#else
#define FLASH_Print(M, ...)
#endif

//#define BODY_UPLOAD_SIZE	(1000)	//����ֵ
//#define PRESSURE_UPLOAD_SIZE	(98)	//����ֵ

#if (BODY_UPLOAD_SIZE > BODY_DATA_MAX_SIZE)
	#error "BODY_UPLOAD_SIZE too large"
#endif


#if (PRESSURE_UPLOAD_SIZE > PRESSURE_DATA_MAX_SIZE)
	#error "PRESSURE_UPLOAD_SIZE too large"
#endif

#define OP_ENABLE_MARK		(0x55)
#define OP_DATE_OLD         (60 * 60 * 24 * 4)	// 4������


#define FLASH_DIR_OF_PAGE_MAX   16
#define FLASH_PAGE_SIZE			256
#define FLASH_SECTOR_SIZE		4096
#define FLASH_PAGE_OF_SECTOR	(FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE)
#define FLASH_DIR_OF_PAGE		(FLASH_PAGE_SIZE / DIR_SIZE)

#define DIR_MAX					(FLASH_SECTOR_SIZE / sizeof(flash_op_dir_t))

#define DIR_ADDR_OFFSET			(FLASH_SECTOR_SIZE)
#define DATA_ADDR_OFFSET		(DIR_ADDR_OFFSET + FLASH_SECTOR_SIZE)

//************************************************************************************************************
//�ڴ滮�֣�
//	1.MARK & LAST_DATA_ADDR,	// �������ڴ��Ƿ��������ݣ�д�� MARK_STRING��д��һ֡���ݼ�¼һ�����ݵ�ַ
//	2.DIR						// Ŀ¼�洢����Ԥ��4KB��ÿ��Ŀ¼16Byte���˴���������256��Ŀ¼
//	3.DATA0						// ��0��4KB�����ݻ�����
//	  DATA1						// ��1��4KB�����ݻ�����
//	  DATAn						// ��n��4KB�����ݻ�����
//	----------------------		// ����3���֣�ÿ�������Ƕ�����ѭ������д
//************************************************************************************************************


#define FLASH_VALID_OFFSET		(11)

#define FLASH_STORAGE_N_DAYS		(3)


#define FLASH_WRITE_INDEX_START		1
#define FLASH_WRITE_INDEX_END		(FLASH_SECTOR_SIZE / (sizeof(uint32_t)) - 1)
	
//#if ((FLASH_WRITE_INDEX_START + 1) >= FLASH_WRITE_INDEX_END)
//# error "Flash write index error!"
//#endif

#define FLASH_SPORT_FRAME_LEN		2
#define FLASH_SPORT_ORIGIN_ADDR		(RESET_ADDR + 4096)		
#define FLASH_SPORT_HEAD_ADDR		(FLASH_SPORT_ORIGIN_ADDR + DATA_ADDR_OFFSET)
#define FLASH_SPORT_TAIL_ADDR		(FLASH_SPORT_HEAD_ADDR + (12 * 1024))

#define FLASH_SLEEP_FRAME_LEN		2
#define FLASH_SLEEP_ORIGIN_ADDR		(FLASH_SPORT_TAIL_ADDR)		
#define FLASH_SLEEP_HEAD_ADDR		(FLASH_SLEEP_ORIGIN_ADDR + DATA_ADDR_OFFSET)
#define FLASH_SLEEP_TAIL_ADDR		(FLASH_SLEEP_HEAD_ADDR + (12 * 1024))


// ���ȴ���������2��
#define FLASH_BODY_FRAME_LEN		2
#define FLASH_BODY_ORIGIN_ADDR		0		
#define FLASH_BODY_HEAD_ADDR		(FLASH_BODY_ORIGIN_ADDR + DATA_ADDR_OFFSET)
#define FLASH_BODY_TAIL_ADDR		(FLASH_BODY_HEAD_ADDR + (16 * 1024))	// 4 sectors

#define FLASH_PRESSURE_FRAME_LEN		4
#define FLASH_PRESSURE_ORIGIN_ADDR		FLASH_BODY_TAIL_ADDR		
#define FLASH_PRESSURE_HEAD_ADDR		(FLASH_PRESSURE_ORIGIN_ADDR + DATA_ADDR_OFFSET)
#define FLASH_PRESSURE_TAIL_ADDR		(FLASH_PRESSURE_HEAD_ADDR + (32 * 1024))


//#define __ALIGNS(...)		
//#define __ALIGNS(n)		__ALIGN(n)

#if CONFIG_GSENSOR_ENABLED
// �ǵ�����4�ֽڶ���
__ALIGN(4)	flash_op_t 	flash_op_sport = {		// ����ְ�����
	.sample.sample_unit = SAMPLE_UNIT_10S,
	.sample.sample_interval = 6,	// 300s
	.frame_len = 2,
	.origin = FLASH_SPORT_ORIGIN_ADDR,
	.head = FLASH_SPORT_HEAD_ADDR,
	.tail = FLASH_SPORT_TAIL_ADDR,
	.init_flag = 0,
	.dir_ready = 0,
};
__ALIGN(4)	flash_op_t	flash_op_sleep = {		// ˯��
	.sample.sample_unit = SAMPLE_UNIT_10S,
	.sample.sample_interval = 6,	// 300s
	.frame_len = 2,
	.origin = FLASH_SLEEP_ORIGIN_ADDR,
	.head = FLASH_SLEEP_HEAD_ADDR,
	.tail = FLASH_SLEEP_TAIL_ADDR,
	.init_flag = 0,
	.dir_ready = 0,
};

//************************************************************************************************************
// ��������:   	�رռǲ��ְ����ݵ�Ŀ¼
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
void DirCloseSteps(void)
{
    
    if (OP_ENABLE_MARK == flash_op[STORE_INDEX_STEPS]->dir_ready)
	{
        FLASH_Print("StepsDirClose......\n\r");
        flash_dir_write_finaladdr(flash_op[STORE_INDEX_STEPS]);
        gsensor_mode = GSENSOR_MODE_NONE;
	}
}

//************************************************************************************************************
// ��������:   	�ر�˯�߷ְ����ݵ�Ŀ¼
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
void DirCloseSleep(void)
{
    if (OP_ENABLE_MARK == flash_op[STORE_INDEX_SLEEP]->dir_ready)
	{
        FLASH_Print("SleepDirClose......\n\r");
        flash_dir_write_finaladdr(flash_op[STORE_INDEX_SLEEP]);        
        gsensor_mode = GSENSOR_MODE_NONE;
	}
}
#endif


//void DirCloseSteps(void);
//void DirCloseSleep(void);
//void DirCloseBody(void);
//void DirCloseEnvir(void);


#if CONFIG_BODY_TEMPERATOR_ENABLED

// ������������2��
__ALIGN(4)	flash_op_t	flash_op_body = {	// �Ӵ�ʽ���·ְ�����
	.sample.sample_unit = SAMPLE_UNIT_1S,
	.sample.sample_interval = 60,	// 60s
	.frame_len = 2,
	.origin = FLASH_BODY_ORIGIN_ADDR,
	.head = FLASH_BODY_HEAD_ADDR,
	.tail = FLASH_BODY_TAIL_ADDR,
	.init_flag = 0,
	.dir_ready = 0,
};

//************************************************************************************************************
// ��������:   	�ر����·ְ����ݵ�Ŀ¼
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
void DirCloseBody(void)
{  
    if (OP_ENABLE_MARK == flash_op[STORE_INDEX_BODY]->dir_ready)
	{
		FLASH_Print("BodyDirClose......\n\r");
		flash_dir_write_finaladdr(flash_op[STORE_INDEX_BODY]);
	}
}
#endif

#if CONFIG_PRESSURE_ENABLED

__ALIGN(4)	flash_op_t	flash_op_envir = {		// �����¶�
	.sample.sample_unit = SAMPLE_UNIT_1S,
	.sample.sample_interval = 60,	// 60s
	.frame_len = 4,
	.origin = FLASH_PRESSURE_ORIGIN_ADDR,
	.head = FLASH_PRESSURE_HEAD_ADDR,
	.tail = FLASH_PRESSURE_TAIL_ADDR,
	.init_flag = 0,
	.dir_ready = 0,
};

//************************************************************************************************************
// ��������:   	�رջ����ְ����ݵ�Ŀ¼
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
void DirCloseEnvir(void)
{
    if (OP_ENABLE_MARK == flash_op[STORE_INDEX_PRESSURE]->dir_ready)
	{
		FLASH_Print("EnvirDirClose......\n\r");
		flash_dir_write_finaladdr(flash_op[STORE_INDEX_PRESSURE]);
	}
}
#endif


#if CONFIG_TEMPERATURE_HUMIDITY_ENABLED
// �ǵ�����4�ֽڶ���
__ALIGN(4)	flash_op_t 	flash_op_th = {		// ��ʪ�ȷְ�����
	.sample.sample_unit = SAMPLE_UNIT_10S,
	.sample.sample_interval = 6,	// 300s
	.frame_len = 4,
	.origin = 0,
	.head = 0,
	.tail = 0,
	.init_flag = 0,
	.dir_ready = 0,
};

//************************************************************************************************************
// ��������:   	�ر���ʪ�ȷְ����ݵ�Ŀ¼
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
void DirCloseTH(void)
{
    if (OP_ENABLE_MARK == flash_op[STORE_INDEX_TH]->dir_ready)
	{
		FLASH_Print("DirCloseTH......\n\r");
		flash_dir_write_finaladdr(flash_op[STORE_INDEX_TH]);
	}
}
#endif




/* ���ڹرռƲ���˯�ߡ����¡���������ʪ�ȵ���طְ����� */
pFun DirClose[STORE_INDEX_MAX] = {
	#if CONFIG_GSENSOR_ENABLED
	DirCloseSteps, 
	DirCloseSleep,
	#endif
	
	#if CONFIG_BODY_TEMPERATOR_ENABLED
	DirCloseBody, 
	#endif
	
	#if CONFIG_PRESSURE_ENABLED
	DirCloseEnvir,
	#endif
	
	#if CONFIG_TEMPERATURE_HUMIDITY_ENABLED
	DirCloseTH,
	#endif
};

/*
* �ϵ磬�ȴ�Flash�ı�ͷλ�ã���ȡ�ϴ�д���ݵ����һ����ַ���������ַд��û�н�����ַ��Ŀ¼��
* �ڱ���Ŀ¼��ʱ������¼N��Ŀ¼��Ŀ¼�б�
* ���Ŀ¼�б��������������ټ�¼�µ�Ŀ¼��
* ��APPɾ��Ŀ¼ʱ����Ŀ¼�б�������ݣ����Ѷ�ӦFlash�е�Ŀ¼��Ч��־λ�����Ȼ���Flash��ȡ�µ�Ŀ¼
* �����Ǵ������ļ����ְ����ݽṹ��
*/
flash_op_t	*flash_op[STORE_INDEX_MAX] = {
	#if CONFIG_GSENSOR_ENABLED
	&flash_op_sport, 
	&flash_op_sleep, 
	#endif	

	#if CONFIG_BODY_TEMPERATOR_ENABLED
	&flash_op_body, 
	#endif
	
	#if CONFIG_PRESSURE_ENABLED	
	&flash_op_envir
	#endif
};

const uint8_t data_maps[STORE_INDEX_MAX][2] =
{														
	#if CONFIG_GSENSOR_ENABLED							
	{STORE_INDEX_STEPS, 		0},			
	{STORE_INDEX_SLEEP,			1},			
	#endif	
	
	#if CONFIG_BODY_TEMPERATOR_ENABLED					
	{STORE_INDEX_BODY,			DATA_TYPE_BODY},			
	#endif		
	
	#if CONFIG_PRESSURE_ENABLED					
	{STORE_INDEX_PRESSURE,			DATA_TYPE_PRESSURE},			
	#endif	
	
	#if CONFIG_TEMPERATURE_HUMIDITY_ENABLED				
	{STORE_INDEX_TH,			DATA_TYPE_TH},				
	#endif												
};

//************************************************************************************************************
// ��������:   	�ر�����Ŀ¼
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
void DirCloseAll(void)
{
	for (uint8_t i = 0; i < STORE_INDEX_MAX; i++)
		DirClose[i]();
}

//************************************************************************************************************
// ��������:   	���ݷְ����������е��±꣬��ȡ�����ݵ�����
// ��������:    �±�λ��
// ����ֵ  :	��������        
//************************************************************************************************************
uint8_t	flash_arrayIndex_to_dataType(uint8_t index)
{
	if (index >= STORE_INDEX_MAX)
		return STORE_INDEX_MAX;
	
	return data_maps[index][1];
}

//************************************************************************************************************
// ��������:   	���ݷְ����ݵ����ݵ����ͣ���ȡ���������е��±�
// ��������:    ��������
// ����ֵ  :	�±�λ��        
//************************************************************************************************************
uint8_t	flash_dataType_to_arrayIndex(uint8_t type)
{
	uint8_t 	i;
	for (i = 0; i < STORE_INDEX_MAX; i++)
	{
		if (type == data_maps[i][1])
			break;
	}
	
	return i;
}


//************************************************************************************************************
// ��������:   	�����ļ�ĳ���ڴ����ĳ���
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				start	->	�ڴ�����ʼ��ַ
//				end	    ->	�ڴ���������ַ
// ����ֵ  :	�����������ݳ���         
//************************************************************************************************************
ret_code_t flash_data_calculate_length(flash_op_t *op, uint32_t start, uint32_t end)
{
	uint32_t	len;
	if (end >= start)
	{
		len = end - start;
	}
	else
	{
		len = (end - op->head) + (op->tail - start);
	}

	return len;
}


//************************************************************************************************************
// ��������:   	��Flash��ɾ��Ŀ¼����Ŀ¼��Ч��־λ�����
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				offset	->	Ŀ¼ƫ�Ƶ�ַ
// ����ֵ  :	�������         
//************************************************************************************************************
ret_code_t flash_dir_clear(flash_op_t *op, uint16_t offset)
{
	uint8_t		buf = DIR_INVALID;
	w25x80_write(op->origin + DIR_ADDR_OFFSET + offset * DIR_SIZE + DIR_BEYOND_SIZE, &buf, 1);
	
	return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	��Flash�в�����ЧĿ¼��д��Ŀ¼�б��ȴ�APP���ٻ�(�ر�Ŀ¼��ɾ��Ŀ¼ʱ�����иò���)
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������         
//************************************************************************************************************
ret_code_t  flash_dir_find_no_first(flash_op_t *op)
{
	uint32_t		read = op->origin + DIR_ADDR_OFFSET + ((op->list_travel_offset) * 16);	// Ŀ¼ƫ�Ƶ�ַ��ÿ����ַռ16Bytes

	uint8_t			buf[FLASH_PAGE_SIZE];
	uint16_t		remain = FLASH_PAGE_SIZE - (read & 0xFF);

	uint8_t			i, cnt;
	uint16_t		offset = 0;	

	flash_op_dir_t	*pdir = NULL; 
    
    // Ŀ¼�б������������ٶ�Flash
    if (op->list_total >= DIR_LIST_SIZE)
        return NRF_SUCCESS;
    
    for (; ;)
    {
        // Ŀ¼��ַ�Ѿ�Խ�絽��������
        if (read >= op->head)
		{
			FLASH_Print("read-> 0x%x-0x%x:0x%x\n\r", op->list_travel_offset, read, op->head);
            break;
		}
        
        // ָ�򻺴������׵�ַ
        pdir = (flash_op_dir_t *)buf;
        
        // һ��������ȡ256ByteĿ¼����
		w25x80_read(read, buf, remain);
		FLASH_Print("0x%x, %d\n\r", read, remain);
        // ÿ����ַռ16Byte
        cnt = remain >> 4;
        for (i = 0; i < cnt; i++)
        {
			FLASH_Print("utc: %d, valid: 0x%x, s-e: 0x%x:0x%x\n\r", pdir->utc, pdir->valid, pdir->start_addr, pdir->end_addr);
            
            if (pdir->utc != IDLE_4BYTE_DATA)      		// utc��Ч
            {
                if ((pdir->valid == IDLE_BYTE_DATA) &&	// Ŀ¼��־��Ч��Ĭ����Ч��д0x00��Ч
					(pdir->start_addr >= op->head) && 	// �洢������ʼ��ַ��Ч
					(pdir->start_addr < op->tail))
                {
                    offset = (read - op->origin - DIR_ADDR_OFFSET) >> 4;
                    
                    if (NRF_SUCCESS != flash_dir_list_insert(op, pdir, offset+i))	// ע��ƫ����
                    {
                        return NRF_SUCCESS;
                    }
                }
            }
            else
            {
                return NRF_SUCCESS;
            }
            pdir++;
        }
        
        // �´ζ�ȡ���ݵĿ�ʼ��ַ
		read += remain;
		remain = FLASH_PAGE_SIZE - (read & 0xFF);
    }
	
	#if FLASH_DIR_LIST_TRAVEL
	flash_dir_list_travel(op);
	#endif
	
	return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	��Flash�е�Ŀ¼д��Ŀ¼�б��ȴ�APP���ٻ�
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				dir		->	����д���б��Ŀ¼
//				offset	->	dir��Flash��������ƫ����
// ����ֵ  :	�������         
//************************************************************************************************************
ret_code_t	flash_dir_list_insert(flash_op_t *op, flash_op_dir_t *dir, uint16_t offset)
{
	uint8_t			i;

	// ���ֻ��0-255��Ŀ¼
	if (offset >= DIR_MAX)		// �Ƿ���Ҫ����ж�
		return NRF_ERROR_INVALID_PARAM;
    
    // ѧԱ����������ѧ����������
    if (op->list_total >= DIR_LIST_SIZE)
        return NRF_ERROR_INVALID_PARAM;
	
	
	if ((dir->end_addr < op->head) || (dir->end_addr >= op->tail))
		return NRF_ERROR_INVALID_PARAM; 
    
	

	// ���ҿ���λ�ã�������ʱ��
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (DIR_VALID != op->list[i].valid)
			break;
	}

	// �޿��пռ�
	if (i == DIR_LIST_SIZE)
	{
        return NRF_ERROR_INVALID_LENGTH;
	}

    uint32_t        old_utc = dir->utc + OP_DATE_OLD;
    
    if ((dir->start_addr != dir->end_addr) && (old_utc >= system_database.utc_seconds))
    {
        op->list[i].utc = dir->utc;
        op->list[i].sample = dir->sample;
        op->list[i].frame_len = dir->frame_len;
        op->list[i].start_addr = dir->start_addr;
        op->list[i].end_addr = dir->end_addr;
        op->list[i].dir_offset = offset;
        op->list[i].valid = DIR_VALID;
        
        op->list_total++;
    }
    else    // ���ݲ������ɵ�Ŀ¼����������
    {
        flash_dir_clear(op, offset);
    }
    // û�����ݣ����������棬ƫ�Ƶ�ַ�����
    op->list_travel_offset = offset + 1;   // ��һ�α�������ʼ��ַ
	
	FLASH_Print("travel_offset111: %d\n\r", op->list_travel_offset);
	
	return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	����UTC��Ŀ¼�б��ȡ��Ӧ���±�λ��
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				utc		->	���ҵ�UTCʱ��
// ����ֵ  :	Ŀ¼�б���±�λ��         
//************************************************************************************************************
ret_code_t flash_dir_list_read_utc(flash_op_t *op, uint32_t utc)
{
	uint8_t		i;

	// �������飬ƥ��UTC
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		// Ŀ¼��Ч									UTCһ��
		if ((DIR_VALID == op->list[i].valid) && (utc == op->list[i].utc))
        {
			break;
        } 
	}
	
	return i;
}

//************************************************************************************************************
// ��������:   	��APP��Ҫ��ȡ�ĵ�offset��Ŀ¼ת����Ŀ¼�б���±�
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				offset	->	�����ҵ�����Ŀ¼
// ����ֵ  :	Ŀ¼�б���±�λ��         
//************************************************************************************************************
ret_code_t flash_dir_list_offset_to_index(flash_op_t *op, uint32_t offset)
{
	uint8_t		i;
    uint8_t     index = 0;
    
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (DIR_VALID == op->list[i].valid)
        {
            if (offset == index)
                break;
            
           index++; 
        } 
	}
	
	return i;
}


//************************************************************************************************************
// ��������:   	��Ŀ¼�б�����ЧĿ¼д��Flash���ò���������Ŀ¼��д������Ҫ������д֮ǰ����Ŀǰ��ЧĿ¼
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				pdir	->	Ŀ¼����
//				plen	->	������Ŀ¼����
// ����ֵ  :	�������         
//************************************************************************************************************
ret_code_t flash_dir_list_read_all(flash_op_t *op, flash_op_dir_t *pdir, uint8_t *plen)
{
	uint8_t		len = 0;
	uint8_t		i;	
	
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		// Ŀ¼�б�����Ч��Ŀ¼
		if ((DIR_VALID == op->list[i].valid) && (IDLE_4BYTE_DATA != op->list[i].end_addr))
		{
			// ���н�����ַ������Ŀ¼

			pdir[len].end_addr = op->list[i].end_addr;
			
			pdir[len].start_addr = op->list[i].start_addr;

			pdir[len].sample = op->list[i].sample;
			pdir[len].frame_len = op->list[i].frame_len;
			pdir[len].utc = op->list[i].utc;
			pdir[len].valid = DIR_VALID;		

			len++;
		}
	}

	*plen = len;
    
    return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	����APP�·���UTC��ɾ��Ŀ¼�б�ƥ��Ŀ¼
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				utc		->	��ɾ��Ŀ¼��UTC��ֵ
// ����ֵ  :	�������         
//************************************************************************************************************
ret_code_t flash_dir_list_del_utc(flash_op_t *op, uint32_t utc)
{
	uint8_t		i;

	// ����Ŀ¼�б�
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (op->list[i].utc == utc)
		{
			// ����б���ƥ���Ŀ¼����
			op->list[i].valid = DIR_INVALID;
			op->list[i].utc = IDLE_4BYTE_DATA;

			// ���Flash�ж�Ӧ��Ŀ¼����
			flash_dir_clear(op, op->list[i].dir_offset);
            
            op->list_total--;
			break;
		}
	}
    
    // �Է���һ
    if (op->list_total > DIR_LIST_SIZE)
        op->list_total = 0;
    
    // ɾ��Ŀ¼֮����ͼ��Flash��ȡ��ЧĿ¼����Ŀ¼�б�
    flash_dir_find_no_first(op);

	return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	ɾ��Ŀ¼�б��е�����Ŀ¼
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������         
//************************************************************************************************************
ret_code_t flash_dir_list_del_all(flash_op_t *op)
{
	uint8_t		i;

	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (op->list[i].valid == DIR_VALID)
		{
			// ��Flash���Ŀ¼
			flash_dir_clear(op, op->list[i].dir_offset);
		}			
	}
	
	// Ŀ¼�б�ָ���ʼ��
	flash_dir_list_init(op);

    op->list_total = 0;
    // ɾ��Ŀ¼֮����ͼ��Flash��ȡ��ЧĿ¼Ŀ¼����Ŀ¼�б�
    flash_dir_find_no_first(op);
	return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	��Ŀ¼�б��ȡ��ǰ��ЧĿ¼������
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	��ЧĿ¼������         
//************************************************************************************************************
ret_code_t flash_dir_list_num_get(flash_op_t *op)
{
	return op->list_total;
}

//************************************************************************************************************
// ��������:   	��ȡĳ�ְ����ݵ���ЧĿ¼�����ݳ����ܺ�
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�����ܳ���         
//************************************************************************************************************
ret_code_t flash_dir_list_get_size_all(flash_op_t *op)
{
	uint8_t		i;
	uint32_t		ret = 0;

	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		// �ҵ�������ЧĿ¼���Ѹ�Ŀ¼�����ݳ����ۼ�
		if (op->list[i].valid == DIR_VALID)
		{
            ret += flash_data_calculate_length(op, op->list[i].start_addr, op->list[i].end_addr);		
		}
	}

	return ret;
}


//************************************************************************************************************
// ��������:   	��ȡĳ�ְ����ݵ�ĳ��Ŀ¼�����ݳ���
// ��������:    op 	    -> 	�ְ����ݽṹ��
//				index	->	��Ҫ�������ݳ��ȵ�Ŀ¼�±�
// ����ֵ  :	�����ܳ���         
//************************************************************************************************************
ret_code_t flash_dir_list_get_size_single(flash_op_t *op, uint8_t index)
{
    if (index >= DIR_LIST_SIZE)
        return 0;
    
    return flash_data_calculate_length(op, op->list[index].start_addr, op->list[index].end_addr);
}


//************************************************************************************************************
// ��������:   	��ʼ��Ŀ¼�б�
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_dir_list_init(flash_op_t *op)
{
	for (uint8_t i = 0; i < DIR_LIST_SIZE; i++)
	{
		memset(&op->list[i], 0xFF, sizeof(flash_dir_list_t));
		op->list[i].valid = DIR_INVALID;
	}

	return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	������ͷ��д���ʼ����Ч���(��Ҫ������ͷ����ʱ��������)
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_table_create(flash_op_t *op)
{
	uint8_t		buf[8];
	w25x80_erase_sector(op->origin);

	// ��ͷ��Чλ
	memcpy(buf, MARK_STRING, 4);
	
	// ��ͷ��һ������
	// ��һ��ƫ�Ƶ�ַ
	memcpy(&buf[4], (uint8_t *)&op->write, 4);

	// һ����д���־λ�����ݵ�ַ
	w25x80_write(op->origin, buf, 8);

	op->data_offset = FLASH_WRITE_INDEX_START;

	return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	�ڱ�ͷ�����¼���һ��д���ݵĵ�ַ(ÿ��д���ݶ����¼)
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_table_insert(flash_op_t *op)
{
	// ����ƫ�Ƶ�ַ�Ѿ�д����󣬲�����д
	if (++op->data_offset > FLASH_WRITE_INDEX_END)
	{
		flash_table_create(op);
	}
	else
	{
		w25x80_write(op->origin + sizeof(uint32_t) * op->data_offset, (uint8_t *)&op->write, sizeof(uint32_t));
	}
    
//	op->data_offset++;
    return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	д��Ŀ¼������ַ��Ҳ��־���Ŀ¼�Ѿ�д���ˣ����������µ�Ŀ¼
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_dir_write_finaladdr(flash_op_t *op)
{
	// Ŀ¼������ַ
	w25x80_write(op->origin + DIR_ADDR_OFFSET + DIR_SIZE * (op->dir_offset) - sizeof(uint32_t), (uint8_t *)&op->write, 4);	
	op->dir.end_addr = op->write;
    
	FLASH_Print("dir_total: %d, %d, 0x%x, 0x%x\n\r", op->list_total, op->dir_offset, op->write, op->origin + DIR_ADDR_OFFSET + DIR_SIZE * (op->dir_offset - 1) - sizeof(uint32_t));
	
    // ����Ŀ¼д�꣬������ͼ�Ѹ�Ŀ¼����Ŀ¼�б�
    flash_dir_find_no_first(op);
	
	op->dir_ready = 0;

	FLASH_Print("dir_total: %d\n\r", op->list_total);
	return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	��ʼ��Ŀ¼��(��һ��ʹ�û�Ŀ¼��������ʱ�����)
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_dir_init(flash_op_t *op)
{
    flash_op_dir_t  dir[FLASH_DIR_OF_PAGE_MAX];
	
	if (OP_ENABLE_MARK == op->dir_ready)
		return NRF_SUCCESS;
    
	// Ŀ¼���������������һҳ���ݣ�д��Flash���������ж�ȡ��ЧĿ¼
    if (op->dir_offset >= DIR_MAX)
    {
        w25x80_read(op->head - FLASH_PAGE_SIZE, (uint8_t *)dir, FLASH_PAGE_SIZE);	// Ŀ¼�����һҳ
    }      
    
	// ����Ŀ¼�ռ�
	w25x80_erase_sector(op->origin + DIR_ADDR_OFFSET);
    
    if (op->dir_offset >= DIR_MAX)
    {
        op->dir_offset = FLASH_DIR_OF_PAGE_MAX;
        w25x80_write(op->origin + DIR_ADDR_OFFSET, (uint8_t *)dir, FLASH_PAGE_SIZE);	// ���һҳĿ¼����д����ҳ
    }
    
	// Ŀ¼��ɨ����Ŀ¼�б����ϴ
    flash_dir_list_init(op);
    op->list_total = 0;
    op->list_travel_offset = 0;
	
	
	FLASH_Print("Dir Init\n\r");

	// ���´�Flash��ȡ��ЧĿ¼
    flash_dir_find_no_first(op);
    
	return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	����һ���µ�Ŀ¼(д������ʱ�������û��Ŀ¼�ʹ���Ŀ¼��������ԭ��Ŀ¼�н���д)
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_dir_insert(flash_op_t *op)
{
	// �Ƿ��Ѵ�����Ŀ¼
	if (OP_ENABLE_MARK == op->dir_ready)
		return NRF_SUCCESS;

	
	if (op->dir_offset >= DIR_MAX)
	{
		// Ŀ¼��������Ҫ����ռ䣬����д
		flash_dir_init(op);
	}
	#if FLASH_RTC_PRINT
	uint32_t	rtc = NRF_RTC1->COUNTER;
	#endif
    
    // һ��ʼ��Ӧ�߱���Ŀ¼��Ϣ��������ַ����Чλ�Ǻ���д��
    op->dir.utc = system_database.utc_seconds;
    op->dir.start_addr = op->write;
    op->dir.frame_len = op->frame_len;
    op->dir.sample = op->sample;
    w25x80_write(op->origin + DIR_ADDR_OFFSET + DIR_SIZE * (op->dir_offset), (uint8_t *)&op->dir, DIR_BEYOND_SIZE);
	

	op->dir_offset++;
    
    op->dir_ready = OP_ENABLE_MARK;

	#if FLASH_RTC_PRINT
	rtc = (NRF_RTC1->COUNTER - rtc) * 1000;
	FLASH_Print("rtc: %d.%d ms\n\r", rtc >> 15, ((rtc * 10) >> 15) % 10);
	#else
	FLASH_Print("Insert new Dir\n\r");
	#endif
    return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	�ӱ�ͷ�������һ��д���ݵĵ�ַƫ����
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_table_find_finally_data_offset(flash_op_t *op)
{
	uint16_t	index[INDEX_MAX] = {FLASH_WRITE_INDEX_START, FLASH_WRITE_INDEX_END};
	uint16_t	pos;
	uint32_t	addr = 0;
	

	// ��ȡ���һ��λ��
	w25x80_read(op->origin + (FLASH_WRITE_INDEX_END << 2), (uint8_t *)&addr, 4);
	
	// �Ѿ�д�������
	if (IDLE_4BYTE_DATA != addr)
	{
		return FLASH_WRITE_INDEX_END;
	}

	for (; ;)
	{
		// �������м�λ�õĵ�ַƫ��
		pos = (index[INDEX_LOW] + index[INDEX_TOP]) >> 1;

		// ��ȡ�м��ַ�����ݣ�ÿ����ַռ��4���ֽڣ���ʼλ������"MOVT"������ƫ��sizeof(uint32_t)
		w25x80_read(op->origin + (pos << 2), (uint8_t *)&addr, 4);
		if (IDLE_4BYTE_DATA == addr)	// δ��д�����ݵĿռ�Ӧ�ô��ڸߵ�ַ
		{
			// ���ڴ�ǰ����д������ INDEX_TOP ��ŵ���δ��д���ĵ�ַ0xFF
			index[INDEX_TOP] = pos;
		}
		else
		{
			// ���ڴ�ǰ����д������ INDEX_LOW ��ŵ����ѱ�д���ĵ�ַ��0xFF
			index[INDEX_LOW] = pos;
		}

		// д���ݵĵ�ַ��δд���ݵĵ�ַ���ڻ���ȣ���Ϊд���ݵ����һ����ַ���ҵ�
		if ((index[INDEX_LOW] + 1) >= index[INDEX_TOP])
			break;
	}

	// ����������д�����һ����ַ��ƫ����
	return index[INDEX_LOW];
}


//************************************************************************************************************
// ��������:   	���ݱ�ͷ���һ��д���ݵĵ�ַƫ��������ȡ(���һ��д����)��Ӧ�ĵ�ַ
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_table_find_finaly_data_addr(flash_op_t *op)
{
	uint32_t		addr = IDLE_4BYTE_DATA;

	// û�н�����ַ��Ӧ�ӱ�ͷ��ʼ���ң����ر�ͷ���ݵ�ƫ��λ��
	op->data_offset = flash_table_find_finally_data_offset(op);

	// ��ȡ������ַ	
	w25x80_read(op->origin + sizeof(uint32_t) * (op->data_offset), (uint8_t *)&addr, sizeof(uint32_t));
	FLASH_Print("lastest data addr: 0x%x\n\r", addr);
	// ��һ��ֻд������������Ǵ˴ο���д����ʼ��ַ
	return addr;
}



//************************************************************************************************************
// ��������:   	�������һ�δ�Flash��ȡĿ¼��Ϣ������Ŀ¼�б�
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_dir_find_first(flash_op_t *op)
{
	uint32_t		read = op->origin + DIR_ADDR_OFFSET;	// Ŀ¼ƫ�Ƶ�ַ

	uint8_t			buf[FLASH_PAGE_SIZE];
//	uint16_t		remain = FLASH_PAGE_SIZE - (read & 0xFF);

	uint8_t			i, j;
	uint16_t		offset = 0;	

	flash_op_dir_t	*dir = NULL; 
	FLASH_Print("rr: 0x%x\n\r", read);
	// һ������ȡһҳ256B��4K�ռ���Ҫ��ȡ�Ĵ���
	for (i = 0; i < FLASH_PAGE_OF_SECTOR; i++)	// 16��
	{
        // Ŀ¼��ַԽ�絽��������
        if (read >= op->head)
            break;
        
        // ָ�򻺴������׵�ַ
        dir = (flash_op_dir_t *)buf;
        
        // һ���Զ�ȡ256ByteĿ¼����
		w25x80_read(read, buf, FLASH_PAGE_SIZE);

		// �´ζ�ȡ���ݵĿ�ʼ��ַ
		read += FLASH_PAGE_SIZE;
//		remain = FLASH_PAGE_SIZE - (read & 0xFF);

		for (j = 0; j < FLASH_DIR_OF_PAGE; j++)		// 16��
		{
			// ��Ч��ʱ���¼�������Ѿ�û��������
			if (dir->utc == IDLE_4BYTE_DATA)
			{
				FLASH_Print("op read: 0x%x, %d\n\r", read - FLASH_PAGE_SIZE, j);
				goto IDLE_DATA;
			
			}

            // Ŀ¼��ƫ�Ƶ�ַ 0-255(ÿ��Ŀ¼16Byte)
			offset = i * FLASH_DIR_OF_PAGE + j;
			// ��Ŀ¼��ʱ���¼��������Чλδ���
			if ((dir->utc != IDLE_4BYTE_DATA) &&    // utc��Ч
				(dir->valid == IDLE_BYTE_DATA) &&   // Ŀ¼��־��Ч��Ĭ����Ч��д0x00��Ч
				(dir->start_addr >= op->head) && 	// �洢������ʼ��ַ��Ч
				(dir->start_addr < op->tail))
			{	
				// ��Ч�Ľ�����ַ���Ȳ��ҽ�����ַ����д��ȥ���쳣��λ�Ż���û�н�����ַ��
				if (dir->end_addr == IDLE_4BYTE_DATA)
				{
					// ��������ַ��ֵ
					dir->end_addr = op->write;

					// ��ǰ����Ŀ¼��ƫ��λ��
					op->dir_offset = offset + 1;
					
                    // д�¸�Ŀ¼�Ľ�����ַ
					flash_dir_write_finaladdr(op);
					
					// дĿ¼�б� 
					flash_dir_list_insert(op, dir, offset);
					goto IDLE_DATA;
				}
				else
                // дĿ¼�б�
				flash_dir_list_insert(op, dir, offset);

			}
            
            // ȥ����һ��Ŀ¼
            dir++;
		}

		// �����˿�Ŀ¼
		if (j != FLASH_DIR_OF_PAGE)
		{
			FLASH_Print("Read Blank Dir\n\r");
			goto IDLE_DATA;
		}
	}
	
IDLE_DATA:
	// ��¼Ŀ¼��ƫ��λ��
	op->dir_offset = offset + 1;
	FLASH_Print("dir offset: %d\n\r", op->dir_offset);
	return NRF_SUCCESS;
}


//************************************************************************************************************
// ��������:   	�ѷְ�����д��Flash
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_data_insert(flash_op_t *op, flash_data_t *flash_data)
{
    uint16_t	remain;
	
	// flash�ڵ�Ԫ�س�����Ҫ������ݳ��Ȳ����
	if (op->dir.frame_len != flash_data->len)
		return NRF_ERROR_INVALID_PARAM;

	#if FLASH_RTC_PRINT
	uint32_t	rtc = NRF_RTC1->COUNTER;
	#endif
	for (; ;)
	{
        // ��һҹ����д�¶����ֽ�
		remain = FLASH_PAGE_SIZE - (op->write & 0xFF);
        
        // 4096�ı�������Ҫ�������ݣ�����д
        if (0 == (op->write & 0xFFF))	
		{
			w25x80_erase_sector(op->write);
		}

		// flashĳҳ���ڴ�����¼���������ڴ棬ֱ��д��
		if (remain >= flash_data->len)
		{
            // д������
			w25x80_write(op->write, flash_data->data, flash_data->len);
            // д��ַƫ��
			op->write += flash_data->len;
            // д�꣬���ݳ������㣬�жϴ˱������������Ƿ�д��
			flash_data->len = 0;
		}
		else
		{
            // ��д����ҳ������Ŀռ�
			w25x80_write(op->write, flash_data->data, remain);
            // д��ַƫ��
			op->write += remain;		// �´�Ҫд���ڴ��ַ
            // �����´�Ӧ��д�����ݳ���
			flash_data->len -= remain;	// ʣ��Ҫд��flash��С
		}

		// flashд���ַ�Ѵ����ޣ���ͷ��ʼ
		if (op->write >= op->tail)
				op->write = op->head;
        
		// �����Ѿ�д���ˣ��˳�ѭ��
		if (0 == flash_data->len)
			break;
	}
    
    // �ڱ�ͷ��¼����д�������ˣ�ÿ��д���ݶ����¼
    flash_table_insert(op);

	#if FLASH_RTC_PRINT
	rtc = (NRF_RTC1->COUNTER - rtc) * 1000;
	FLASH_Print("rtc: %d.%d ms\n\r", rtc >> 15, ((rtc * 10) >> 15) % 10);
	#endif
	return NRF_SUCCESS;
}



//************************************************************************************************************
// ��������:   	��ʼ���ְ�����Flash����
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t flash_table_init(flash_op_t *op)
{
	uint8_t		buf[4];
	
	if (OP_ENABLE_MARK == op->init_flag)
		return NRF_SUCCESS;

    // ʶ����Ч��־
	w25x80_read(op->origin, buf, 4);
	op->dir_ready = 0;
	op->write = op->head;		// �ȸ���Ĭ��ֵ
	if (0 == memcmp(buf, MARK_STRING, 4))
	{
		// ��ͷ�ѱ���ʼ��
		// ��ȡ��д����ЧĿ¼
		// ��ȡ��д�����������һ����ַ
		FLASH_Print("Already MOVT\n\r");
		
		// �ϴ�д��������ַ
		op->write = flash_table_find_finaly_data_addr(op);
		
		// ��flash��ȡĿ¼��Ϣ�����浽Ŀ¼�б�������Ŀ¼�Ľ�����ַ����ȡĿ¼ƫ�Ƶ�ַ
        flash_dir_find_first(op);
		
        // �ȵ�д����ʱ�������Ŀ¼
		// ����Ŀ¼�б�󣬴���һ���µ�Ŀ¼��ֻ��Ŀ¼�����󣬲�д��Ŀ¼�б�
//        flash_dir_insert(op);
	}
	else
	{
		FLASH_Print("No MOVT\n\r");
		// ��ͷ��δ��ʼ��
		// �ڱ�ͷ��д���һ�����ݵ�ַ
		// ������ͷ
        flash_table_create(op);
		// ������һ��Ŀ¼
        flash_dir_init(op);
	}
	
	op->init_flag = OP_ENABLE_MARK;
	return NRF_SUCCESS;
}

// ��ĳ��Ŀ¼������


//************************************************************************************************************
// ��������:   	��ʼ���ְ�������ز���
// ��������:    op 	    -> 	�ְ����ݽṹ��
// ����ֵ  :	�������        
//************************************************************************************************************
ret_code_t  flash_op_init(flash_op_t *op)
{
	#if FLASH_RTC_PRINT
	uint32_t	rtc = NRF_RTC1->COUNTER;
	#endif
	
    flash_dir_list_init(op);
    flash_table_init(op);
	
	#if FLASH_RTC_PRINT
	rtc = (NRF_RTC1->COUNTER - rtc) * 1000;
	FLASH_Print("rtc: %d.%d ms\n\r", rtc >> 15, ((rtc * 10) >> 15) % 10);
	#endif
	
    return NRF_SUCCESS;
}

#if FLASH_DIR_LIST_TRAVEL
ret_code_t flash_dir_list_travel(flash_op_t *op)
{
	if (op == (&flash_op_body))
	{
		FLASH_Print("Traval Body Flash: \n\r");
	}
	else if (op == (&flash_op_envir))
	{
		FLASH_Print("Traval Envir Flash: \n\r");
	}
	else if (op == (&flash_op_sleep))
	{
		FLASH_Print("Traval Sleep Flash: \n\r");
	}
	else
	{
		FLASH_Print("Traval Steps Flash: \n\r");
	}
	
	for (uint8_t i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (op->list[i].valid == DIR_VALID)
		{
			FLASH_Print("utc: %d, start: 0x%x, end: 0x%x\n\r", op->list[i].utc, op->list[i].start_addr, op->list[i].end_addr);
		}
	}
	
	return NRF_SUCCESS;
}
#endif



