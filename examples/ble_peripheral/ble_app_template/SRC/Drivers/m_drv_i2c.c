#include "m_drv_i2c.h"

//TWI0
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(0);
static bool m_i2c_initial = false;

//************************************************************************************************************
// 功能描述:   	初始化IIC，可防止重复初始化
// 函数参数:    无
// 返回值  :	无
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
// 功能描述:   	注销IIC
// 函数参数:    无
// 返回值  :	NRF_SUCCESS -> 	成功注销
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
// 功能描述:   	调用IIC底层函数，发送数据
// 函数参数:    dev_addr 	-> 	设备总线地址
//				pbuf		->	待发送数据的缓存区
//				len			->	待发送数据的长度
//				no_stop		->	发送完数据后，是否无停止位(true: 无停止位, false: 有停止位)
// 返回值  :	NRF_SUCCESS -> 	发送成功
//************************************************************************************************************
ret_code_t i2c_send(uint8_t dev_addr, uint8_t *pbuf, uint16_t len, bool no_stop)
{
    return nrf_drv_twi_tx(&m_twi_master, dev_addr, pbuf, len, no_stop);
}

//************************************************************************************************************
// 功能描述:   	调用IIC底层函数，接收数据
// 函数参数:    dev_addr 	-> 	设备总线地址
//				pbuf		->	待接收数据的缓存区
//				len			->	待接收数据的长度
// 返回值  :	NRF_SUCCESS -> 	接收成功
//************************************************************************************************************
ret_code_t i2c_receieve(uint8_t dev_addr, uint8_t *pbuf, uint16_t len)
{
    
    return nrf_drv_twi_rx(&m_twi_master, dev_addr, pbuf, len);
}

