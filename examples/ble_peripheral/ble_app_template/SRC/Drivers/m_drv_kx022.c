#include "m_drv_kx022.h"
#include "m_drv_spi.h"
#include "includes.h"



sAXIS GsensorDate[82]={0};

#if (ACC_DEVICE_TYPE == ACC_KX022)


__STATIC_INLINE void _kx022_gpio_init(void)
{
	nrf_gpio_cfg_output(CONFIG_IO_ACCEL_CS_PIN);
    nrf_gpio_pin_clear(CONFIG_IO_ACCEL_CS_PIN);
}

__STATIC_INLINE void _kx022CS(unsigned char level)
{
    if (level)
    {
        nrf_gpio_pin_set(CONFIG_IO_ACCEL_CS_PIN);
    }
    else
    {
        nrf_gpio_pin_clear(CONFIG_IO_ACCEL_CS_PIN);
    }

    nrf_delay_us(2);
}

/*
* 双向传输，返回读字节，传递写字节参数(data)
*/
//__STATIC_INLINE uint8_t spi0_transfer(uint8_t data)
//{
//    uint8_t ret = 0;
//    static uint16_t  cnt =0; 
//    NRF_SPI1->TXD = data;
//    while ( NRF_SPI1->EVENTS_READY == 0);
//    NRF_SPI1->EVENTS_READY = 0;
//    ret = NRF_SPI1->RXD;

//    return ret;
//}


__STATIC_INLINE void _spi1Write(unsigned char var)
{
	m_spi_rw(&var, &var, 1);
//	spi0_transfer(var);
}


__STATIC_INLINE unsigned char _spi1Read(void)
{
    uint8_t var = 0;
	m_spi_rw(&var, &var, 1);
	return var;
//	return spi0_transfer(0x00);
}


__STATIC_INLINE void _spi1WriteReg(unsigned char addr,unsigned char val)
{
    _spi1Write(addr);
    _spi1Write(val);
}

__STATIC_INLINE unsigned char _spi1ReadReg(unsigned char addr)
{
    _spi1Write(addr);
    return _spi1Read();
}

/*
    出于某种不能告知的原因，应保持kx022的CS线空闲时为低电平
    需要开始通信时，将CS线拉高再拉低
*/

void g_sensor_reset(void)
{
    unsigned char data=0xFF;

	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_ACC))
	{
		return;
	}
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1), BIN2CHAR(0,0,0,0,0,0,0,0)); // off
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL2),BIN2CHAR(1,0,1,1,1,1,1,1));  // reset
    _kx022CS(1);

    volatile uint16_t cnt = 0;
    do{
        _kx022CS(0);
        data=_spi1ReadReg(READ(CNTL2));
        _kx022CS(1);
         nrf_delay_us(6);
        if(cnt++ >= 2000)
        {
            break;
        }
    }while((data&0x80)>0);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(1,0,0,0,0,0,0,0));  // on
    _kx022CS(1);

//    _kx022CS(0);
}

// 25hz-stream-low power
void g_sensor_setup(void)
{
    unsigned char data=0xFF;
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_ACC))
	{
		return;
	}
    _kx022CS(1);
    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(0,0,0,0,0,0,0,0));  // off
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL2),BIN2CHAR(1,0,1,1,1,1,1,1));  // reset
    _kx022CS(1);
    volatile uint16_t cnt = 0;
    do{
        _kx022CS(0);
        data=_spi1ReadReg(READ(CNTL2));
        _kx022CS(1);
        nrf_delay_us(6);
        if(cnt++ >= 2000)
        {
            break;
        }
    }while((data&0x80)>0);

    _kx022CS(0);
    _spi1WriteReg(WRITE(ODCNTL),BIN2CHAR(1,0,0,0,0,0,0,1)); // 25 hz
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(LP_CNTL),BIN2CHAR(0,0,1,0,1,0,1,1));    // avr 4 sample
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(BUF_CNTL2),BIN2CHAR(1,0,0,0,0,0,0,1));  // 8bit stream mode
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(BUF_CLEAR),BIN2CHAR(0,0,0,0,0,0,0,0));  // buf clear
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(1,0,0,0,0,0,0,0));  // on
    _kx022CS(1);
//    _kx022CS(0);
}

void g_sensor_25hz(void)
{
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_ACC))
	{
		return;
	}
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(0,0,0,0,0,0,0,0));  // off
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(BUF_CNTL2),BIN2CHAR(1,0,0,0,0,0,0,1));  // 8bit stream mode
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(BUF_CLEAR),BIN2CHAR(0,0,0,0,0,0,0,0));  // buf clear
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(ODCNTL),BIN2CHAR(1,0,0,0,0,0,0,1)); // 25 hz
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(1,0,0,0,0,0,0,0));  // on
    _kx022CS(1);

//    _kx022CS(0);
}

void g_sensor_1hz(void)
{
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_ACC))
	{
		return;
	}
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(0,0,0,0,0,0,0,0));  // off
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(BUF_CNTL2),BIN2CHAR(1,0,0,0,0,0,0,1));  // 8bit stream mode
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(BUF_CLEAR),BIN2CHAR(0,0,0,0,0,0,0,0));  // buf clear
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(ODCNTL),BIN2CHAR(1,0,0,0,1,0,1,0)); // 3 hz
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(1,0,0,0,0,0,0,0));  // on
    _kx022CS(1);

//    _kx022CS(0);
}

uint8_t g_sensor_buffer_read_all(char* ptr)
{
    uint8_t level,index=0;
	uint32_t 	time = NRF_RTC1->COUNTER;
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_ACC))
	{
		return 0;
	}
//	m_spi0_enable();
	
    _kx022CS(1);

    _kx022CS(0);
    level =_spi1ReadReg(READ(BUF_STATUS_1))/3;
    _kx022CS(1);

//	SEGGER_RTT_printf(0, "timexxx: %d\n\r", level);
    _kx022CS(0);
    _spi1Write(READ(BUF_READ));
    if(level > 82 ) level =82;
    while(index++<level) {
        *ptr++=_spi1Read();
        *ptr++=_spi1Read();
        *ptr++=_spi1Read();
        *ptr++=1;
        //ptr+=sizeof(sAXIS)-3;
    }
    _kx022CS(1);
//	SEGGER_RTT_printf(0, "time2(%d): %d\n\r", level, NRF_RTC1->COUNTER);

//    _kx022CS(0);
//	SEGGER_RTT_printf(0, "rtc111(0.1ms): %d\n\r", (NRF_RTC1->COUNTER - time) * 10000 / 32768);	// 精确到0.1ms
//	SEGGER_RTT_printf(0, "test ending...\n\r");
	
//	m_spi0_disable();
    return level;
}



static unsigned char GsensorSamplingRateTpy = 2;
unsigned char GetGsensorSamplingRateTpy(void)
{
    return GsensorSamplingRateTpy;
}

void GsensorSwitch(unsigned char tpy)
{
    if(tpy ==0 )
    {
        g_sensor_sleep();
        GsensorSamplingRateTpy =0;
        //??gsensor?
    }
    if(tpy == 1)
    {
        //开读
        //采样率设置1hz
        g_sensor_1hz();
        GsensorSamplingRateTpy =1;
    }
    else
    {
        //??
        //?????? 25hz
        g_sensor_25hz();
        GsensorSamplingRateTpy =2;
    }
}

void GsensorStateCheck(unsigned char data)
{
    static char TimeCount =0;
    if(data == 0)
    {
        TimeCount ++;
        if(TimeCount > 10)
        {
            TimeCount = 0;
            g_sensor_reset();
            g_sensor_setup();
            g_sensor_1hz();
			
			
            GsensorSamplingRateTpy =1;
        }
    }
    else
    {
        TimeCount = 0;
    }
}

unsigned char  UpdataGsensorData(void)
{
    unsigned char DataLen = 0;

//	uint32_t		time;
	
    memset((char *)GsensorDate, 0, sizeof(GsensorDate));
	
//	time = NRF_RTC1->COUNTER;
    DataLen = g_sensor_buffer_read_all((char *)GsensorDate);
    
    
    GsensorStateCheck(DataLen);
    return DataLen ;
}

//void GsensorInit(void)
//{	
////	Mc36xx_Init();
//}

void g_sensor_stop(void)
{
    g_sensor_reset();
    g_sensor_setup();
    g_sensor_sleep();
}

void g_sensor_start(void)
{
    g_sensor_25hz();
    GsensorSamplingRateTpy =2;
}


void g_sensor_sleep(void)
{
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_ACC))
	{
		return;
	}
    _kx022CS(1);
    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(0,0,0,0,0,0,0,0));  // off
    _kx022CS(1);
//    _kx022CS(0);
}

void g_sensor_init(void)
{
	_kx022_gpio_init();
	
	if (NRF_SUCCESS != m_spi_init(SPI_SELECT_ACC))
	{
		return;
	}
	
	g_sensor_reset();
    g_sensor_setup();
    g_sensor_sleep();
}


#endif
