#include "m_flash_op.h"
#include "m_drv_w25x80.h"

#include "m_flash_addr.h"
#include "m_database.h"
#include "includes.h"

#include "AD_DataStore.h"

/* 是否需要遍历目录列表 */
#define FLASH_DIR_LIST_TRAVEL	false
/* 是否需要打印该函数运行时间，单位0.1ms */
#define FLASH_RTC_PRINT			true

#if CONFIG_RTT_FLASH_OP_ENABLED
#define FLASH_Print(M, ...)    SEGGER_Print("FLASH_OP", M, ##__VA_ARGS__)
#else
#define FLASH_Print(M, ...)
#endif

//#define BODY_UPLOAD_SIZE	(1000)	//假设值
//#define PRESSURE_UPLOAD_SIZE	(98)	//假设值

#if (BODY_UPLOAD_SIZE > BODY_DATA_MAX_SIZE)
	#error "BODY_UPLOAD_SIZE too large"
#endif


#if (PRESSURE_UPLOAD_SIZE > PRESSURE_DATA_MAX_SIZE)
	#error "PRESSURE_UPLOAD_SIZE too large"
#endif

#define OP_ENABLE_MARK		(0x55)
#define OP_DATE_OLD         (60 * 60 * 24 * 4)	// 4天数据


#define FLASH_DIR_OF_PAGE_MAX   16
#define FLASH_PAGE_SIZE			256
#define FLASH_SECTOR_SIZE		4096
#define FLASH_PAGE_OF_SECTOR	(FLASH_SECTOR_SIZE / FLASH_PAGE_SIZE)
#define FLASH_DIR_OF_PAGE		(FLASH_PAGE_SIZE / DIR_SIZE)

#define DIR_MAX					(FLASH_SECTOR_SIZE / sizeof(flash_op_dir_t))

#define DIR_ADDR_OFFSET			(FLASH_SECTOR_SIZE)
#define DATA_ADDR_OFFSET		(DIR_ADDR_OFFSET + FLASH_SECTOR_SIZE)

//************************************************************************************************************
//内存划分：
//	1.MARK & LAST_DATA_ADDR,	// 标记这块内存是否已有数据，写入 MARK_STRING，写入一帧数据记录一次数据地址
//	2.DIR						// 目录存储区，预留4KB，每个目录16Byte，此处最多可容纳256个目录
//	3.DATA0						// 第0块4KB的数据缓存区
//	  DATA1						// 第1块4KB的数据缓存区
//	  DATAn						// 第n块4KB的数据缓存区
//	----------------------		// 上述3部分，每个区域都是独立的循环擦、写
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


// 优先处理下面这2个
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
// 记得配置4字节对齐
__ALIGN(4)	flash_op_t 	flash_op_sport = {		// 活动量分包数据
	.sample.sample_unit = SAMPLE_UNIT_10S,
	.sample.sample_interval = 6,	// 300s
	.frame_len = 2,
	.origin = FLASH_SPORT_ORIGIN_ADDR,
	.head = FLASH_SPORT_HEAD_ADDR,
	.tail = FLASH_SPORT_TAIL_ADDR,
	.init_flag = 0,
	.dir_ready = 0,
};
__ALIGN(4)	flash_op_t	flash_op_sleep = {		// 睡眠
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
// 功能描述:   	关闭记步分包数据的目录
// 函数参数:    无
// 返回值  :	无        
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
// 功能描述:   	关闭睡眠分包数据的目录
// 函数参数:    无
// 返回值  :	无        
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

// 优先配置下面2个
__ALIGN(4)	flash_op_t	flash_op_body = {	// 接触式体温分包数据
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
// 功能描述:   	关闭体温分包数据的目录
// 函数参数:    无
// 返回值  :	无        
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

__ALIGN(4)	flash_op_t	flash_op_envir = {		// 环境温度
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
// 功能描述:   	关闭环境分包数据的目录
// 函数参数:    无
// 返回值  :	无        
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
// 记得配置4字节对齐
__ALIGN(4)	flash_op_t 	flash_op_th = {		// 温湿度分包数据
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
// 功能描述:   	关闭温湿度分包数据的目录
// 函数参数:    无
// 返回值  :	无        
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




/* 用于关闭计步、睡眠、体温、环境、温湿度的相关分包数据 */
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
* 上电，先从Flash的表头位置，读取上次写数据的最后一个地址，把这个地址写入没有结束地址的目录，
* 在遍历目录的时候，最多记录N个目录到目录列表，
* 如果目录列表数据已满，则不再记录新的目录，
* 当APP删除目录时，从目录列表清除数据，并把对应Flash中的目录有效标志位清除，然后从Flash读取新的目录
* 以下是待操作的几个分包数据结构体
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
// 功能描述:   	关闭所有目录
// 函数参数:    无
// 返回值  :	无        
//************************************************************************************************************
void DirCloseAll(void)
{
	for (uint8_t i = 0; i < STORE_INDEX_MAX; i++)
		DirClose[i]();
}

//************************************************************************************************************
// 功能描述:   	根据分包数据数组中的下标，获取该数据的类型
// 函数参数:    下标位置
// 返回值  :	数据类型        
//************************************************************************************************************
uint8_t	flash_arrayIndex_to_dataType(uint8_t index)
{
	if (index >= STORE_INDEX_MAX)
		return STORE_INDEX_MAX;
	
	return data_maps[index][1];
}

//************************************************************************************************************
// 功能描述:   	根据分包数据的数据的类型，获取其在数组中的下标
// 函数参数:    数据类型
// 返回值  :	下标位置        
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
// 功能描述:   	计算文件某段内存区的长度
// 函数参数:    op 	    -> 	分包数据结构体
//				start	->	内存区起始地址
//				end	    ->	内存区结束地址
// 返回值  :	计算所得数据长度         
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
// 功能描述:   	从Flash中删除目录（把目录有效标志位清除）
// 函数参数:    op 	    -> 	分包数据结构体
//				offset	->	目录偏移地址
// 返回值  :	操作结果         
//************************************************************************************************************
ret_code_t flash_dir_clear(flash_op_t *op, uint16_t offset)
{
	uint8_t		buf = DIR_INVALID;
	w25x80_write(op->origin + DIR_ADDR_OFFSET + offset * DIR_SIZE + DIR_BEYOND_SIZE, &buf, 1);
	
	return NRF_SUCCESS;
}

//************************************************************************************************************
// 功能描述:   	从Flash中查找有效目录，写入目录列表，等待APP的召唤(关闭目录或删除目录时，会有该操作)
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果         
//************************************************************************************************************
ret_code_t  flash_dir_find_no_first(flash_op_t *op)
{
	uint32_t		read = op->origin + DIR_ADDR_OFFSET + ((op->list_travel_offset) * 16);	// 目录偏移地址，每个地址占16Bytes

	uint8_t			buf[FLASH_PAGE_SIZE];
	uint16_t		remain = FLASH_PAGE_SIZE - (read & 0xFF);

	uint8_t			i, cnt;
	uint16_t		offset = 0;	

	flash_op_dir_t	*pdir = NULL; 
    
    // 目录列表已满，无须再读Flash
    if (op->list_total >= DIR_LIST_SIZE)
        return NRF_SUCCESS;
    
    for (; ;)
    {
        // 目录地址已经越界到数据区了
        if (read >= op->head)
		{
			FLASH_Print("read-> 0x%x-0x%x:0x%x\n\r", op->list_travel_offset, read, op->head);
            break;
		}
        
        // 指向缓存区的首地址
        pdir = (flash_op_dir_t *)buf;
        
        // 一次性最多读取256Byte目录数据
		w25x80_read(read, buf, remain);
		FLASH_Print("0x%x, %d\n\r", read, remain);
        // 每个地址占16Byte
        cnt = remain >> 4;
        for (i = 0; i < cnt; i++)
        {
			FLASH_Print("utc: %d, valid: 0x%x, s-e: 0x%x:0x%x\n\r", pdir->utc, pdir->valid, pdir->start_addr, pdir->end_addr);
            
            if (pdir->utc != IDLE_4BYTE_DATA)      		// utc有效
            {
                if ((pdir->valid == IDLE_BYTE_DATA) &&	// 目录标志有效，默认有效，写0x00无效
					(pdir->start_addr >= op->head) && 	// 存储区域起始地址有效
					(pdir->start_addr < op->tail))
                {
                    offset = (read - op->origin - DIR_ADDR_OFFSET) >> 4;
                    
                    if (NRF_SUCCESS != flash_dir_list_insert(op, pdir, offset+i))	// 注意偏移量
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
        
        // 下次读取数据的开始地址
		read += remain;
		remain = FLASH_PAGE_SIZE - (read & 0xFF);
    }
	
	#if FLASH_DIR_LIST_TRAVEL
	flash_dir_list_travel(op);
	#endif
	
	return NRF_SUCCESS;
}

//************************************************************************************************************
// 功能描述:   	把Flash中的目录写入目录列表，等待APP的召唤
// 函数参数:    op 	    -> 	分包数据结构体
//				dir		->	即将写入列表的目录
//				offset	->	dir在Flash中所处的偏移量
// 返回值  :	操作结果         
//************************************************************************************************************
ret_code_t	flash_dir_list_insert(flash_op_t *op, flash_op_dir_t *dir, uint16_t offset)
{
	uint8_t			i;

	// 最多只有0-255个目录
	if (offset >= DIR_MAX)		// 是否需要这个判断
		return NRF_ERROR_INVALID_PARAM;
    
    // 学员已满，请下学期再来报名
    if (op->list_total >= DIR_LIST_SIZE)
        return NRF_ERROR_INVALID_PARAM;
	
	
	if ((dir->end_addr < op->head) || (dir->end_addr >= op->tail))
		return NRF_ERROR_INVALID_PARAM; 
    
	

	// 查找空闲位置，或最早时间
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (DIR_VALID != op->list[i].valid)
			break;
	}

	// 无空闲空间
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
    else    // 数据不被采纳的目录，将被舍弃
    {
        flash_dir_clear(op, offset);
    }
    // 没有数据，将不被保存，偏移地址会后跳
    op->list_travel_offset = offset + 1;   // 下一次遍历的起始地址
	
	FLASH_Print("travel_offset111: %d\n\r", op->list_travel_offset);
	
	return NRF_SUCCESS;
}


//************************************************************************************************************
// 功能描述:   	根据UTC从目录列表读取对应的下标位置
// 函数参数:    op 	    -> 	分包数据结构体
//				utc		->	查找的UTC时间
// 返回值  :	目录列表的下标位置         
//************************************************************************************************************
ret_code_t flash_dir_list_read_utc(flash_op_t *op, uint32_t utc)
{
	uint8_t		i;

	// 遍历数组，匹配UTC
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		// 目录有效									UTC一致
		if ((DIR_VALID == op->list[i].valid) && (utc == op->list[i].utc))
        {
			break;
        } 
	}
	
	return i;
}

//************************************************************************************************************
// 功能描述:   	把APP想要获取的第offset个目录转换成目录列表的下标
// 函数参数:    op 	    -> 	分包数据结构体
//				offset	->	待查找的神秘目录
// 返回值  :	目录列表的下标位置         
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
// 功能描述:   	从目录列表拷贝有效目录写入Flash，该操作发生在目录区写满，需要擦除重写之前保留目前有效目录
// 函数参数:    op 	    -> 	分包数据结构体
//				pdir	->	目录数组
//				plen	->	拷贝的目录个数
// 返回值  :	操作结果         
//************************************************************************************************************
ret_code_t flash_dir_list_read_all(flash_op_t *op, flash_op_dir_t *pdir, uint8_t *plen)
{
	uint8_t		len = 0;
	uint8_t		i;	
	
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		// 目录列表中有效的目录
		if ((DIR_VALID == op->list[i].valid) && (IDLE_4BYTE_DATA != op->list[i].end_addr))
		{
			// 已有结束地址的完整目录

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
// 功能描述:   	根据APP下发的UTC，删除目录列表匹配目录
// 函数参数:    op 	    -> 	分包数据结构体
//				utc		->	待删除目录的UTC数值
// 返回值  :	操作结果         
//************************************************************************************************************
ret_code_t flash_dir_list_del_utc(flash_op_t *op, uint32_t utc)
{
	uint8_t		i;

	// 遍历目录列表
	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (op->list[i].utc == utc)
		{
			// 清除列表中匹配的目录数据
			op->list[i].valid = DIR_INVALID;
			op->list[i].utc = IDLE_4BYTE_DATA;

			// 清除Flash中对应的目录参数
			flash_dir_clear(op, op->list[i].dir_offset);
            
            op->list_total--;
			break;
		}
	}
    
    // 以防万一
    if (op->list_total > DIR_LIST_SIZE)
        op->list_total = 0;
    
    // 删了目录之后，试图从Flash读取有效目录加入目录列表
    flash_dir_find_no_first(op);

	return NRF_SUCCESS;
}


//************************************************************************************************************
// 功能描述:   	删除目录列表中的所有目录
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果         
//************************************************************************************************************
ret_code_t flash_dir_list_del_all(flash_op_t *op)
{
	uint8_t		i;

	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		if (op->list[i].valid == DIR_VALID)
		{
			// 从Flash清除目录
			flash_dir_clear(op, op->list[i].dir_offset);
		}			
	}
	
	// 目录列表恢复初始化
	flash_dir_list_init(op);

    op->list_total = 0;
    // 删了目录之后，试图从Flash读取有效目录目录加入目录列表
    flash_dir_find_no_first(op);
	return NRF_SUCCESS;
}


//************************************************************************************************************
// 功能描述:   	从目录列表获取当前有效目录的数量
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	有效目录的数量         
//************************************************************************************************************
ret_code_t flash_dir_list_num_get(flash_op_t *op)
{
	return op->list_total;
}

//************************************************************************************************************
// 功能描述:   	获取某分包数据的有效目录的数据长度总和
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	数据总长度         
//************************************************************************************************************
ret_code_t flash_dir_list_get_size_all(flash_op_t *op)
{
	uint8_t		i;
	uint32_t		ret = 0;

	for (i = 0; i < DIR_LIST_SIZE; i++)
	{
		// 找到所有有效目录，把各目录的数据长度累加
		if (op->list[i].valid == DIR_VALID)
		{
            ret += flash_data_calculate_length(op, op->list[i].start_addr, op->list[i].end_addr);		
		}
	}

	return ret;
}


//************************************************************************************************************
// 功能描述:   	获取某分包数据的某个目录的数据长度
// 函数参数:    op 	    -> 	分包数据结构体
//				index	->	需要计算数据长度的目录下标
// 返回值  :	数据总长度         
//************************************************************************************************************
ret_code_t flash_dir_list_get_size_single(flash_op_t *op, uint8_t index)
{
    if (index >= DIR_LIST_SIZE)
        return 0;
    
    return flash_data_calculate_length(op, op->list[index].start_addr, op->list[index].end_addr);
}


//************************************************************************************************************
// 功能描述:   	初始化目录列表
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
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
// 功能描述:   	创建表头，写入初始化有效标记(需要擦除表头数据时，被调用)
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_table_create(flash_op_t *op)
{
	uint8_t		buf[8];
	w25x80_erase_sector(op->origin);

	// 表头有效位
	memcpy(buf, MARK_STRING, 4);
	
	// 表头第一个数据
	// 第一个偏移地址
	memcpy(&buf[4], (uint8_t *)&op->write, 4);

	// 一次性写入标志位和数据地址
	w25x80_write(op->origin, buf, 8);

	op->data_offset = FLASH_WRITE_INDEX_START;

	return NRF_SUCCESS;
}


//************************************************************************************************************
// 功能描述:   	在表头区域记录最后一次写数据的地址(每次写数据都会记录)
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_table_insert(flash_op_t *op)
{
	// 数据偏移地址已经写到最后，擦除重写
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
// 功能描述:   	写入目录结束地址，也标志这个目录已经写完了，即将创建新的目录
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_dir_write_finaladdr(flash_op_t *op)
{
	// 目录结束地址
	w25x80_write(op->origin + DIR_ADDR_OFFSET + DIR_SIZE * (op->dir_offset) - sizeof(uint32_t), (uint8_t *)&op->write, 4);	
	op->dir.end_addr = op->write;
    
	FLASH_Print("dir_total: %d, %d, 0x%x, 0x%x\n\r", op->list_total, op->dir_offset, op->write, op->origin + DIR_ADDR_OFFSET + DIR_SIZE * (op->dir_offset - 1) - sizeof(uint32_t));
	
    // 有新目录写完，都会试图把该目录加入目录列表
    flash_dir_find_no_first(op);
	
	op->dir_ready = 0;

	FLASH_Print("dir_total: %d\n\r", op->list_total);
	return NRF_SUCCESS;
}

//************************************************************************************************************
// 功能描述:   	初始化目录区(第一次使用或目录区存满的时候调用)
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_dir_init(flash_op_t *op)
{
    flash_op_dir_t  dir[FLASH_DIR_OF_PAGE_MAX];
	
	if (OP_ENABLE_MARK == op->dir_ready)
		return NRF_SUCCESS;
    
	// 目录区存满，读出最后一页数据，写入Flash，后面会从中读取有效目录
    if (op->dir_offset >= DIR_MAX)
    {
        w25x80_read(op->head - FLASH_PAGE_SIZE, (uint8_t *)dir, FLASH_PAGE_SIZE);	// 目录区最后一页
    }      
    
	// 擦除目录空间
	w25x80_erase_sector(op->origin + DIR_ADDR_OFFSET);
    
    if (op->dir_offset >= DIR_MAX)
    {
        op->dir_offset = FLASH_DIR_OF_PAGE_MAX;
        w25x80_write(op->origin + DIR_ADDR_OFFSET, (uint8_t *)dir, FLASH_PAGE_SIZE);	// 最后一页目录重新写到首页
    }
    
	// 目录大扫除，目录列表大清洗
    flash_dir_list_init(op);
    op->list_total = 0;
    op->list_travel_offset = 0;
	
	
	FLASH_Print("Dir Init\n\r");

	// 重新从Flash读取有效目录
    flash_dir_find_no_first(op);
    
	return NRF_SUCCESS;
}


//************************************************************************************************************
// 功能描述:   	插入一个新的目录(写入数据时，如果还没有目录就创建目录，否则在原有目录中接着写)
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_dir_insert(flash_op_t *op)
{
	// 是否已创建了目录
	if (OP_ENABLE_MARK == op->dir_ready)
		return NRF_SUCCESS;

	
	if (op->dir_offset >= DIR_MAX)
	{
		// 目录已满，需要清除空间，重新写
		flash_dir_init(op);
	}
	#if FLASH_RTC_PRINT
	uint32_t	rtc = NRF_RTC1->COUNTER;
	#endif
    
    // 一开始就应具备的目录信息，结束地址与有效位是后面写的
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
// 功能描述:   	从表头查找最后一次写数据的地址偏移量
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_table_find_finally_data_offset(flash_op_t *op)
{
	uint16_t	index[INDEX_MAX] = {FLASH_WRITE_INDEX_START, FLASH_WRITE_INDEX_END};
	uint16_t	pos;
	uint32_t	addr = 0;
	

	// 读取最后一个位置
	w25x80_read(op->origin + (FLASH_WRITE_INDEX_END << 2), (uint8_t *)&addr, 4);
	
	// 已经写到了最后
	if (IDLE_4BYTE_DATA != addr)
	{
		return FLASH_WRITE_INDEX_END;
	}

	for (; ;)
	{
		// 计算最中间位置的地址偏移
		pos = (index[INDEX_LOW] + index[INDEX_TOP]) >> 1;

		// 读取中间地址的数据，每个地址占据4个字节，起始位置填了"MOVT"，所以偏移sizeof(uint32_t)
		w25x80_read(op->origin + (pos << 2), (uint8_t *)&addr, 4);
		if (IDLE_4BYTE_DATA == addr)	// 未被写入数据的空间应该处于高地址
		{
			// 由于从前往后写，所以 INDEX_TOP 存放的是未被写过的地址0xFF
			index[INDEX_TOP] = pos;
		}
		else
		{
			// 由于从前往后写，所以 INDEX_LOW 存放的是已被写过的地址非0xFF
			index[INDEX_LOW] = pos;
		}

		// 写数据的地址与未写数据的地址相邻或相等，认为写数据的最后一个地址被找到
		if ((index[INDEX_LOW] + 1) >= index[INDEX_TOP])
			break;
	}

	// 返回数据所写的最后一个地址的偏移量
	return index[INDEX_LOW];
}


//************************************************************************************************************
// 功能描述:   	根据表头最后一次写数据的地址偏移量，获取(最后一次写数据)对应的地址
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_table_find_finaly_data_addr(flash_op_t *op)
{
	uint32_t		addr = IDLE_4BYTE_DATA;

	// 没有结束地址，应从表头开始查找，返回表头数据的偏移位置
	op->data_offset = flash_table_find_finally_data_offset(op);

	// 读取结束地址	
	w25x80_read(op->origin + sizeof(uint32_t) * (op->data_offset), (uint8_t *)&addr, sizeof(uint32_t));
	FLASH_Print("lastest data addr: 0x%x\n\r", addr);
	// 上一次只写到这里，所以这是此次开机写的起始地址
	return addr;
}



//************************************************************************************************************
// 功能描述:   	开机后第一次从Flash读取目录信息，存入目录列表
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_dir_find_first(flash_op_t *op)
{
	uint32_t		read = op->origin + DIR_ADDR_OFFSET;	// 目录偏移地址

	uint8_t			buf[FLASH_PAGE_SIZE];
//	uint16_t		remain = FLASH_PAGE_SIZE - (read & 0xFF);

	uint8_t			i, j;
	uint16_t		offset = 0;	

	flash_op_dir_t	*dir = NULL; 
	FLASH_Print("rr: 0x%x\n\r", read);
	// 一次最多读取一页256B，4K空间需要读取的次数
	for (i = 0; i < FLASH_PAGE_OF_SECTOR; i++)	// 16次
	{
        // 目录地址越界到了数据区
        if (read >= op->head)
            break;
        
        // 指向缓存区的首地址
        dir = (flash_op_dir_t *)buf;
        
        // 一次性读取256Byte目录数据
		w25x80_read(read, buf, FLASH_PAGE_SIZE);

		// 下次读取数据的开始地址
		read += FLASH_PAGE_SIZE;
//		remain = FLASH_PAGE_SIZE - (read & 0xFF);

		for (j = 0; j < FLASH_DIR_OF_PAGE; j++)		// 16次
		{
			// 无效的时间记录，后面已经没有数据了
			if (dir->utc == IDLE_4BYTE_DATA)
			{
				FLASH_Print("op read: 0x%x, %d\n\r", read - FLASH_PAGE_SIZE, j);
				goto IDLE_DATA;
			
			}

            // 目录的偏移地址 0-255(每个目录16Byte)
			offset = i * FLASH_DIR_OF_PAGE + j;
			// 该目录有时间记录，并且有效位未清除
			if ((dir->utc != IDLE_4BYTE_DATA) &&    // utc有效
				(dir->valid == IDLE_BYTE_DATA) &&   // 目录标志有效，默认有效，写0x00无效
				(dir->start_addr >= op->head) && 	// 存储区域起始地址有效
				(dir->start_addr < op->tail))
			{	
				// 无效的结束地址，先查找结束地址，再写回去（异常复位才会有没有结束地址）
				if (dir->end_addr == IDLE_4BYTE_DATA)
				{
					// 给结束地址赋值
					dir->end_addr = op->write;

					// 当前操作目录的偏移位置
					op->dir_offset = offset + 1;
					
                    // 写下该目录的结束地址
					flash_dir_write_finaladdr(op);
					
					// 写目录列表 
					flash_dir_list_insert(op, dir, offset);
					goto IDLE_DATA;
				}
				else
                // 写目录列表
				flash_dir_list_insert(op, dir, offset);

			}
            
            // 去往下一个目录
            dir++;
		}

		// 遇到了空目录
		if (j != FLASH_DIR_OF_PAGE)
		{
			FLASH_Print("Read Blank Dir\n\r");
			goto IDLE_DATA;
		}
	}
	
IDLE_DATA:
	// 记录目录的偏移位置
	op->dir_offset = offset + 1;
	FLASH_Print("dir offset: %d\n\r", op->dir_offset);
	return NRF_SUCCESS;
}


//************************************************************************************************************
// 功能描述:   	把分包数据写入Flash
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_data_insert(flash_op_t *op, flash_data_t *flash_data)
{
    uint16_t	remain;
	
	// flash内的元素长度与要存的数据长度不相等
	if (op->dir.frame_len != flash_data->len)
		return NRF_ERROR_INVALID_PARAM;

	#if FLASH_RTC_PRINT
	uint32_t	rtc = NRF_RTC1->COUNTER;
	#endif
	for (; ;)
	{
        // 这一夜还能写下多少字节
		remain = FLASH_PAGE_SIZE - (op->write & 0xFF);
        
        // 4096的倍数，需要擦除数据，才能写
        if (0 == (op->write & 0xFFF))	
		{
			w25x80_erase_sector(op->write);
		}

		// flash某页的内存可容下即将存入的内存，直接写入
		if (remain >= flash_data->len)
		{
            // 写入数据
			w25x80_write(op->write, flash_data->data, flash_data->len);
            // 写地址偏移
			op->write += flash_data->len;
            // 写完，数据长度清零，判断此变量分析数据是否写完
			flash_data->len = 0;
		}
		else
		{
            // 先写下这页，仅存的空间
			w25x80_write(op->write, flash_data->data, remain);
            // 写地址偏移
			op->write += remain;		// 下次要写的内存地址
            // 计算下次应该写的数据长度
			flash_data->len -= remain;	// 剩余要写的flash大小
		}

		// flash写入地址已达上限，从头开始
		if (op->write >= op->tail)
				op->write = op->head;
        
		// 数据已经写完了，退出循环
		if (0 == flash_data->len)
			break;
	}
    
    // 在表头记录现在写到哪里了，每次写数据都会记录
    flash_table_insert(op);

	#if FLASH_RTC_PRINT
	rtc = (NRF_RTC1->COUNTER - rtc) * 1000;
	FLASH_Print("rtc: %d.%d ms\n\r", rtc >> 15, ((rtc * 10) >> 15) % 10);
	#endif
	return NRF_SUCCESS;
}



//************************************************************************************************************
// 功能描述:   	初始化分包数据Flash区域
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
//************************************************************************************************************
ret_code_t flash_table_init(flash_op_t *op)
{
	uint8_t		buf[4];
	
	if (OP_ENABLE_MARK == op->init_flag)
		return NRF_SUCCESS;

    // 识别有效标志
	w25x80_read(op->origin, buf, 4);
	op->dir_ready = 0;
	op->write = op->head;		// 先给个默认值
	if (0 == memcmp(buf, MARK_STRING, 4))
	{
		// 表头已被初始化
		// 读取已写的有效目录
		// 读取已写数据区的最后一个地址
		FLASH_Print("Already MOVT\n\r");
		
		// 上次写到的最后地址
		op->write = flash_table_find_finaly_data_addr(op);
		
		// 从flash读取目录信息，保存到目录列表，并配置目录的结束地址，获取目录偏移地址
        flash_dir_find_first(op);
		
        // 等到写数据时，再添加目录
		// 读完目录列表后，创建一个新的目录，只在目录结束后，才写入目录列表
//        flash_dir_insert(op);
	}
	else
	{
		FLASH_Print("No MOVT\n\r");
		// 表头从未初始化
		// 在表头，写入第一个数据地址
		// 创建表头
        flash_table_create(op);
		// 创建第一个目录
        flash_dir_init(op);
	}
	
	op->init_flag = OP_ENABLE_MARK;
	return NRF_SUCCESS;
}

// 读某个目录的数据


//************************************************************************************************************
// 功能描述:   	初始化分包数据相关参数
// 函数参数:    op 	    -> 	分包数据结构体
// 返回值  :	操作结果        
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



