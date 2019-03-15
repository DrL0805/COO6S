#ifndef SPI1_CONTROL_H_
#define SPI1_CONTROL_H_

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "sr3_config.h"


#define SPI1_CONFIG_IRQ_PRIORITY    APP_IRQ_PRIORITY_LOW


__STATIC_INLINE void _kx022CS(unsigned char level)
{
    if (level)
    {
        nrf_gpio_pin_set(CONFIG_IO_GSENSOR_SPI_CS_PIN);
    }
    else
    {
        nrf_gpio_pin_clear(CONFIG_IO_GSENSOR_SPI_CS_PIN);
    }

    nrf_delay_us(2);
}

/*
* 双向传输，返回读字节，传递写字节参数(data)
*/
__STATIC_INLINE uint8_t spi1_transfer(uint8_t data)
{
    uint8_t ret = 0;

    NRF_SPI1->TXD = data;
    while (NRF_SPI1->EVENTS_READY == 0);
    NRF_SPI1->EVENTS_READY = 0;
    ret = NRF_SPI1->RXD;

    return ret;
}


__STATIC_INLINE void _spi1Write(unsigned char var)
{
    (void)spi1_transfer(var);
}

__STATIC_INLINE unsigned char _spi1Read(void)
{
    uint8_t var = 0;
    return spi1_transfer(var);
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

void spi1_init(void);

#endif

