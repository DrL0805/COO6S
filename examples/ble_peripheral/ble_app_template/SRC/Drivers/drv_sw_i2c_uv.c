#include "sdk_common.h"
#include "sr3_config.h"
#include "nrf_delay.h"

#include "nrf_gpio.h"
#include "app_debug.h"

#include "drv_sw_i2c_uv.h"

#define UV_I2C_ADDRESS         (0x53) //0x52 0xd3 0xd2
#define PART_ID                (0x06)
#define MAIN_CTRL              (0x00)
#define ALS_UVS_MEAS_RATE      (0x04)
#define ALS_UVS_GAIN           (0x05)
#define INT_CFG                (0x19)
#define INT_PST                (0x1A)
#define ALS_DATA               (0x0D)
#define UVS_DATA               (0x10)

#define APP_TWI_WRITE_OP(address)      (((address) << 1) | 0)
#define APP_TWI_READ_OP(address)       (((address) << 1) | 1)


#define		SDA								CONFIG_IO_IIC_SDA_PIN
#define		SCL								CONFIG_IO_IIC_SCL_PIN
#define     INT                             0x13
/*
#define Sda_Config_Output()       do{ nrf_gpio_cfg_output(SDA); }while(0)  

#define Sda_Config_Input()        do{ nrf_gpio_cfg_input(SCL,NRF_GPIO_PIN_PULLUP)	; }while(0) 

#define Scl_Config_Output()       do{ nrf_gpio_cfg_output(SCL); }while(0) 

#define Sda_Output_High()         do{ nrf_gpio_pin_set(SDA);    }while(0) 

#define Sda_Output_Low()          do{ nrf_gpio_pin_clear(SDA);  }while(0) 

#define Scl_Output_High()         do{ nrf_gpio_pin_set(SCL);    }while(0) 

#define Scl_Output_Low()          do{ nrf_gpio_pin_clear(SCL);  }while(0) 

#define I2C_Delay()               do{ nrf_delay_us(2);          }while(0)

#define Sda_Pin                   nrf_gpio_pin_read(SDA)   
*/

#define		Sda_Config_Output()				nrf_gpio_cfg_output(SDA)
#define		Sda_Config_Input()				nrf_gpio_cfg_input(SDA,GPIO_PIN_CNF_PULL_Disabled)	

#define		Scl_Config_Output()				nrf_gpio_cfg_output(SCL)

#define		Sda_Output_High()				nrf_gpio_pin_set(SDA)
#define		Sda_Output_Low()				nrf_gpio_pin_clear(SDA)
#define		Scl_Output_High()				nrf_gpio_pin_set(SCL)
#define		Scl_Output_Low()				nrf_gpio_pin_clear(SCL)

#define		Sda_Pin							nrf_gpio_pin_read(SDA)

#define		I2C_Delay()						nrf_delay_us(4)

// *******************************************************************************
// *Funtion name:I2C_1_Init
// *Description : Init the IO port and init the bus
// *
// *Input: None
// *Output:None
// /******************************************************************************
void I2C_1_Init(void)
{

    Scl_Config_Output(); //am_hal_gpio_pin_config(SCL,AM_HAL_PIN_OUTPUT);
//    Sda_Config_Input();//am_hal_gpio_pin_config(SDA,AM_HAL_PIN_INPUT|AM_HAL_GPIO_PULL1_5K);
    Scl_Output_High(); //am_hal_gpio_out_bit_set(SCL);
    I2C_1_Start();
    I2C_1_WriteByte(0);
    I2C_1_Stop();
}


void I2C_1_UnInit(void)
{
   nrf_gpio_cfg_input(SDA, NRF_GPIO_PIN_NOPULL);
   nrf_gpio_cfg_input(SCL, NRF_GPIO_PIN_NOPULL);
}

// *******************************************************************************
// *Funtion name:I2C_1_Start
// *Description : IIC Start single
// *
// *Input: None
// *Output:None
// /******************************************************************************
void I2C_1_Start(void)
{
    Sda_Config_Output();
    Scl_Output_High();
    I2C_Delay();
    Sda_Output_High();
    I2C_Delay();
    Sda_Output_Low();
    I2C_Delay();
    Scl_Output_Low();
    I2C_Delay();
}

// *******************************************************************************
// *Funtion name: I2C_1_Stop
// *Description :IIC Stop single
// *
// *Input: None
// *Output:None
// /******************************************************************************
void I2C_1_Stop(void)
{
    Scl_Output_Low();
    Sda_Config_Output();
    Sda_Output_Low();
    I2C_Delay();
    Scl_Output_High();
    I2C_Delay();
    Sda_Output_High();
    I2C_Delay();
}

// *******************************************************************************
// *Funtion name: I2C_1_NoACK
// *Description :IIC NoACK single
// *
// *Input: None
// *Output:None
// /******************************************************************************
void I2C_1_NoACK(void)
{
    Sda_Output_High();
    I2C_Delay();
    Scl_Output_High();
    I2C_Delay();
    Scl_Output_Low();
}

// *******************************************************************************
// *Funtion name: I2C_1_ACK
// *Description :IIC NoACK single
// *
// *Input: None
// *Output:None
// /******************************************************************************
void I2C_1_ACK(void)
{
    Sda_Output_Low();
    I2C_Delay();
    Scl_Output_High();
    I2C_Delay();
    Scl_Output_Low();
}

// *******************************************************************************
// *Funtion name: I2C_1_ReadByte
// *Description :Read one byte from I2C
// *
// *Input: None
// *Output:Data (one Byte from EEPROM)
// /******************************************************************************
unsigned char I2C_1_ReadByte(void)
{
    unsigned char i;
    unsigned char Data = 0;
    Sda_Config_Input();
    I2C_Delay();
    for (i = 0; i < 8; i++)
    {
        Scl_Output_Low();
        I2C_Delay();
        Scl_Output_High();
        I2C_Delay();
        if (Sda_Pin == 1)
        {
            Data = (Data << 1) + 0x01;
        }
        else
        {
            Data = (Data << 1) + 0x00;
        }
    }
    Scl_Output_Low();
    Sda_Config_Output();
    I2C_Delay();
    return (Data);
}

// *******************************************************************************
// *Funtion name: I2C_1_WriteByte
// *Description :write one byte to EEPROM
// *
// *Input: None
// *Output:0x00:write success
//         0xFF:write fail
// /******************************************************************************
unsigned short I2C_1_WriteByte(unsigned char Data)
{
    unsigned char i;
    Sda_Config_Output();
    for (i = 0; i < 8; i++)
    {
        if ((Data & 0x80) == 0x80)
        {
            Sda_Output_High();
        }
        else
        {
            Sda_Output_Low();
        }
        nrf_delay_us(2);
        Scl_Output_High();
        nrf_delay_us(4);
        Scl_Output_Low();
        Data <<= 0x01;
        nrf_delay_us(2);
    }
    //      End of Write A Byte
    //      Wait "ACK" Signal
    Sda_Config_Input();
    nrf_delay_us(2);
    Scl_Output_High();
    I2C_Delay();
    while(1)
    {
        if(Sda_Pin == 1)
        {
            if ((--i) == 0)
            {
                //no ack indicate
                Sda_Config_Output();
                Sda_Output_High();
                I2C_Delay();
                return (0xFF);
            }
        }
        else
        {
            break;
        }
    }
    Scl_Output_Low();
    Sda_Config_Output();
    Sda_Output_High();
    I2C_Delay();
    return (0x00);                      // Write OK
}


unsigned short I2C_1_WriteBytes(unsigned char* dataAddr, unsigned short length)
{
	unsigned short i;
	unsigned short ret = 0;
	for(i = 0; i < length; i++)
	{
		ret = I2C_1_WriteByte(dataAddr[i]);
		if(ret == 0xff)
		{
			break;
		}
	}
	return ret;
}


void I2C_1_ReadBytes(unsigned char *dataAddr, unsigned short length)
{
	unsigned short i;
	for(i = 0; i < length; i++)
	{
		dataAddr[i] = I2C_1_ReadByte();
		if(i < (length - 1))
		{
			I2C_1_ACK();
		}
		else
		{
			I2C_1_NoACK();
		}

	}
}

uint32_t I2C_1_DeviceWrite(unsigned char deviceAddr, unsigned char regAddr, unsigned char* data_Point, unsigned short length )
{
    uint32_t ret = NRF_SUCCESS;
    I2C_1_Start();
    ret = I2C_1_WriteByte(APP_TWI_WRITE_OP(deviceAddr));
    if(ret != NRF_SUCCESS)
    {
        return ret;
    }   
    ret = I2C_1_WriteByte(regAddr);
    if(ret != NRF_SUCCESS)
    {
        return ret;
    }
    ret = I2C_1_WriteBytes(data_Point, length);
    if(ret != NRF_SUCCESS)
    {
        return ret;
    }
    I2C_1_Stop();
    return NRF_SUCCESS;
}




uint32_t I2C_1_DeviceRead(unsigned char deviceAddr, unsigned char regAddr, unsigned char* data_Point, unsigned short length)
{
    uint32_t ret = NRF_SUCCESS;
    I2C_1_Start();
    ret = I2C_1_WriteByte(APP_TWI_WRITE_OP(deviceAddr));
    if(ret != NRF_SUCCESS)
    {
        return ret;
    }  

    ret = I2C_1_WriteByte(regAddr);
    if(ret != NRF_SUCCESS)
    {
        return ret;
    }  
    I2C_1_Start();
    ret = I2C_1_WriteByte(APP_TWI_READ_OP(deviceAddr));
    if(ret != NRF_SUCCESS)
    {
        return ret;
    }  
    I2C_1_ReadBytes(data_Point, length);
    I2C_1_Stop();
    return NRF_SUCCESS;
}
