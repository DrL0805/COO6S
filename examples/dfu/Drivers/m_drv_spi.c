#include "m_drv_spi.h"
#include "app_error.h"
#include "m_debug.h"
#include "nrf_gpio.h"
#include "sr3_config.h"

#if CONFIG_RTT_SPI_ENABLED
#define SPI_Print(M, ...)	SEGGER_Print("SPI", M, ##__VA_ARGS__)
#else
#define SPI_Print(M, ...)
#endif

uint8_t	spi_sel = SPI_SELECT_MAX;

//static bool m_spi_initial = false;
//static bool m_spi0_initial = false;
/* 用寄存器完成SPI的收、发 */
#if (SPI_TYPE_SEL == SPI_TYPE_SAMPLE)

//************************************************************************************************************
// 功能描述:   	SPI管脚选择
// 函数参数:    给定选择的类型
// 返回值  :	配置结果
//************************************************************************************************************

 

static uint8_t	m_spi_select(spi_select_t sel)
{
	m_spi_disable();
	switch(sel)
	{
		case SPI_SELECT_FLASH:
			NRF_SPIX->PSEL.SCK    	= CONFIG_IO_FLASH_SPI_SCK_PIN;
			NRF_SPIX->PSEL.MOSI 	= CONFIG_IO_FLASH_SPI_MOSI_PIN;
			NRF_SPIX->PSEL.MISO 	= CONFIG_IO_FLASH_SPI_MISO_PIN;
			NRF_SPIX->FREQUENCY= SPI_FREQUENCY_FREQUENCY_M8;		// 8M bit	MC3630最高只支持4M，且未经验证，所以配置为2M比较妥当
			NRF_SPIX->CONFIG =  (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos)|
						(SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos)|
						(SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
			break;
		
		case SPI_SELECT_ACC:
			NRF_SPIX->PSEL.SCK 	    = CONFIG_IO_GSENSOR_SPI_SCK_PIN;
			NRF_SPIX->PSEL.MOSI 	= CONFIG_IO_GSENSOR_SPI_MOSI_PIN;
			NRF_SPIX->PSEL.MISO 	= CONFIG_IO_GSENSOR_SPI_MISO_PIN;
			NRF_SPIX->FREQUENCY= SPI_FREQUENCY_FREQUENCY_M8;		// 8M bit	MC3630最高只支持4M，且未经验证，所以配置为2M比较妥当
			NRF_SPIX->CONFIG =  (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos)|
						(SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos)|
						(SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);
			break;
		
		default:
			return NRF_ERROR_INVALID_PARAM;
	}
	
	NRF_SPIX->EVENTS_READY = 0U;
	m_spi_enable();
	spi_sel = sel;
	return NRF_SUCCESS;
}

//************************************************************************************************************
// 功能描述:   	SPI初始化配置
// 函数参数:    当前需要配置的SPI管脚选择
// 返回值  :	配置结果
//************************************************************************************************************
uint8_t	m_spi_init(spi_select_t sel)
{

	if (sel != spi_sel)
	{
		// 需要重新进行初始化		
		return m_spi_select(sel);
	} 
	
	return NRF_SUCCESS;
}

//************************************************************************************************************
// 功能描述:   	SPI初始化配置
// 函数参数:    无
// 返回值  :	配置结果
//************************************************************************************************************
//uint8_t m_spi_hw_init(void)
//{	
//    
//    
//    return NRF_SUCCESS;
//}


//************************************************************************************************************
// 功能描述:   	SPI用于接收、发送数据
// 函数参数:    tx_data 	    -> 	发送缓存区
//				rx_data		    ->	接收缓存区
//				transfer_size	->	传输的数据长度
// 返回值  :	数据交互反馈    ->  (NRF_ERROR_TIMEOUT: 超时, NRF_SUCCESS: 正常)         
//************************************************************************************************************
uint8_t m_spi_rw(uint8_t *tx_data, uint8_t *rx_data, uint16_t transfer_size)
{
	uint32_t counter = 0;
    uint16_t number_of_txd_bytes = 0;
	
	while(number_of_txd_bytes < transfer_size)
	{
		counter = 0;
		NRF_SPIX->TXD = (uint32_t)(tx_data[number_of_txd_bytes]);	
		/* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
		while ((NRF_SPIX->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER)) 
		{
			counter++;
		}
		
		if(TIMEOUT_COUNTER == counter)
		{
			/* timed out, disable slave (slave select active low) and return with error */
			return NRF_ERROR_TIMEOUT;
		}
		else
		{
			/* clear the event to be ready to receive next messages */
			NRF_SPIX->EVENTS_READY = 0U;
		}
		rx_data[number_of_txd_bytes++] = (uint8_t)NRF_SPIX->RXD;
	}

	return NRF_SUCCESS;
}

#elif (SPI_TYPE_SEL == SPI_TYPE_HARD)
/* 调用底层库函数完成SPI的收、发 */
#include "nrf_drv_spi.h"

//SPI TWI共享内存，同时使用，需要开启不同序号的2个，比如TWI0 SPI1
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(1);  /**< SPI instance. */
static volatile bool spi_xfer_done = false;  /**< Flag used to indicate that SPI instance completed the transfer. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
//    SPI_Print("Transfer completed.");
}

uint8_t m_spi_hw_init(void)
{
    uint8_t     ret;
    if(true == m_spi_initial)
    {
        return NRF_SUCCESS;
    }
    
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
//    spi_config.ss_pin   = SPI_CS_FLASH_Pin;
    spi_config.miso_pin = SPI_SDI_Pin;
    spi_config.mosi_pin = SPI_SDO_Pin;
    spi_config.sck_pin  = SPI_SCLK_Pin;
    
    ret = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    if(NRF_SUCCESS == ret)
    {
        m_spi_initial = true;
    }
    
    return ret;
}

void m_spi_enable(void)
{
    m_spi_hw_init();
}

void m_spi_disable(void)
{
    nrf_drv_spi_uninit(&spi);
}

uint8_t m_spi_tx_rx(uint8_t *tx_data, uint8_t *rx_data, uint16_t transfer_size)
{
    uint8_t     ret;
    
    spi_xfer_done = false;
    ret = nrf_drv_spi_transfer(&spi, tx_data, transfer_size, rx_data, transfer_size);
    
    if(ret != NRF_SUCCESS)
    {
        SPI_Print("SPI Err: %d", ret);
        return ret;
    }
    
    while(false == spi_xfer_done);
    
    return ret;
}

uint8_t m_spi_tx_rx_all(uint8_t *tx_data, uint8_t tx_len, uint8_t *rx_data, uint16_t rx_len)
{
    uint8_t     ret;
    
    spi_xfer_done = false;
    ret = nrf_drv_spi_transfer(&spi, tx_data, tx_len, rx_data, rx_len);
    
    if(ret != NRF_SUCCESS)
    {
        SPI_Print("SPI Err: %d", ret);
        return ret;
    }
    
    while(false == spi_xfer_done);
    
    return ret;
}


#endif

