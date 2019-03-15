#include "m_drv_i2c.h"

//TWI0
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(0);
static bool m_i2c_initial = false;

//************************************************************************************************************
// ��������:   	��ʼ��IIC���ɷ�ֹ�ظ���ʼ��
// ��������:    ��
// ����ֵ  :	��
//************************************************************************************************************
ret_code_t i2c_init(void)
{
    ret_code_t  ret;
    
    if(true == m_i2c_initial)
    {
        return NRF_SUCCESS;
    }
    
    const nrf_drv_twi_config_t config =
    {
       .scl                = CONFIG_IO_IIC_SCL_PIN,
       .sda                = CONFIG_IO_IIC_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOW,
       .clear_bus_init     = false
    };

    ret = nrf_drv_twi_init(&m_twi_master, &config, NULL, NULL);

    if (NRF_SUCCESS == ret)
    {
        nrf_drv_twi_enable(&m_twi_master);
        m_i2c_initial = true;
    }

    return ret;
}

//************************************************************************************************************
// ��������:   	ע��IIC
// ��������:    ��
// ����ֵ  :	NRF_SUCCESS -> 	�ɹ�ע��
//************************************************************************************************************
ret_code_t i2c_uninit(void)
{
    
    if(false == m_i2c_initial)
    {
        return NRF_SUCCESS;
    }

    nrf_drv_twi_uninit(&m_twi_master);
     
//    nrf_gpio_cfg_input(CONFIG_IO_IIC_SCL, NRF_GPIO_PIN_PULLUP);
//    nrf_gpio_cfg_input(CONFIG_IO_IIC_SDA, NRF_GPIO_PIN_PULLUP);
    
//    nrf_gpio_input_disconnect(CONFIG_IO_IIC_SCL);
//    nrf_gpio_input_disconnect(CONFIG_IO_IIC_SDA);

    m_i2c_initial = false;

    return NRF_SUCCESS;
}

//************************************************************************************************************
// ��������:   	����IIC�ײ㺯������������
// ��������:    dev_addr 	-> 	�豸���ߵ�ַ
//				pbuf		->	���������ݵĻ�����
//				len			->	���������ݵĳ���
//				no_stop		->	���������ݺ��Ƿ���ֹͣλ(true: ��ֹͣλ, false: ��ֹͣλ)
// ����ֵ  :	NRF_SUCCESS -> 	���ͳɹ�
//************************************************************************************************************
ret_code_t i2c_send(uint8_t dev_addr, uint8_t *pbuf, uint16_t len, bool no_stop)
{
    return nrf_drv_twi_tx(&m_twi_master, dev_addr, pbuf, len, no_stop);
}

//************************************************************************************************************
// ��������:   	����IIC�ײ㺯������������
// ��������:    dev_addr 	-> 	�豸���ߵ�ַ
//				pbuf		->	���������ݵĻ�����
//				len			->	���������ݵĳ���
// ����ֵ  :	NRF_SUCCESS -> 	���ճɹ�
//************************************************************************************************************
ret_code_t i2c_receieve(uint8_t dev_addr, uint8_t *pbuf, uint16_t len)
{
    
    return nrf_drv_twi_rx(&m_twi_master, dev_addr, pbuf, len);
}

