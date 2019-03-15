#ifndef __SPI_HARDWARE_H__
#define __SPI_HARDWARE_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sr3_config.h"
#include "nrf52.h"
//#include "bsp_cfg.h"

#define SPI_TYPE_SAMPLE     1
#define SPI_TYPE_HARD       2


#define SPI_TYPE_SEL      SPI_TYPE_SAMPLE  

#define TIMEOUT_COUNTER          	0x3000UL 

#if (SPI_TYPE_SEL == SPI_TYPE_SAMPLE)

#define NRF_SPIX		NRF_SPI1

#define m_spi_enable();		{NRF_SPIX->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);}
#define m_spi_disable();		{NRF_SPIX->ENABLE = (SPI_ENABLE_ENABLE_Disabled<< SPI_ENABLE_ENABLE_Pos);}


//#define m_spi1_enable();		{NRF_SPI1->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);}
//#define m_spi1_disable();		{NRF_SPI1->ENABLE = (SPI_ENABLE_ENABLE_Disabled<< SPI_ENABLE_ENABLE_Pos);}
#else
void m_spi_disable(void);
void m_spi_enable(void);
#endif

typedef enum
{
	SPI_SELECT_FLASH,
	SPI_SELECT_ACC,
	SPI_SELECT_MAX,
}spi_select_t;

extern uint8_t	spi_sel;

//************************************************************************************************************
// 功能描述:   	SPI初始化配置
// 函数参数:    当前需要配置的SPI管脚选择
// 返回值  :	配置结果
//************************************************************************************************************
uint8_t	m_spi_init(spi_select_t sel);

//************************************************************************************************************
// 功能描述:   	SPI初始化配置
// 函数参数:    无
// 返回值  :	配置结果
//************************************************************************************************************
uint8_t m_spi_hw_init(void);

//************************************************************************************************************
// 功能描述:   	SPI用于接收、发送数据
// 函数参数:    tx_data 	    -> 	发送缓存区
//				rx_data		    ->	接收缓存区
//				transfer_size	->	传输的数据长度
// 返回值  :	数据交互反馈    ->  (NRF_ERROR_TIMEOUT: 超时, NRF_SUCCESS: 正常)         
//************************************************************************************************************
uint8_t m_spi_rw(uint8_t *tx_data, uint8_t *rx_data, uint16_t transfer_size);


uint8_t m_spi_tx_rx_all(uint8_t *tx_data, uint8_t tx_len, uint8_t *rx_data, uint16_t rx_len);

#endif
