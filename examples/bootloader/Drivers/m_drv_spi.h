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
// ��������:   	SPI��ʼ������
// ��������:    ��ǰ��Ҫ���õ�SPI�ܽ�ѡ��
// ����ֵ  :	���ý��
//************************************************************************************************************
uint8_t	m_spi_init(spi_select_t sel);

//************************************************************************************************************
// ��������:   	SPI��ʼ������
// ��������:    ��
// ����ֵ  :	���ý��
//************************************************************************************************************
uint8_t m_spi_hw_init(void);

//************************************************************************************************************
// ��������:   	SPI���ڽ��ա���������
// ��������:    tx_data 	    -> 	���ͻ�����
//				rx_data		    ->	���ջ�����
//				transfer_size	->	��������ݳ���
// ����ֵ  :	���ݽ�������    ->  (NRF_ERROR_TIMEOUT: ��ʱ, NRF_SUCCESS: ����)         
//************************************************************************************************************
uint8_t m_spi_rw(uint8_t *tx_data, uint8_t *rx_data, uint16_t transfer_size);


uint8_t m_spi_tx_rx_all(uint8_t *tx_data, uint8_t tx_len, uint8_t *rx_data, uint16_t rx_len);

#endif
