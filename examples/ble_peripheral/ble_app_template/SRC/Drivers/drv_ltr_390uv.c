#include "sdk_common.h"
#include "app_util_platform.h"

//#include "drv_iic1.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "app_debug.h"
#include "drv_ltr_390uv.h"
#include "resources.h"
#include "app_timer.h"
#include "drv_sw_i2c_uv.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "includes.h"


#if CONFIG_RTT_FLASH_OP_ENABLED
#define DRV_UV_Print(M, ...)    SEGGER_Print("DRV_UV", M, ##__VA_ARGS__)
#else
#define DRV_UV_Print(M, ...)
#endif


#if CONFIG_UV_ENABLED

#define UV_I2C_ADDRESS         (0x53)
#define PART_ID                (0x06)
#define MAIN_CTRL              (0x00)
#define ALS_UVS_MEAS_RATE      (0x04)
#define ALS_UVS_GAIN           (0x05)
#define INT_CFG                (0x19)
#define INT_PST                (0x1A)
#define ALS_DATA               (0x0D)
#define UVS_DATA               (0x10)


APP_TIMER_DEF(uv_test_timer_id);

#define TWI_ENANBLE 0   // 使能硬件TWI模块, 如果未使能TWI，则使用软件iic （硬件iic和spi寄存器地址相同，使用了spi0，则不能使用twi0，同理spi1 跟 twi1）


ret_code_t ltr_390uv_write_cmd(uint8_t write_cmd, uint8_t data)
{
   
    #if TWI_ENANBLE
    uint8_t send_len=1;
    uint8_t send_buf[4] = {0};
    send_buf[0] = write_cmd;
    send_buf[1] = data;
    return drv_iic1_write_buf(UV_I2C_ADDRESS, send_buf, send_len, true);       
    #else
    return I2C_1_DeviceWrite(UV_I2C_ADDRESS,write_cmd, &data,1);
    #endif    
}

ret_code_t ltr_390uv_read_buf(uint8_t read_cmd, uint8_t *buf, uint8_t len)
{
    ASSERT(buf);

   
    #if TWI_ENANBLE
    uint32_t err_code = NRF_SUCCESS;
    err_code = drv_iic1_write_buf(UV_I2C_ADDRESS, &read_cmd, 1, false);
    if (NRF_SUCCESS == err_code)
    {
        err_code = drv_iic1_read_buf(UV_I2C_ADDRESS, buf, len, true);
    }
    return err_code;    
    #else
    return I2C_1_DeviceRead(UV_I2C_ADDRESS,read_cmd, buf, len);
    #endif    
}

static ret_code_t uv_verify_id(void)
{
    
    ret_code_t status;
    uint8_t reg_val=0;;

    status = ltr_390uv_read_buf(PART_ID, &reg_val, 1);
    if (status != NRF_SUCCESS)
    {
        return status;
    }
    DRV_UV_Print("PART_ID %02x ",reg_val);
    if(reg_val == 0xB2)
    {
        return NRF_SUCCESS;
    }
    return 1;    
}
/*
18x 20bit  : uv source irradiance(uw/cm2) =  uv count / 100 
uv index = mw/m2 * 0.04
1uw / cm^2 =10^4 uw / m^2 = 10mw /m2
*/

ret_code_t ltr_390uv_enable(void)
{
//    nrf_gpio_cfg_output(19);
//    nrf_gpio_pin_clear(19);
//    nrf_delay_ms(1);
    
    uint32_t err_code = NRF_SUCCESS;
    err_code = ltr_390uv_write_cmd(MAIN_CTRL,0x0A);  //uv   active mode 
    if(err_code!=NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ltr_390uv_write_cmd(ALS_UVS_MEAS_RATE,0x04); //20bitas meas rate = 500ms 
    if(err_code!=NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ltr_390uv_write_cmd(ALS_UVS_GAIN,0x04);  //gain range 18
    
    return err_code;
}

ret_code_t ltr_390uv_disable(void)
{
    uint32_t err_code = NRF_SUCCESS;
    err_code = ltr_390uv_write_cmd(MAIN_CTRL,0x00);  //default   standby mode
//    nrf_gpio_cfg_input(19,GPIO_PIN_CNF_PULL_Pullup);
    return err_code;    
}


ret_code_t ltr_390uv_read_uv_data(uint8_t *uv_data_buf)
{
    uint32_t err_code = NRF_SUCCESS;
    err_code = ltr_390uv_read_buf(UVS_DATA,  &uv_data_buf[0],1);
    if(err_code!=NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ltr_390uv_read_buf(UVS_DATA+1,&uv_data_buf[1],1);
    if(err_code!=NRF_SUCCESS)
    {
        return err_code;
    }
    err_code = ltr_390uv_read_buf(UVS_DATA+2,&uv_data_buf[2],1);
    return err_code;
}

static void uv_test_sched_event_handler(void * p_context)
{
    uv_test();
}

static void uv_test_timeout_timer_handler(void * p_context)
{
    APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, uv_test_sched_event_handler, NULL));
}




ret_code_t drv_ltr_390uv_init(void)
{
    #if TWI_ENANBLE
    drv_iic1_init();
    nrf_delay_ms(4);
    #else
    I2C_1_Init();
    #endif
//    ltr_390uv_enable();
//    app_timer_create(&uv_test_timer_id,APP_TIMER_MODE_REPEATED,uv_test_sched_event_handler);
//    app_timer_start(uv_test_timer_id,APP_TIMER_TICKS(400,0),NULL);
    return NRF_SUCCESS;
}

ret_code_t drv_ltr_390uv_uninit(void)
{
    #if TWI_ENANBLE
    drv_iic1_uninit();
    #else
    I2C_1_UnInit();
    #endif
    return NRF_SUCCESS;
}



ret_code_t uv_test(void)
{    
    ret_code_t status;
    uint8_t uv_data_buf[3]={0};
    ltr_390uv_enable();
    status = uv_verify_id();
    if(status != NRF_SUCCESS)
    {
        return status;
    }
    status = ltr_390uv_read_uv_data(uv_data_buf);
    uint32_t uvs_data = (uv_data_buf[2]*65536) + (uv_data_buf[1]*256) + uv_data_buf[0];
    ltr_390uv_disable();
    DRV_UV_Print("status %d uvs_data %d ",status,uvs_data);
    return status;
}
#endif


