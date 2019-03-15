#include "sdk_common.h"


#include "m_drv_spi1.h"

void spi1_init(void)
{
    nrf_gpio_cfg_input(CONFIG_IO_GSENSOR_SPI_MISO_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_output(CONFIG_IO_GSENSOR_SPI_CS_PIN);
    nrf_gpio_cfg_output(CONFIG_IO_GSENSOR_SPI_SCK_PIN);
    nrf_gpio_cfg_output(CONFIG_IO_GSENSOR_SPI_MOSI_PIN);

    NRF_SPI1->CONFIG    = (0 << 0) | (0 << 1) | (0 << 2);
    NRF_SPI1->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M1;
    NRF_SPI1->PSELSCK   = CONFIG_IO_GSENSOR_SPI_SCK_PIN;
    NRF_SPI1->PSELMOSI  = CONFIG_IO_GSENSOR_SPI_MOSI_PIN;
    NRF_SPI1->PSELMISO  = CONFIG_IO_GSENSOR_SPI_MISO_PIN;

    NRF_SPI1->ENABLE    = 1;
    
    _kx022CS(0);
}

void spi1_uninit(void)
{
    nrf_gpio_cfg_input(CONFIG_IO_GSENSOR_SPI_MISO_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(CONFIG_IO_GSENSOR_SPI_CS_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(CONFIG_IO_GSENSOR_SPI_SCK_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(CONFIG_IO_GSENSOR_SPI_MOSI_PIN, NRF_GPIO_PIN_NOPULL);

    NRF_SPI1->ENABLE    = 0;
}
