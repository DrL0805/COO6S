

#include "sdk_common.h"
#include "nrf_delay.h"

#include "m_drv_kx022.h"
#include "m_drv_spi1.h"



/*
    出于某种不能告知的原因，应保持kx022的CS线空闲时为低电平
    需要开始通信时，将CS线拉高再拉低
*/

void kx022_reset(void)
{
    unsigned char data=0xFF;

    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1), BIN2CHAR(0,0,0,0,0,0,0,0)); // off
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL2),BIN2CHAR(1,0,1,1,1,1,1,1));  // reset
    _kx022CS(1);

    do{
        _kx022CS(0);
        data=_spi1ReadReg(READ(CNTL2));
        _kx022CS(1);
    }while((data&0x80)>0);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(1,0,0,0,0,0,0,0));  // on
    _kx022CS(1);

    _kx022CS(0);
}

// 25hz-stream-low power
void kx022_setup(void)
{
    unsigned char data=0xFF;
    _kx022CS(1);
    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(0,0,0,0,0,0,0,0));  // off
    _kx022CS(1);

    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL2),BIN2CHAR(1,0,1,1,1,1,1,1));  // reset
    _kx022CS(1);

    do{
        _kx022CS(0);
        data=_spi1ReadReg(READ(CNTL2));
        _kx022CS(1);
        nrf_delay_us(9);
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
    _kx022CS(0);
}

void kx022_set_25hz(void)
{
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

    _kx022CS(0);
}

void kx022_set_1hz(void)
{
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

    _kx022CS(0);
}

void kx022_set_slp(void)
{
    _kx022CS(1);
    _kx022CS(0);
    _spi1WriteReg(WRITE(CNTL1),BIN2CHAR(0,0,0,0,0,0,0,0));  // off
    _kx022CS(1);
    _kx022CS(0);
}
#if 0
__STATIC_INLINE unsigned char kx022_buflevel_get(void){
    unsigned char level;
    _kx022CS(1);
    _kx022CS(0);
    level=_spi1ReadReg(READ(BUF_STATUS_1));
    _kx022CS(1);
    _kx022CS(0);
    return level;
}

__STATIC_INLINE void kx022_buffer_read(char* ptr){
    _kx022CS(1);
    _kx022CS(0);
    _spi1Write(READ(BUF_READ));
    *ptr++=_spi1Read();
    *ptr++=_spi1Read();
    *ptr=_spi1Read();
    _kx022CS(1);
    _kx022CS(0);
}
#endif
__STATIC_INLINE unsigned char kx022_buffer_read_all(char* ptr){
    unsigned char level,index=0;
    _kx022CS(1);

    _kx022CS(0);
    level=_spi1ReadReg(READ(BUF_STATUS_1))/3;
    _kx022CS(1);

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

    _kx022CS(0);
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
        kx022_set_slp();
        GsensorSamplingRateTpy =0;
        //关掉gsensor读
    }
    if(tpy == 1)
    {
        //开读
        //采样率设置1hz
        kx022_set_1hz();
        GsensorSamplingRateTpy =1;
    }
    else
    {
        //开读
        //采样率设置快 25hz
        kx022_set_25hz();
        GsensorSamplingRateTpy =2;
    }
}

sAXIS GsensorDate[82]={0};

void GsensorStateCheck(unsigned char data)
{
    static char TimeCount =0;
    if(data == 0)
    {
        TimeCount ++;
        if(TimeCount > 20)
        {
            TimeCount = 0;
            kx022_reset();
            kx022_setup();
            kx022_set_1hz();
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

    memset((char *)GsensorDate, 0, sizeof(GsensorDate));
    DataLen = kx022_buffer_read_all((char *)GsensorDate);
    
//    for(int i =0;i<DataLen ;i++)
//    {
//        NRF_LOG_RAW_INFO("i %d, x %d, y %d, z %d\r\n", i, GsensorDate[i].x, GsensorDate[i].y, GsensorDate[i].z);
//    }
    
    GsensorStateCheck(DataLen);
    return DataLen ;
}

void g_sensor_init(void)
{
    spi1_init();

    kx022_reset();
    kx022_setup();
    kx022_set_slp();
}

void g_sensor_stop(void)
{
    kx022_reset();
    kx022_setup();
    kx022_set_slp();
}

void g_sensor_start(void)
{
    kx022_set_25hz();
    GsensorSamplingRateTpy =2;
}
