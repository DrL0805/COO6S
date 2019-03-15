#ifndef __BSP_CFG_H__
#define __BSP_CFG_H__

#define H004_BOARD  1
#define H005_BOARD  2

#define BOARD   H004_BOARD  //PCB板子选择

#define BLE_LOWSPP_ENABLED      1
#define BLE_ANCS_ENABLED        1

#if (BOARD == H004_BOARD)

#define SHT3X_ENABLED   0


#define VERSION_NEW     1
#define VERSION_OLD     2
#define VERSION         VERSION_OLD  

#if (VERSION == VERSION_NEW)

//Led显示
#define LED_1_Pin   (18)
#define LED_2_Pin   (16)
#define LED_3_Pin   (15)
#define LED_4_Pin   (25)
#define LED_5_Pin   (26)
#define LED_6_Pin   (27)

//按键
#define BUTTON_1_Pin    (17)
#define BUTTON_2_Pin    (14)

//蜂鸣器
#define BUZZ_Pin        (6)

//ADC检测使能
#define ADC_EN_Pin      (12)    //没用到，悬空

//5V检测脚
#define DETECT_5V_Pin   (20)

//充电检测
#define BAT_CHARGE_Pin  (19)

//电池电量检测
#define BAT_ADC_Pin     (4)

//SPI Bus
#define SPI_SDI_Pin     (28)
#define SPI_SDO_Pin     (29)  
#define SPI_SCLK_Pin    (5)

//W25Q80片选
#define SPI_CS_FLASH_Pin    (11)

//3轴加速度计片选
#define SPI_CS_3G_Pin       (8)

//I2C Bus
#define I2C_SCL_Pin     (2)
#define I2C_SDA_Pin     (3)

//PDM数字麦克风
#define MP_SCLK_Pin     (22)
#define MP_DATA_Pin     (23)

//心电传感器
#define ECC_TX_Pin      (31)
#define ECC_RX_Pin      (30)
#define ECC_EN_Pin      (7)
#define ECC_RST_Pin     (24)

#elif (VERSION == VERSION_OLD)

//Led显示
#define LED_1_Pin   (22)
#define LED_2_Pin   (23)
#define LED_3_Pin   (24)
#define LED_4_Pin   (25)
#define LED_5_Pin   (26)
#define LED_6_Pin   (27)

//按键
#define BUTTON_1_Pin    (13)
#define BUTTON_2_Pin    (14)

//蜂鸣器
#define BUZZ_Pin        (6)

//ADC检测使能
#define ADC_EN_Pin      (12)    //没用到，悬空

//5V检测脚
#define DETECT_5V_Pin   (20)

//充电检测
#define BAT_CHARGE_Pin  (19)

//电池电量检测
#define BAT_ADC_Pin     (4)

//SPI Bus
#define SPI_SDI_Pin     (28)
#define SPI_SDO_Pin     (29)  
#define SPI_SCLK_Pin    (5)

//W25Q80片选
#define SPI_CS_FLASH_Pin    (11)

//3轴加速度计片选
#define SPI_CS_3G_Pin       (8)

//I2C Bus
#define I2C_SCL_Pin     (2)
#define I2C_SDA_Pin     (3)

//PDM数字麦克风
//#define MP_SCLK_Pin     (22)
//#define MP_DATA_Pin     (23)

//心电传感器
#define ECC_TX_Pin      (31)
#define ECC_RX_Pin      (30)
#define ECC_EN_Pin      (7)
#define ECC_RST_Pin     (24)

#endif
 

#elif   (BOARD == H005_BOARD)

#define SHT3X_ENABLED   1

//没有电量检测

//UART Debug IO
#define DEBUG_RX        9
#define DEBUG_TX        10

//3轴加速度计
#define G_TRIG_Pin      12 
#define G_SDI_Pin       13
#define G_SDO_Pin       14
#define G_SCLK_Pin      15
#define G_CS_Pin        19

//按键
#define BUTTON_Pin      20

//温湿度
#define SHT_SCL_Pin     30    
#define SHT_SDA_Pin     28
#define SHT_RST_Pin     31
#define SHT_TRIG_Pin    29   

//Led显示
#define LED_1_Pin       25
#define LED_2_Pin       26
#define LED_3_Pin       27

#endif



#endif

