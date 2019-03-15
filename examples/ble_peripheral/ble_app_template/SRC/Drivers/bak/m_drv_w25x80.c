
#include "sdk_common.h"


#include "sr3_config.h"

#include <string.h>
#include "nrf_gpio.h"
#include "nrf_assert.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#include "m_drv_spi0.h"

#include "m_drv_w25x80.h"



#define TIMEOUT_WRITE_PAGE                  (20UL)      // 写flash超时, 单位 500us
#define TIMEOUT_READ                        (50UL)      // 读flash超时, 单位 500us
#define TIMEOUT_ERASE_SECTOR                (20)        // 擦扇区超时，单位 50ms
#define TIMEOUT_ERASE_CHIP                  (20)        // 擦芯片超时，单位 2s

#define BUSY                                false
#define READY                               true
#define FAIL                                false
#define OK                                  true

#define REG_WRITE_ENABLE                    (0x06)
#define REG_WRITE_DISABLE                   (0x04)

#define REG_READ_STATU_REG1                 (0x05)
#define REG_READ_STATU_REG2                 (0x35)
#define REG_WRITE_STATU_REG                 (0x01)

#define REG_PAGE_PROGRAM                    (0x02)
#define REG_SECTOR_ERASE                    (0x20)
#define REG_32K_BLOCK_ERASE                 (0x52)
#define REG_64K_BLOCK_ERASE                 (0xD8)
#define REG_CHIP_ERASE                      (0xC7)

#define REG_READ_DATA                       (0x03)
#define REG_FAST_READ_DATA                  (0x0B)

#define REG_WAKE_UP                         (0xAB)
#define REG_SLEEP                           (0xB9)
#define REG_READ_UID                        (0x4B)

#define GET_SEC_NUM(ADDR)                   ((ADDR) / SECTOR_SIZE)
#define GET_SEC_START_ADDR(SEC_NUM)         ((SEC_NUM) * SECTOR_SIZE)

#define CS_ENABLE_DELAY             1 // CS 线使能延时 US
#define POWERON_DELAY               10  // CS 线使能延时 MS


typedef enum
{
    SRB_BUSY    = (BIT_0), // 1表示忙
    SRB_WEL     = (BIT_1), // 1表示写使能，可以进行写操作
}STATUS_REG_BIT;

static bool w25x80_init_flag = false;

#define W25X80_CHECK_INIT_STATE(ret_data)       \
do                                              \
{                                               \
    if (!w25x80_init_flag)                      \
    {                                           \
        return (ret_data);                      \
    }                                           \
}while(0)

static __INLINE void enable_cs(void)
{
    nrf_gpio_pin_clear(CONFIG_IO_FLASH_SPI_CS_PIN);
    nrf_delay_us(CS_ENABLE_DELAY);
}

static __INLINE void disable_cs(void)
{
    nrf_gpio_pin_set(CONFIG_IO_FLASH_SPI_CS_PIN);
    nrf_delay_us(CS_ENABLE_DELAY);
}

static __INLINE uint8_t tran_read_byte(void)
{
    return spi0_transfer(0xff);
}

static __INLINE void tran_write_byte(uint8_t data)
{
    UNUSED_RETURN_VALUE(spi0_transfer(data));
}

static __INLINE void tran_init(void)
{
    nrf_gpio_cfg_output(CONFIG_IO_FLASH_SPI_CS_PIN);
    nrf_gpio_pin_set(CONFIG_IO_FLASH_SPI_CS_PIN);
    spi0_init();
}

static void write_reg(uint8_t reg)
{
    enable_cs();
    tran_write_byte(reg);
    disable_cs();
}

static uint8_t read_reg(uint8_t reg)
{
    uint8_t ret;

    enable_cs();
    tran_write_byte(reg);
    ret = tran_read_byte();
    disable_cs();

    return ret;
}

static __INLINE uint8_t get_status1(void)
{
    uint8_t ret = read_reg(REG_READ_STATU_REG1);
    return ret;
}

static bool wait_flash_ready(uint32_t timeout_500us)
{
    uint32_t cnt = timeout_500us;
    do
    {
        if((get_status1() & SRB_BUSY) != SRB_BUSY)
        {
            return READY;
        }

        if (cnt) cnt--;

        nrf_delay_us(500);

    }while(cnt > 0);

    if (cnt == 0)
    {
        return BUSY;
    }
    
    return READY;
}


static bool wait_flash_erase_ready(uint32_t timeout_50ms)
{
    uint32_t cnt = timeout_50ms;
    do
    {
        if((get_status1() & SRB_BUSY) != SRB_BUSY)
        {
            return READY;
        }

        if (cnt) cnt--;

        nrf_delay_ms(50);

    }while(cnt > 0);

    if (cnt == 0)
    {
        return BUSY;
    }
    
    return READY;
}


static bool enable_write(void)
{
    uint32_t us_cnt = 50;

    write_reg(REG_WRITE_ENABLE);

    while (us_cnt--)
    {
        if((get_status1() & SRB_WEL) == SRB_WEL)
        {
            return OK;
        }

        nrf_delay_us(500);
    }

    return FAIL;
}

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

    //bool ret = wait_flash_ready(TIMEOUT_READ);
    
    return OK; //(ret == READY) ? OK : FAIL;
}

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

static bool erase_sector(uint32_t addr)
{
    if (addr >= CHIP_SIZE)      return FAIL;

    if (FAIL == enable_write()) return FAIL;

    enable_cs();
    tran_write_byte(REG_SECTOR_ERASE);
    tran_write_byte((addr >> 16) & 0xff);
    tran_write_byte((addr >> 8)  & 0xff);
    tran_write_byte((addr >> 0)  & 0xff);
    disable_cs();

    bool ret = wait_flash_erase_ready(TIMEOUT_ERASE_SECTOR);
    
    return (ret == READY) ? OK : FAIL;
}

static __INLINE void wakeup(void)
{
    write_reg(REG_WAKE_UP);
}

static __INLINE void sleep(void)
{
    write_reg(REG_SLEEP);
}


void w25x80_read_uuid(uint8_t * buffer)
{
   // W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);

    wakeup();

    enable_cs();

    tran_write_byte(0X4B);
    tran_write_byte(0);
    tran_write_byte(0);
    tran_write_byte(0);
    tran_write_byte(0);

    for(uint8_t i = 0; i < 8; i++)
    {
        *(buffer + i) = tran_read_byte();
    }

    disable_cs();

    sleep();
}


ret_code_t w25x80_erase_sector(uint32_t addr)
{
//    NRF_LOG_RAW_INFO("[W25X80]: w25x80_erase_sector,addr : %08x\r\n",addr);
    W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);

    if (GET_SEC_NUM(addr) >= SECTOR_NUM_MAX)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    wakeup();

    bool ret = erase_sector(addr);

    sleep();
    
    return (ret == OK) ? NRF_SUCCESS : NRF_ERROR_TIMEOUT;
}

bool w25x80_test(void)
{
    const uint8_t buf[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t rx_buf[9] = {0};

    W25X80_CHECK_INIT_STATE(false);

    w25x80_erase_sector(0);


    wakeup();

    write_buf(0, (uint8_t *)buf, 9);
    memset(rx_buf, 0, 9);
    read_buf(0, rx_buf, 9);

    sleep();



    if (memcmp(buf, rx_buf, 9) == 0)
    {
        NRF_LOG_INFO("[W25X80]: Test ok\r\n");
        return true;
    }
    else
    {
        NRF_LOG_INFO("[W25X80]: Test fail\r\n");
        return false;
    }
}

ret_code_t w25x80_write(uint32_t addr, uint8_t *buf, uint32_t len)
{
    ASSERT(buf);

    W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);

    uint16_t last_byte_sec_num  = GET_SEC_NUM(addr + len);  // 待写入数据的最后一个字节(尾字节)所在的扇区

    /* 一次最多写一页 */
    if (len > PAGE_SIZE)
    {
        return NRF_ERROR_DATA_SIZE;
    }

    if (last_byte_sec_num > SECTOR_NUM_MAX) // 最后一个字节所在的扇区，不超出芯片容量，即表示待写入数据都落在芯片容量范围内
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    wakeup();

    bool ret = write_buf(addr, buf, len);

    sleep();

    return ((OK == ret) ? NRF_SUCCESS : NRF_ERROR_TIMEOUT);
}

ret_code_t w25x80_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    ASSERT(buf);

    W25X80_CHECK_INIT_STATE(NRF_ERROR_INVALID_STATE);

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

ret_code_t w25x80_init(void)
{
    tran_init();

    nrf_delay_ms(POWERON_DELAY);

    sleep();

    w25x80_init_flag = true;

    return NRF_SUCCESS;
}




