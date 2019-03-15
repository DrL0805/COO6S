#include "m_fs.h"
#include "includes.h"

#if 0

#if CONFIG_RTT_FS_ENABLED
#define FS_Print(M, ...)	SEGGER_Print("FS", M, ##__VA_ARGS__)
#else
#define FS_Print(M, ...)
#endif

static void m_fs_evt_handler(nrf_fstorage_evt_t * p_evt);

/* FDS重要的结构体 */
NRF_FSTORAGE_DEF(nrf_fstorage_t m_fsc) =
{
    // The flash area boundaries are set in fds_init().
    .evt_handler = m_fs_evt_handler,
    .start_addr = START_ADDR,
    .end_addr = END_ADDR,
};

/* Flash device information. */
static nrf_fstorage_info_t m_flash_info =
{
#if   defined(NRF51)
    .erase_unit   = 1024,
#elif defined(NRF52_SERIES)
    .erase_unit   = 4096,
#endif
    .program_unit = 4,
    .rmap         = false,
    .wmap         = true,
};

//************************************************************************************************************
// 功能描述:   	内部Flash操作完的回调通知
// 函数参数:    p_evt		->	相关结构体	
// 返回值  :	无
//************************************************************************************************************
static void m_fs_evt_handler(nrf_fstorage_evt_t * p_evt)
{
	switch(p_evt->id)
	{
		case NRF_FSTORAGE_EVT_READ_RESULT:		// 读内部Flash的操作结果
			if(NRF_SUCCESS == p_evt->result)
			{

				FS_Print("NRF_FSTORAGE_EVT_READ_RESULT\n\r");
                #ifdef SEGGER_MODE
				SEGGER_RTT_printf(0, "fs erase success\n\r");
                #endif
			}
			break;

		case NRF_FSTORAGE_EVT_WRITE_RESULT:		// 写内部Flash的操作结果
			if(NRF_SUCCESS == p_evt->result)
			{
				FS_Print("NRF_WRITE: %d\t\taddr: 0x%.8x\t\tlen: %d\n\r", p_evt->result, p_evt->addr, p_evt->len);
			}
			break;

		case NRF_FSTORAGE_EVT_ERASE_RESULT:		// 擦内部Flash的操作结果
			if(NRF_SUCCESS == p_evt->result)
			{
				FS_Print("NRF_FSTORAGE_EVT_ERASE_RESULT\n\r");
			}
			break;
	}
}

//************************************************************************************************************
// 功能描述:   	初始化FS
// 函数参数:    无	
// 返回值  :	操作结果
//************************************************************************************************************
ret_code_t m_fs_init(void)
{
	m_fsc.p_api = &nrf_fstorage_sd;
	m_fsc.p_flash_info = &m_flash_info;

//	return NRF_SUCCESS;
	return nrf_fstorage_init(&m_fsc, &nrf_fstorage_sd, NULL);
}

//************************************************************************************************************
// 功能描述:   	从内部Flash读取数据
// 函数参数:    addr		->		内部Flash地址
//				buf			->		缓存区
//				len			->		数据长度
// 返回值  :	操作结果
//************************************************************************************************************ 
ret_code_t m_fs_read(uint32_t addr, uint8_t *buf, uint16_t	len)
{
#if 0
	// 读任意字节
	uint16_t	_cnt = len >> 2;
	uint8_t		_remain = len & 0x03;
	ret_code_t 	ret_code;
	if (_cnt)
	{
		ret_code = nrf_fstorage_read(&m_fsc, addr, buf, _cnt);
		if (ret_code != NRF_SUCCESS)
			return ret_code;
	}
	
	if (_remain)
	{
		uint8_t		tmp[4];
		ret_code = nrf_fstorage_read(&m_fsc, addr + (_cnt << 2), tmp, 1);
		if (ret_code != NRF_SUCCESS)
			return ret_code;
		
		memcpy(buf + (_cnt << 2), tmp, _remain);
	}
	return NRF_SUCCESS;
#else
	// 读取字节为4的倍数
	return nrf_fstorage_read(&m_fsc, addr, buf, len);
#endif
}


//************************************************************************************************************
// 功能描述:   	往内部Flash写数据
// 函数参数:    addr		->		内部Flash地址
//				buf			->		缓存区
//				len			->		数据长度，必须是4的倍数
// 返回值  :	操作结果
//************************************************************************************************************ 
ret_code_t m_fs_write(uint32_t addr, uint8_t *buf, uint16_t len)
{
	return nrf_fstorage_write(&m_fsc, addr, buf, len, NULL);
}

//************************************************************************************************************
// 功能描述:   	擦内部Flash
// 函数参数:    addr		->		内部Flash地址
//				pages		->		擦除多少页的数据
// 返回值  :	操作结果
//************************************************************************************************************
ret_code_t m_fs_erase(uint32_t addr, uint16_t pages)
{
	return nrf_fstorage_erase(&m_fsc, addr, pages, NULL);
}

//************************************************************************************************************
// 功能描述:   	内部Flash是否处于忙碌
// 函数参数:    无
// 返回值  :	无
//************************************************************************************************************
bool m_fs_is_busy(void)
{
	return nrf_fstorage_is_busy(&m_fsc);
}

#endif
