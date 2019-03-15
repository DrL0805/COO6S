#ifndef DRV_SPI_H__
#define DRV_SPI_H__

//#include "boards.h"


uint8_t spi0_transfer(uint8_t data);

void spi0_init(void);

void spi0_disable_all_pins(void);


#endif // DRV_SPI_H__



