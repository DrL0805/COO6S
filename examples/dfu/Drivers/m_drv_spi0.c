#include "sdk_common.h"
#include "sr3_config.h"
//#include "boards.h"
#include "nrf_gpio.h"

#include "m_drv_spi0.h"

/*
* 双向传输，返回读字节，传递写字节参数(data)
*/
uint8_t spi0_transfer(uint8_t data)
{
    uint8_t ret = 0;

    NRF_SPI0->TXD = data;
    while (NRF_SPI0->EVENTS_READY == 0);
    NRF_SPI0->EVENTS_READY = 0;
    ret = NRF_SPI0->RXD;

    return ret;
}


void spi0_init(void)
{
    nrf_gpio_cfg_input(CONFIG_IO_FLASH_SPI_MISO_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(CONFIG_IO_FLASH_SPI_MOSI_PIN);
    nrf_gpio_cfg_output(CONFIG_IO_FLASH_SPI_SCK_PIN);

    NRF_SPI0->CONFIG    = (0 << 0) | (0 << 1) | (0 << 2);
    NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M4;
    NRF_SPI0->PSELSCK   = CONFIG_IO_FLASH_SPI_SCK_PIN;
    NRF_SPI0->PSELMOSI  = CONFIG_IO_FLASH_SPI_MOSI_PIN;
    NRF_SPI0->PSELMISO  = CONFIG_IO_FLASH_SPI_MISO_PIN;

    NRF_SPI0->ENABLE    = 1;
}

void spi0_disable_all_pins(void)
{
    nrf_gpio_cfg_input(CONFIG_IO_FLASH_SPI_CS_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(CONFIG_IO_FLASH_SPI_MISO_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(CONFIG_IO_FLASH_SPI_MOSI_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(CONFIG_IO_FLASH_SPI_SCK_PIN, NRF_GPIO_PIN_NOPULL);

    NRF_SPI0->ENABLE    = 0;
}




