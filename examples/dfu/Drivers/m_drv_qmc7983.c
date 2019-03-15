#include "m_drv_qmc7983.h"

#include "includes.h"

#if CONFIG_RTT_QMC7983_ENABLED
#define QMC_Print(M, ...)	SEGGER_Print("QMC7983", M, ##__VA_ARGS__)
#else
#define QMC_Print(M, ...)
#endif


// addr define
#define QMC7983_ADDRESS 	(0x2C)//(0x2C << 1) //ADO=L

// register macro
#define QMC7983_CHIP_ID_REG 			0x0D
#define QMC7983_OUT_X_L_REG 			0x00
#define QMC7983_OUT_X_M_REG 			0x01
#define QMC7983_OUT_Y_L_REG 			0x02
#define QMC7983_OUT_Y_M_REG 			0x03
#define QMC7983_OUT_Z_L_REG 			0x04
#define QMC7983_OUT_Z_M_REG 			0x05
#define QMC7983_STA_REG 				0x06    /*Status registers */
#define QMC7983_TEMP_L_REG 				0x07 /* Temperature registers */
#define QMC7983_TEMP_H_REG 				0x08
#define QMC7983_CTL_ONE_REG 			0x09          /* Contrl register one */
#define QMC7983_CTL_TWO_REG 			0x0A          /* Contrl register two */
#define QMC7983_SET_RESET_PERIOD_REG 	0x0B /* SET/RESET Period Register */

// magnetism work mode
typedef enum
{
	MAG_STANDBY_MODE 	= 0x00,
	MAG_CONTINUOUS_MODE = 0x01,
	MAG_SELFTEST_MODE 	= 0x02,
} bsp_mag_mode;

//Full Scale range
typedef enum
{
	MAG_SCALE_RANGE_2GS  = 0x00,
	MAG_SCALE_RANGE_8GS  = 0x01,
	MAG_SCALE_RANGE_12GS = 0x02,
	MAG_SCALE_RANGE_20GS = 0x03,
} bsp_mag_scalerange;

//Output Data Rate
typedef enum
{
	MAG_DATA_RATE_10HZ 	= 0x00,
	MAG_DATA_RATE_50HZ 	= 0x01,
	MAG_DATA_RATE_100HZ = 0x02,
	MAG_DATA_RATE_200HZ = 0x03,
} bsp_mag_datarate;

//Over  sample Rate
typedef enum
{
	MAG_OVER_SAMPLE_RATE_512 = 0x00,
	MAG_OVER_SAMPLE_RATE_256 = 0x01,
	MAG_OVER_SAMPLE_RATE_128 = 0x02,
	MAG_OVER_SAMPLE_RATE_64  = 0x03,
} bsp_mag_osr;

#if CONFIG_MAGNET_ENABLED

APP_TIMER_DEF(m_mag_timer_id);

//**********************************************************************
// 函数功能:    寄存器写
//**********************************************************************
static void mag_reg_write(uint8_t reg_addr, uint8_t *regval, uint16_t length)
{
	uint8_t		buf[length + 1];
	
	buf[0] = reg_addr;
	memcpy(&buf[1], regval, length);
	
	uint32_t 	ret_code;
	
    ret_code = i2c_send(QMC7983_ADDRESS, buf, length + 1, false);
	if (NRF_SUCCESS != ret_code)
	{
		QMC_Print("111: 0x%x", ret_code);
	}
}

//**********************************************************************
// 函数功能:    寄存器读
//**********************************************************************
static void mag_reg_read(uint8_t reg_addr, uint8_t *regval, uint16_t length)
{
	uint32_t	ret_code;
	ret_code = i2c_send(QMC7983_ADDRESS, &reg_addr, 1, true);
	if (NRF_SUCCESS != ret_code)
	{
		QMC_Print("111: 0x%x", ret_code);
	}
	
	ret_code = i2c_receieve(QMC7983_ADDRESS, regval, length);
	if (NRF_SUCCESS != ret_code)
	{
		QMC_Print("222: 0x%x", ret_code);
	}
}

//**********************************************************************
// 函数功能:    地磁传感器初始化
//**********************************************************************
uint8_t drv_mag_init(void)
{
    uint8_t regTemp = 0;
	regTemp = i2c_init();
	QMC_Print("i2c_init: 0x%x\n\r", regTemp);
	
	/* read mag sensor ID */
	mag_reg_read(QMC7983_CHIP_ID_REG, &regTemp, 1);
	QMC_Print("QMC7983 ID: 0x%.2x\n\r", regTemp);

    /* Set Sample Rate */
    mag_reg_read(QMC7983_CTL_ONE_REG, &regTemp, 1);
    regTemp &= 0xF3;
    regTemp |= (MAG_DATA_RATE_50HZ << 2);
    mag_reg_write(QMC7983_CTL_ONE_REG, &regTemp, 1);

    /* Set Scale Range */
    mag_reg_read(QMC7983_CTL_ONE_REG, &regTemp, 1);
    regTemp &= 0xCF;
    regTemp |= (MAG_SCALE_RANGE_8GS << 4);
    mag_reg_write(QMC7983_CTL_ONE_REG, &regTemp, 1);

    /* set OVER_SAMPLE_RATE */
    mag_reg_read(QMC7983_CTL_ONE_REG, &regTemp, 1);
    regTemp &= 0x3C;
    regTemp |= (MAG_CONTINUOUS_MODE << 0) | (MAG_OVER_SAMPLE_RATE_64 << 6);
    mag_reg_write(QMC7983_CTL_ONE_REG, &regTemp, 1);

    /* SET_RESET period  */
    regTemp = 0x10;
    mag_reg_write(QMC7983_SET_RESET_PERIOD_REG, &regTemp, 1);

    return 0;
}

//**********************************************************************
// 函数功能:  读取地磁传感器3轴数据，，数据为二进补码形式
//**********************************************************************
uint8_t drv_mag_read(int16_t *magData)
{
    uint8_t axisdata[6];

    mag_reg_read(QMC7983_OUT_X_L_REG, axisdata, 6);
    magData[0] = ((uint16_t)axisdata[1] << 8) + axisdata[0];
    magData[1] = ((uint16_t)axisdata[3] << 8) + axisdata[2];
    magData[2] = ((uint16_t)axisdata[5] << 8) + axisdata[4];

    return 0;
}

//**********************************************************************
// 函数功能:    地磁传感器唤醒
//**********************************************************************
uint8_t drv_mag_wake(void)
{
    uint8_t regTemp = 0;

    //Set CONTROL Into Continuous Mode
    mag_reg_read(QMC7983_CTL_ONE_REG, &regTemp, 1);
    regTemp &= 0xFC;
    regTemp |= (MAG_CONTINUOUS_MODE << 0);
    mag_reg_write(QMC7983_CTL_ONE_REG, &regTemp, 1);

    return 0 ;
}

//**********************************************************************
// 函数功能:    设置地磁传感器进入睡眠状态
//**********************************************************************
uint8_t drv_mag_sleep(void)
{
    uint8_t regTemp = 0;

    //Set CONTROL Into Standby Mode
    mag_reg_read(QMC7983_CTL_ONE_REG, &regTemp, 1);
    regTemp &= 0xFC;
    regTemp |= (MAG_STANDBY_MODE << 0);
    mag_reg_write(QMC7983_CTL_ONE_REG, &regTemp, 1);

    return 1;
}


void m_mag_timeout_handler(void *p_text)
{
	int16_t		mag_data[3];
	
	drv_mag_read(mag_data);
	

	float angle= atan2((double)mag_data[1],(double)mag_data[0]) * (180 / 3.14159265) + 180; // angle in degrees
	float angle1=atan2((double)mag_data[2],mag_data[1]) * (180 / 3.14159265) + 180; // angle in degrees
	
	char str[20];
	sprintf(str, "%.2f-%.2f", angle, angle1);
	
	QMC_Print("%s, X: %d, Y: %d, Z: %d", str, mag_data[0], mag_data[1], mag_data[2]);
}

uint32_t	m_mag_init(void)
{
	drv_mag_init();
	
	
	APP_ERROR_CHECK(app_timer_create(&m_mag_timer_id, APP_TIMER_MODE_REPEATED, m_mag_timeout_handler));
	APP_ERROR_CHECK(app_timer_start(m_mag_timer_id, APP_TIMER_TICKS(500), NULL));
	return NRF_SUCCESS;
}
#endif

