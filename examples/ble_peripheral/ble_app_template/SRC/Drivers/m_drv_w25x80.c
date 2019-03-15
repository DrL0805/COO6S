#include "m_drv_w25x80.h"
#include "m_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "sr3_config.h"
#include "m_drv_spi.h"
#include "includes.h"
#include "m_debug.h"

#if CONFIG_RTT_W25X80_ENABLED
#define W25X80_Print(M, ...)	SEGGER_Print("W25X80", M, ##__VA_ARGS__)
#else
#define W25X80_Print(M, ...)
#endif





#define TIMEOUT_WRITE_PAGE   				(50UL) 	// дFlash��λ��200us
#define TIMEOUT_READ		   				(50UL) 	// ��Flash��λ��200us
#define TIMEOUT_ERASE_SECTOR    			(200UL) // ����4KB��λ��1ms
#define TIMEOUT_ERASE_CHIP    				(1000UL)// ²ÁÐ¾Æ¬³¬Ê±

#define BUSY								false
#define READY								true
#define FAIL								false
#define OK									true

#define REG_WRITE_ENABLE                  	(0x06)
#define REG_WRITE_DISABLE                 	(0x04)

#define REG_READ_STATU_REG1               	(0x05)
#define REG_READ_STATU_REG2               	(0x35)
#define REG_WRITE_STATU_REG               	(0x01)

#define REG_PAGE_PROGRAM                  	(0x02)
#define REG_SECTOR_ERASE                  	(0x20)
#define REG_32K_BLOCK_ERASE               	(0x52)
#define REG_64K_BLOCK_ERASE               	(0xD8)
#define REG_CHIP_ERASE						(0xC7)

#define REG_READ_DATA                     	(0x03)
#define REG_FAST_READ_DATA                	(0x0B)

#define REG_WAKE_UP                       	(0xAB)
#define REG_SLEEP                         	(0xB9)
#define REG_READ_UID                      	(0x4B)

#define GET_SEC_NUM(ADDR)					((ADDR) / SECTOR_SIZE)
#define GET_SEC_START_ADDR(SEC_NUM)			((SEC_NUM) * SECTOR_SIZE)

typedef enum
{
	SRB_BUSY 	= (BIT_0), // 1±íÊ¾Ã¦
	SRB_WEL 	= (BIT_1), // 1±íÊ¾Ð´Ê¹ÄÜ£¬¿ÉÒÔ½øÐÐÐ´²Ù×÷
}STATUS_REG_BIT;

/* w25x80 ��ʼ����־ */
//static bool w25x80_init_flag = false;

//#define W25X80_CHECK_INIT_STATE(ret_data) 		\
//do												\
//{												\
//	if (spi_sel != SPI_SELECT_FLASH)			\
//	{											\
//		return (ret_data);						\
//	}											\
//}while(0)

//************************************************************************************************************
// ��������:   	ʹ��SPI����ʹ��Ƭѡ�ܽ�
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
static __INLINE void enable_cs(void)
{
	m_spi_enable();
    nrf_gpio_pin_clear(CONFIG_IO_FLASH_CS_PIN);
}

//************************************************************************************************************
// ��������:   	ʧ��Ƭѡ�ܽţ���ʧ��SPI
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
static __INLINE void disable_cs(void)
{
    nrf_gpio_pin_set(CONFIG_IO_FLASH_CS_PIN);
	m_spi_disable();
}

//************************************************************************************************************
// ��������:   	��SPI��ȡһ���ֽ�
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
static __INLINE uint8_t tran_read_byte(void)
{
    uint8_t rx;
    m_spi_rw(&rx, &rx, 1);
    return rx;
}

//************************************************************************************************************
// ��������:   	��SPIд��һ���ֽ�
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
static __INLINE void tran_write_byte(uint8_t data)
{
    m_spi_rw(&data, &data, 1);
}

//************************************************************************************************************
// ��������:   	����W25X80 SPI�͹ܽ�
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
static __INLINE void tran_init(void)
{
    nrf_gpio_cfg_output(CONFIG_IO_FLASH_CS_PIN);
    nrf_gpio_pin_set(CONFIG_IO_FLASH_CS_PIN);
    m_spi_init(SPI_SELECT_FLASH);
}

//************************************************************************************************************
// ��������:   	��������
// ��������:    cmd		->	�����͵�����
// ����ֵ  :	��        
//************************************************************************************************************
static void write_cmd(uint8_t cmd)
{
	enable_cs();
    tran_write_byte(cmd);
    disable_cs();
}

//************************************************************************************************************
// ��������:   	���Ĵ���
// ��������:    reg		->	��Ҫ��ȡ�ļĴ���
// ����ֵ  :	��ȡ�ļĴ���ֵ        
//************************************************************************************************************
static uint8_t read_reg(uint8_t reg)
{
	uint8_t temp[2];
    uint8_t buf[2];
    
    buf[0] = reg;

    enable_cs();
    m_spi_rw(buf, temp, 2);

    disable_cs();

    return temp[1];
}

//************************************************************************************************************
// ��������:   	��״̬�Ĵ���
// ��������:    ��
// ����ֵ  :	״ֵ̬        
//************************************************************************************************************
static __INLINE uint8_t get_status1(void)
{
    uint8_t ret = read_reg(REG_READ_STATU_REG1);
    return ret;
}

//************************************************************************************************************
// ��������:   	�ȴ�W25X80����
// ��������:    timeout_ms		->	��ʱʱ��
// ����ֵ  :	�Ƿ����(BUSY/READY)        
//************************************************************************************************************
static bool wait_flash_erase_ready(uint32_t timeout_1ms)
{
    uint32_t cnt = timeout_1ms;
    do
    {
        if((get_status1() & SRB_BUSY) != SRB_BUSY)
        {
            return READY;
        }

        if (cnt) cnt--;

        nrf_delay_ms(1);

    }while(cnt > 0);

    if (cnt == 0)
    {
        return BUSY;
    }
    
    return READY;
}

//************************************************************************************************************
// ��������:   	�ȴ�W25X80����
// ��������:    timeout_ms		->	��ʱʱ��
// ����ֵ  :	�Ƿ����(BUSY/READY)        
//************************************************************************************************************
static bool wait_flash_ready(uint32_t timeout_ms)
{
	uint32_t cnt = timeout_ms;
    do
    {
        if((get_status1() & SRB_BUSY) != SRB_BUSY)
        {
        	return READY;
        }

		if (cnt) cnt--;
	
		nrf_delay_us(200);		// �ӿ���Ӧ�ٶȣ���������ʱ�䣬���͹���

    } while(cnt > 0);

    if (cnt == 0)
    {
        W25X80_Print("flash busy");
    	return BUSY;
    }
    
    return READY;
}

//************************************************************************************************************
// ��������:   	W25X80дʹ��
// ��������:    ��
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
static bool enable_write(void)
{
	uint32_t ms_cnt = 50;

    write_cmd(REG_WRITE_ENABLE);

    while (ms_cnt--)
    {
        if((get_status1() & SRB_WEL) == SRB_WEL)
        {
        	return OK;
        }

        nrf_delay_us(200);
    }

    W25X80_Print("enable write err");
   	return FAIL;
}

//************************************************************************************************************
// ��������:   	��W25X80������
// ��������:    addr		->	�����ݵ���ʼ��ַ
//				buff		->	���ݻ�����
//				len			->	���ݳ���
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
static bool read_buf(uint32_t addr, uint8_t *buff, uint32_t len)
{
    ASSERT(buff);
    
    if (FAIL == enable_write()) return FAIL;
    
    enable_cs();
	
    tran_write_byte(REG_READ_DATA);
    tran_write_byte((addr >> 16) & 0xff);
    tran_write_byte((addr >> 8)  & 0xff);
    tran_write_byte((addr >> 0)  & 0xff);

    for(uint32_t i = 0; i < len; i++)
    {
        buff[i] = tran_read_byte();
    }
    disable_cs();

//    bool ret = wait_flash_ready(TIMEOUT_READ);
    
//    return (ret == READY) ? OK : FAIL;
	return OK;
}

//************************************************************************************************************
// ��������:   	��W25X80д����
// ��������:    addr		->	д���ݵ���ʼ��ַ
//				buff		->	���ݻ�����
//				len			->	���ݳ���
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
static bool write_buf(uint32_t addr, uint8_t *data, uint32_t len)
{
    ASSERT(data);
    
    if (FAIL == enable_write()) return FAIL;
    
    enable_cs();
    
    tran_write_byte(REG_PAGE_PROGRAM);
    tran_write_byte((addr >> 16) & 0xff);
    tran_write_byte((addr >> 8)  & 0xff);
    tran_write_byte((addr >> 0)  & 0xff);

    for(uint32_t i = 0; i < len; i++)
    {
        tran_write_byte(data[i]);
    }
    disable_cs();    

    bool ret = wait_flash_ready(TIMEOUT_WRITE_PAGE);
    
    return (ret == READY) ? OK : FAIL;
}

//************************************************************************************************************
// ��������:   	����һ������(4KB)
// ��������:    addr		->	�������ݵ���ʼ��ַ
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
static bool erase_sector(uint32_t addr)
{
	if (addr >= CHIP_SIZE)      return FAIL;

    if (FAIL == enable_write()) return FAIL;
    
    uint8_t  temp[4]; 
    
    temp[0] = REG_SECTOR_ERASE;
    temp[1] = (addr >> 16) & 0xff;
    temp[2] = (addr >> 8)  & 0xff;
    temp[3] = (addr >> 0)  & 0xff;

    enable_cs();
    
    m_spi_rw(temp, temp, 4);
    disable_cs();

    bool ret = wait_flash_erase_ready(TIMEOUT_ERASE_SECTOR);
    
    return (ret == READY) ? OK : FAIL;
}

//************************************************************************************************************
// ��������:   	��������оƬ
// ��������:    ��
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
static bool erase_chip(void)
{
    if (FAIL == enable_write()) return FAIL;

    enable_cs();
    tran_write_byte(REG_CHIP_ERASE);
    disable_cs();

    bool ret = wait_flash_erase_ready(TIMEOUT_ERASE_SECTOR);
    
    return (ret == READY) ? OK : FAIL;
}

//************************************************************************************************************
// ��������:   	����W25X80
// ��������:    ��
// ����ֵ  :	��       
//************************************************************************************************************
static __INLINE void wakeup(void)
{
	write_cmd(REG_WAKE_UP);
}

//************************************************************************************************************
// ��������:   	W25X80����
// ��������:    ��
// ����ֵ  :	��        
//************************************************************************************************************
static __INLINE void sleep(void)
{
	write_cmd(REG_SLEEP);
}

//************************************************************************************************************
// ��������:   	�����ⲿFlash������4KB����������
// ��������:    ��
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
ret_code_t w25x80_erase_sector(uint32_t addr)
{
//	W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_FLASH))
	{
		W25X80_Print("bbbbbbbbbbbb\n\r");
		return NRF_ERROR_INVALID_PARAM;
	}

	if (GET_SEC_NUM(addr) >= SECTOR_NUM_MAX)
	{
		return NRF_ERROR_INVALID_PARAM;
	}

	wakeup();

    bool ret = erase_sector(addr);

	sleep();
    
    return (ret == OK) ? NRF_SUCCESS : NRF_ERROR_TIMEOUT;
}

//************************************************************************************************************
// ��������:   	�����ⲿFlash����������оƬ����������
// ��������:    ��
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
ret_code_t w25x80_erase_chip(void)
{
//	W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_FLASH))
	{
		W25X80_Print("cccccccccccccccc\n\r");
		return NRF_ERROR_INVALID_PARAM;
	}

	wakeup();

    bool ret = erase_chip();

	sleep();
    
    return (ret == OK) ? NRF_SUCCESS : NRF_ERROR_TIMEOUT;
}

//************************************************************************************************************
// ��������:   	�����ⲿFlash��д�����ݺ���������
// ��������:    addr		->	д���ݵ���ʼ��ַ
//				buff		->	���ݻ�����
//				len			->	���ݳ���
// ����ֵ  :	�������(OK/FAIL)         
//************************************************************************************************************
ret_code_t w25x80_write(uint32_t addr, uint8_t *buf, uint32_t len)
{
	ASSERT(buf);

//	W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_FLASH))
	{
		W25X80_Print("aaaaaaaaaaaaa\n\r");
		return NRF_ERROR_INVALID_PARAM;
	}

	uint16_t last_byte_sec_num  = GET_SEC_NUM(addr + len);  // 

	/* Ò»´Î×î¶àÐ´Ò»Ò³ */
	if (len > PAGE_SIZE)
	{
		return NRF_ERROR_DATA_SIZE;
	}

	if (last_byte_sec_num > SECTOR_NUM_MAX) // 
	{
		return NRF_ERROR_INVALID_PARAM;
	}

	wakeup();

	bool ret = write_buf(addr, buf, len);

	sleep();


	return ((OK == ret) ? NRF_SUCCESS : NRF_ERROR_TIMEOUT);
}

//************************************************************************************************************
// ��������:   	�����ⲿFlash����W25X80�����ݺ���������
// ��������:    addr		->	�����ݵ���ʼ��ַ
//				buff		->	���ݻ�����
//				len			->	���ݳ���
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
ret_code_t w25x80_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
	ASSERT(buf);

//	W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_FLASH))
	{
		W25X80_Print("dddddddddd\n\r");
		return NRF_ERROR_INVALID_PARAM;
	}

	uint16_t last_byte_sec_num  = GET_SEC_NUM(addr + len);

	if (last_byte_sec_num > SECTOR_NUM_MAX)
	{
		return NRF_ERROR_INVALID_PARAM;
	}

	wakeup();

	bool ret = read_buf(addr, buf, len);

	sleep();

	return ((OK == ret) ? NRF_SUCCESS : NRF_ERROR_TIMEOUT);
}

//************************************************************************************************************
// ��������:   	��ʼ��SPI�͹ܽţ�Ȼ������
// ��������:    addr		->	�����ݵ���ʼ��ַ
//				buff		->	���ݻ�����
//				len			->	���ݳ���
// ����ֵ  :	�������(OK/FAIL)        
//************************************************************************************************************
ret_code_t w25x80_init(void)
{

	if (spi_sel == SPI_SELECT_FLASH)
	{
		return NRF_SUCCESS;
	}
	
	tran_init();

	sleep();

	return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	��ȡ�ⲿFlash��ID
// ��������:    ��
// ����ֵ  :	������IDֵ(2Byte)        
//************************************************************************************************************
uint16_t w25x80_read_id(void)
{
	uint8_t		buf[6];
	
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_FLASH))
	{
		return 0xFFFF;
	}
	
	buf[0] = FLASH_CMD_READ_ID;
	buf[1] = FLASH_CMD_DUMMY;
	buf[2] = FLASH_CMD_DUMMY;
	buf[3] = FLASH_CMD_DUMMY;
	buf[4] = FLASH_CMD_DUMMY;
	buf[5] = FLASH_CMD_DUMMY;

    wakeup();

    enable_cs();
    m_spi_rw(buf, buf, 6);
    disable_cs();
    
    sleep();
    W25X80_Print("Id: 0x%.4x\n\r", (buf[4] << 8) | buf[5]);
	return (buf[4] << 8) | buf[5];
}

//************************************************************************************************************
// ��������:   	��ȡ�ⲿFlash�����к�
// ��������:    buff		->		���кŻ�����
// ����ֵ  :	��       
//************************************************************************************************************
void w25x80_read_uuid(uint8_t *buff)
{
//   W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_FLASH))
	{
         W25X80_Print("[W25X80]: w25x80_read_uuid false\r\n");
		memset(buff, 0xff, 8);
		return;
	}
	
    wakeup();

    enable_cs();

    tran_write_byte(0X4B);
    tran_write_byte(0);
    tran_write_byte(0);
    tran_write_byte(0);
    tran_write_byte(0);
    
    LOG_DRAW_Print("Read UUID: ");

    for(uint8_t i = 0; i < 8; i++)
    {
        *(buff + i) = tran_read_byte();
        
        LOG_DRAW_Print("%.2x ", *(buff + i));
    }
    
    LOG_DRAW_Print("\n\r");

    disable_cs();

    sleep();

}


bool w25x80_test(void)
{
    const uint8_t buf[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t rx_buf[9] = {0};



    w25x80_erase_sector(0);


    wakeup();

    write_buf(0, (uint8_t *)buf, 9);
    memset(rx_buf, 0, 9);
    read_buf(0, rx_buf, 9);

    sleep();



    if (memcmp(buf, rx_buf, 9) == 0)
    {
        W25X80_Print("[W25X80]: Test ok\r\n");
        return true;
    }
    else
    {
        W25X80_Print("[W25X80]: Test fail\r\n");
        return false;
    }
}
