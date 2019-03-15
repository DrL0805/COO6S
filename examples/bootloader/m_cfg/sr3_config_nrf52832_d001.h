/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef _C003S_CONFIG_PG000_H_
#define _C003S_CONFIG_PG000_H_

// <<< Use Configuration Wizard in Context Menu >>>

// <h> C001S Base Board Features



// <q> Enable Buttonless Enter Dfu
// <i>  DFU service
#define CONFIG_DFU_SERVICE_ENABLED 1

// <q> Enable G-Sensor
// <i> 使能加速度模块
#define CONFIG_GSENSOR_ENABLED 1

// <q> Enable UV
// <i> 使能UV模块
#define CONFIG_UV_ENABLED 1

// <q> Enable WATCH_HAND
// <i> 使能表针模块
#define CONFIG_WATCH_HAND_ENABLED 1

// <o> TWI Bus
// <i> Select the TWI Bus that will be used to communicate with the accelerometer.
//  <0=>Primary
//  <1=>Secondary
#define CONFIG_ACC_TWI_BUS 0


// <o> linkloss reminder timeout[s] <1-65535>
// <i> 连接断开后，几秒内没有重连，发送断开提醒
#define CONFIG_LINKLOSS_REMINDER_TIMEOUT 10


// <o> Mode timeout[s] <1-65535>
// <i> 进入模式超时时间，如拍照模式，校针模式等
#define CONFIG_MODE_TIMEOUT 30


// <q> Enable Vibrator
// <i> 使能震动模块
#define CONFIG_VIBRATOR_ENABLED 0



// <e> Enable Buttons
// <i> 使能按键模块
#define CONFIG_BUTTON_ENABLED 1

// </e>

// <e> Enable Battery Measurement
// <i> Enable the battery measurement infrastructure
#define CONFIG_BATT_MEAS_ENABLED 0

// <o> Voltage of 5% Battery Level [mV]
// <i> 低于该值，为低电状态
#define CONFIG_BATT_MEAS_LOWPOWER_LEVEL 2700

// <o> Voltage of 95% Battery Level [mV]
// <i> 高于该值，为满电状态
#define CONFIG_BATT_MEAS_FULLPOWER_LEVEL 2900

// <o> Battery Voltage Polling Rate [s] <>
// <i> 电池检测时间间隔
#define CONFIG_BATT_MEAS_POLL_RATE 60

// </e>

// <q> Enable DFU Support
// <i> Enable Device Firmware Update subsystem that allows for firmware update over Bluetooh.
#define CONFIG_DFU_ENABLED 0

// <q> Enable LED Support
// <i> Enable the LED signaling subsystem.
#define CONFIG_LED_ENABLED 1

// <q> Enable WATCH HAND ROLLBACK Support
// <i> Enable the WATCH HAND ROLLBACK subsystem.
#define CONFIG_WATCH_HAND_ROLLBACK_ENABLED 0


// <q> Enable FLASH_TEST Support
// <i> Enable the sleep and step data test .
#define CONFIG_TEST_ENABLED 0
// <e> Enable Power Management
// <i> Enable the power management subsystem.
#define CONFIG_PWR_MGMT_ENABLED 1

// <o> System Off Time-out [s] <1-3600>
// <i> Set the time after which the remote is automatically put into deep sleep mode if no user interaction is detected.
#define CONFIG_PWR_MGMT_SYSTEM_OFF_TIMEOUT 1
// </e>

// <e> Enable Watchdog
// <i> This option enables the watchdog timer.
#define CONFIG_WATCHDOG_ENABLED 1

// <o> Watchdog Time-out [s] <1-300>
// <i> Set the watchdog time-out.
#define CONFIG_WATCHDOG_TIMEOUT 5
// </e>
// </h>

// <h> Bluetooth Options
// <i> This section contains the configuration settings for Bluetooth services.

// <q> Enable Immediate Alert Service
// <i> Enable the Immediate Alert Service.
#define CONFIG_IAS_ENABLED 0


// <q> Enable Battery Measurement Service
// <i> Enable the battery measurement service.
#define CONFIG_BATT_MEAS_SERVICE_ENABLED 0

// <q> Enable Ancs Clent
// <i> ANCS 客户端使能
#define CONFIG_ANCS_C_ENABLED 1


// <h> Device Information
// <i> Configure the settings of the Bluetooth Device Information service.

// <q> Enable Device Infomation service
// <i> Enable the Device Infomation service.
#define CONFIG_DIS_ENABLED 0

// <s.31> Device Name
// <i> UTF-8 string representing the name of the device.
#define CONFIG_DEVICE_NAME "C003S--T"

// <s.31> Project Num
// <i> UTF-8 string representing the name of the device.
#define CONFIG_PROJECT_NUM "PG000"

// <s> Product name
// <i> 产品名称
#define CONFIG_PRODUCT_NAME CONFIG_DEVICE_NAME

// <o> Product fw version major
// <i> 产品固件版本号: 主版本号，十进制
#define CONFIG_PRODUCT_FW_VER_MAJOR 0

// <o> Product fw version minor
// <i> 产品固件版本号: 次版本号，十进制
#define CONFIG_PRODUCT_FW_VER_MINOR 1

// <o> Product fw version test
// <i> 产品固件版本号: 测试版本号，十进制
#define CONFIG_PRODUCT_FW_VER_TEST 0



// <s> Manufacturer Name
// <i> UTF-8 string representing the name of the manufacturer of the device.
#define CONFIG_MANUFACTURER_NAME "SmartMovt"

// <s> Firmware Revision
// <i> UTF-8 string representing the firmware revision for the firmware within the device.
#define CONFIG_FIRMWARE_REVISION "0.1.0"

// <s> Hardware Revision
// <i> UTF-8 string representing the hardware revision for the hardware within the device.
#define CONFIG_HARDWARE_REVISION "C003S"

// <s> Serial Number
// <i> UTF-8 string representing the serial number of the particular instance of the device.
#define CONFIG_SERIAL_NUMBER "0"

// <o> PNP Product ID <0x0000-0xFFFF>
// <i> Manufacturer-managed identifier of this product.
#define CONFIG_PNP_PRODUCT_ID 0x0000

// <o> PNP Product Version <0x0000-0xFFFF>
// <i> Manufacturer-managed version of this product.
#define CONFIG_PNP_PRODUCT_VERSION 0x0000

// <o> PNP Vendor ID Source <0x00-0xFF>
// <i> Identifies the source of the Vendor ID field.
#define CONFIG_PNP_VENDOR_ID_SOURCE 0x00

// <o> PNP Vendor ID <0x0000-0xFFFF>
// <i> Identifies the product vendor from the namespace in the Vendor ID Source.
#define CONFIG_PNP_VENDOR_ID 0x0000
// </h>

// <h> Advertising Parameters
// <i> Configure the Bluetooth advertising parameters.

// <q> Advertising After Disconnected
#define CONFIG_ADV_ARTER_DISCONNECTED 1


// <q> Use Directed Advertising
// <i> Enable Connectable Directed Advertising.
#define CONFIG_ADV_DIRECTED 0

// <o> Advertising Time-out [s]
// <i> Set the time after which the remote is automatically put into deep sleep mode if it cannot connect to the host.
#define CONFIG_ADV_TIMEOUT 300

// <o> Fast Advertising Interval [ms] <20-10000>
// <i> Configure the interval between advertising packets in the Fast Advertising mode.
#define CONFIG_ADV_INTERVAL_FAST_MS 30
#define CONFIG_ADV_INTERVAL_FAST ROUNDED_DIV(1000u * CONFIG_ADV_INTERVAL_FAST_MS, 625)

// <o> Slow Advertising Interval [ms] <20-10000>
// <i> Configure the interval between advertising packets in the Slow Advertising mode.
#define CONFIG_ADV_INTERVAL_SLOW_MS 100
#define CONFIG_ADV_INTERVAL_SLOW ROUNDED_DIV(1000u * CONFIG_ADV_INTERVAL_SLOW_MS, 625)
// </h>

// <h> Connection Parameters
// <i> Configure the Bluetooth connection parameters.

// <o> Minimum Connection Interval [ms] <7-4000>
// <i> Set the Minimum Connection Interval that is sent in the Connection Parameter Update request.
#define CONFIG_MIN_CONN_INTERVAL_MS 20
#define CONFIG_MIN_CONN_INTERVAL ROUNDED_DIV(100u * CONFIG_MIN_CONN_INTERVAL_MS, 125)

// <o> Maximum Connection Interval [ms] <7-4000>
// <i> Set the Maximum Connection Interval that is sent in the Connection Parameter Update request.
#define CONFIG_MAX_CONN_INTERVAL_MS 100
#define CONFIG_MAX_CONN_INTERVAL ROUNDED_DIV(100u * CONFIG_MAX_CONN_INTERVAL_MS, 125)

// <o> Slave Latency [number of connection events] <0x0000-0x01F3>
// <i> Set the Slave Latency that is sent in the Connection Parameter Update request.
#define CONFIG_SLAVE_LATENCY 1

// <o> Connection Supervisory Time-out [ms] <100-32000>
// <i> Set the Connection Supervisory Time-out that is sent in the Connection Parameter Update request.
#define CONFIG_CONN_SUP_TIMEOUT_MS 5000
#define CONFIG_CONN_SUP_TIMEOUT ROUNDED_DIV(CONFIG_CONN_SUP_TIMEOUT_MS, 10)

// <o> First Connection Parameter Update Delay [s] <1-300>
// <i> Set the time between establishing the connection and the first Connection Parameter Update request.
#define CONFIG_FIRST_CONN_PARAMS_UPDATE_DELAY 30

// <o> Next Connection Parameter Update Delay [s] <1-300>
// <i> Set the time between subsequent Connection Parameter Update requests.
#define CONFIG_NEXT_CONN_PARAMS_UPDATE_DELAY 30

// <o> Maximum Attempts of Connection Parameter Negotiation <1-16>.
// <i> Set the number of attempts before giving up the Connection Parameter negotiation.
#define CONFIG_MAX_CONN_PARAMS_UPDATE_COUNT 3
// </h>

// <h> Bonding Options
// <i> Configure the Bluetooth bonding settings.

// <o> Maximum Bonds <1-254>
// <i> Set the maximum number of bonds stored in flash memory.
#define CONFIG_MAX_BONDS 2

// <q> Change the device address for each new bond
// <i> When enabled, the Bluetooth address is changed for each new bond.
// <i> This prevents previously connected masters from attempting to reconnect.
#define CONFIG_CHANGE_ADDRESS 0

// <q> Attempt to reconnect to all bonded masters instead of only to the last one
#define CONFIG_RECONNECT_ALL 0

// <q> When pairing is requested, attempt to reconnect to other bonded masters before advertising as bondable
#define CONFIG_BOND_RECONNECT_ALL 1
// </h>

// <h> Security Options
// <i> Configure the Bluetooth link security settings.

// <q> Allow Legacy Pairing
// <i> Allow for Legacy pairing. Refer to Bluetooth specification document for details.
#define CONFIG_SEC_LEGACY_PAIRING 1

// <q> Allow LESC Pairing
// <i> Allow for Low Energy Secure Connections pairing. Refer to Bluetooth specification document for details.
#define CONFIG_SEC_LESC_PAIRING 0

// <q> Allow Repairing
// <i> Choose whether to allow a peer to pair if it wants to, when it is already bonded. In a production environment, this option should be disabled for increased security.
#define CONFIG_SEC_ALLOW_REPAIRING 1

// <q> Enable Bonding
// <i> Choose whether to allow the device to bond.
#define CONFIG_SEC_BOND 1

// <o> Minimum Encryption Key Size [octets] <7-16>
#define CONFIG_SEC_MIN_KEY_SIZE 7

// <o> Maximum Encryption Key Size [octets] <7-16>
#define CONFIG_SEC_MAX_KEY_SIZE 16

// <o> Encryption Request Delay [ms]
// <i> Set the time between establishing the connection and the Bluetooth authentication request.
#define CONFIG_ENC_REQ_DELAY 500
// </h>


// <o> Radio TX Power
//  <0xD8=>-40 dBm  <0xE2=>-30 dBm  <0xEC=>-20 dBm  <0xF0=>-16 dBm
//  <0xF4=>-12 dBm  <0xF8=>-8 dBm   <0xFC=>-4 dBm   <0x00=>0 dBm
//  <0x04=>4 dBm
// <i> Set the power of the radio transmitter.
#define CONFIG_RADIO_TX_POWER 0
// </h>

// <h> I/O Configuration
// <i> This section contains configuration for the I/O pin assignment.

// <h> Flash spi
// <i> 配置 flash 所用 spi 的接口引脚

// <o> spi sck
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_FLASH_SPI_SCK_PIN 5

// <o> spi cs
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_FLASH_SPI_CS_PIN 11

// <o> spi mosi
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_FLASH_SPI_MOSI_PIN 29

// <o> spi miso
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_FLASH_SPI_MISO_PIN 28
// </h>

// <h> Gsensor spi
// <i> 配置 g-sensor 所用 spi 的接口引脚

// <o> spi sck
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_GSENSOR_SPI_SCK_PIN 4

// <o> spi cs
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_GSENSOR_SPI_CS_PIN 8

// <o> spi mosi
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_GSENSOR_SPI_MOSI_PIN 31

// <o> spi miso
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_GSENSOR_SPI_MISO_PIN 30
// </h>


// <h> uv spi
// <i> 配置 uv 所用 i2c 的接口引脚

// <o> i2c sck
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_IIC_SCL_PIN  20

// <o> I2C SDA
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_IIC_SDA_PIN  18

// <o> UV INT
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_UV_INT_PIN   19


// <q> Keep the interface powered up during system-off state.
#define CONFIG_TWI0_UP_IN_SYSOFF_STATE 1
// </h>
// <q> Keep the interface powered up during system-off state.
#define CONFIG_TWI1_UP_IN_SYSOFF_STATE 1
// </h>

// <o> TWI Transaction Queue Size (Secondary Interface) <1-16>
#define CONFIG_APP_TWI1_QSIZE 1




// <h> Adc
// <i> 配置 ADC 采样电阻供电开关

// <o> Adc enabled pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_ADC_ENABLE_PIN 0x00000010

// <o> Adc enabled pin Active High
// <i> ADC 采样电阻供电控制管脚是否是高电平有效
#define CONFIG_IO_ADC_ENABLE_ACTIVE_HIGH  1
// </h>


// <h> Vibrator
// <i> 配置马达输出控制管脚

// <o> Vibrator pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_VIBRATOR 0x00000006

// <o> Vibrator pin Active High
// <i> 马达控制管脚是否是高电平产生震动
#define CONFIG_IO_VIBRATOR_ACT_HIGH 1
// </h>


// <h> Watch Hand
// <i> 配置表针输出管脚

// <o> Watch hand 1 left pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_WATCH_HAND_1_LEFT 0x00000019

// <o> Watch hand 1 right pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_WATCH_HAND_2_RIGHT 0x0000000C
// </h>


// <h> Board Power Control
// <i> Configure the Board Power Control settings.
// <o> Board Power Control Switch Pin
// <i> The Board Power Control Switch cuts off power to peripherals during sleep.
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_BOARD_PWR_CTRL 0xFFFFFFFF

// <q> Board Power Control Switch Active Low
// <i> Select this option if the Board Power Control Switch is active-low.
#define CONFIG_BOARD_PWR_CTRL_ACT_LOW 1
// </h>

// <h> BUTTONs
// <i> Configure the BUTTON outputs.

// <o> BUTTON 0 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_BUTTON_0 0x00000011


// <o> BUTTON 1 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_BUTTON_1 0xFFFFFFFF

// <o> BUTTON 2 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_BUTTON_2 0xFFFFFFFF

// <o> BUTTON 3 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_BUTTON_3 0xFFFFFFFF

// <o> BUTTON WAKEUP Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_BUTTON_WAKEUP 0x000000011


// <o.0> BUTTON 0 Active Low
// <i> Select this option if BUTTON 0 output is active-low.
// <o.1> BUTTON 1 Active Low
// <i> Select this option if BUTTON 1 output is active-low.
// <o.2> BUTTON 2 Active Low
// <i> Select this option if BUTTON 2 output is active-low.
// <o.3> BUTTON 3 Active Low
// <i> Select this option if BUTTON 3 output is active-low.
#define CONFIG_BUTTON_ACT_LOW_MASK 1
// </h> 


// <h> LEDs
// <i> Configure the LED outputs.

// <o> LED 0 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_LED_0 0x0000001A

// <o> LED 1 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_LED_1 0x0000001B

// <o> LED 2 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_LED_2 0xFFFFFFFF

// <o> LED 3 Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_LED_3 0xFFFFFFFF

// <o.0> LED 0 Active Low
// <i> Select this option if LED 0 output is active-low.
// <o.1> LED 1 Active Low
// <i> Select this option if LED 1 output is active-low.
// <o.2> LED 2 Active Low
// <i> Select this option if LED 2 output is active-low.
// <o.3> LED 3 Active Low
// <i> Select this option if LED 3 output is active-low.
#define CONFIG_LED_ACT_LOW_MASK 3
// </h>

// <h> Touchpad DC/DC Converter Control
// <o> Discharge Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_TP_DCDC_DISCHARGE 0xFFFFFFFF

// <q> Discharge Signal Active-Low
#define CONFIG_TP_DCDC_DISCHARGE_ACT_LOW 0

// <o> Shutdown Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_TP_DCDC_SHUTDOWN 0xFFFFFFFF

// <q> Shutdown Signal Active-Low
#define CONFIG_TP_DCDC_SHUTDOWN_ACT_LOW 0
// </h>


// <h> TWI Transaction Scheduling Options
// <i> Configure the advanced TWI settings.
// <o> TWI Transaction Queue Size (Primary Interface) <1-16>
#define CONFIG_APP_TWI0_QSIZE 4
// </h>


// <h> Advanced Options
// <i> This section contains advanced configuration settings of the Smart Remote 3 nRF52 firmware.

// <h> Event Bus Configuration
// <i> Configure the Event Bus.

// <o> Event Pool size <4-254>
// <i> Specify how many events can be simultaneously queued in the Event Bus.
#define CONFIG_EVENT_POOL_SIZE 8

// <q> Force Event Bus Error Checking
// <i> By default, errors that appear during non-essential event handling are ignored.
// <i> When this option is enabled, all errors are fatal.
#define CONFIG_EVENT_FORCE_ERROR_CHECKING 1
// </h>

// <h> Stack Configuration
// <i> Configure the stack settings.
// <e> Enable Stack Guard Page
// <i> Enable the stack guard page to help detect stack-related errors. The stack guard page is the inaccessible part of memory located at the end of the stack.
#define CONFIG_STACK_GUARD_ENABLED 1

// <o> Stack Guard Page Size
//  <32=>32 bytes
//  <64=>64 bytes
//  <128=>128 bytes
//  <256=>256 bytes
//  <512=>512 bytes
//  <1024=>1024 bytes
//  <2048=>2048 bytes
//  <4096=>4096 bytes
// <i> Set the size of the stack guard page.
#define CONFIG_STACK_GUARD_SIZE 256
// </e>

// <o> Base Stack Size [bytes] <2048-65536:8>
// <i> Stack space reserved for the core of the Smart Remote 3 nRF52 firmware.
#define CONFIG_STACK_SIZE_BASE 2048
// </h>

// <q> Enable nRF52 DC/DC Converter
// <i> Use the nRF52 DC/DC converter. For details on the DC/DC converter, see the nRF52 Product Specification document.
#define CONFIG_DCDC_ENABLED 1
// </h>

// <e> Debug Build
// <i> This option enables the debugging infrastructure of the Smart Remote 3 nRF52 firmware.
#define CONFIG_DEBUG_ENABLED 1

// <e> Enable Debug Pins
// <i> Enable the pins that are used for debugging.
#define CONFIG_DEBUG_PIN_ENABLED (1 && CONFIG_DEBUG_ENABLED)

// <o> CONFIG_IO_DBG_MARK Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_MARK 0xFFFFFFFF

// <o> CONFIG_IO_DBG_CLOCK Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_CLOCK 0xFFFFFFFF

// <o> CONFIG_IO_DBG_DATA Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_DATA 0xFFFFFFFF

// <o> CONFIG_IO_DBG_HALF Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_HALF 0xFFFFFFFF

// <o> CONFIG_IO_DBG_FULL Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_FULL 0xFFFFFFFF

// <o> CONFIG_IO_DBG_PCM Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_PCM 0xFFFFFFFF

// <o> CONFIG_IO_DBG_SLEEP Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_SLEEP 0xFFFFFFFF

// <o> CONFIG_IO_DBG_PWM_INT Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_PWM_INT 0xFFFFFFFF

// <o> CONFIG_IO_DBG_SIRC_SACK Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_SIRC_SACK 0xFFFFFFFF

// <o> CONFIG_IO_DBG_SIRC_EACK Pin
//     <0=>P0.0   <1=>P0.1   <2=>P0.2   <3=>P0.3   <4=>P0.4   <5=>P0.5   <6=>P0.6   <7=>P0.7
//     <8=>P0.8   <9=>P0.9   <10=>P0.10 <11=>P0.11 <12=>P0.12 <13=>P0.13 <14=>P0.14 <15=>P0.15
//     <16=>P0.16 <17=>P0.17 <18=>P0.18 <19=>P0.19 <20=>P0.20 <21=>P0.21 <22=>P0.22 <23=>P0.23
//     <24=>P0.24 <25=>P0.25 <26=>P0.26 <27=>P0.27 <28=>P0.28 <29=>P0.29 <30=>P0.30 <31=>P0.31
//     <0xFFFFFFFF=>Disable
#define CONFIG_IO_DBG_SIRC_EACK 0xFFFFFFFF
// </e>

// <e> Enable Event Logger
// <i> Event Logger is a tool that collects event logging information and displays it on the RTT console.
// <i> Event Logger stores event entries in a queue where they are processed and placed in a buffer.
// <i> RTT picks up this information from the buffer when the system is idle.
#define CONFIG_DEBUG_EVENT_LOGGER_ENABLED (1 && CONFIG_DEBUG_ENABLED)

// <o> Log Level <0-2>
// <i> Set a number from range 0-2. The higher the log level, the more logs will appear in the Event Logger tool.
#define CONFIG_DEBUG_EVENT_LOGGER_LOG_LEVEL 2

// <o> Event Queue Size [entries] <1-254>
// <i> Define how many events Event Logger can keep in its queue. If the number of events exceeds the queue size, the oldest ones are replaced.
#define CONFIG_DEBUG_EVENT_LOGGER_QSIZE 16

// <o> Segger RTT Buffer Size (upstream) [bytes] <16-4096>
// <i> Define the Segger RTT upstream buffer size when it connects to Event Logger.
#define CONFIG_DEBUG_RTT_BUFFER_SIZE_UP 2048

// <o> Segger RTT Buffer Size (downstream) [bytes] <16-4096>
// <i> Define the Segger RTT downstream buffer size when it connects to Event Logger.
#define CONFIG_DEBUG_RTT_BUFFER_SIZE_DOWN 16

// <o> Segger RTT ptintf() Buffer Size [bytes] <16-1024>
// <i> Set the maximum length of a single message that can be logged atomically.
#define CONFIG_DEBUG_RTT_PRINTF_BUFFER_SIZE 96
// </e>

// <e> Enable Event Bus Monitor
// <i> Enable monitoring of the Event Bus activity on the RTT console.
#define CONFIG_EVENT_MONITOR_ENABLED (1 && CONFIG_DEBUG_EVENT_LOGGER_ENABLED)

// <o.0> Monitor System Events
// <i> Monitor system events in the Event Bus monitor.
// <o.1> Monitor Button Events
// <i> Monitor Button events in the Event Bus monitor.
// <o.2> Monitor Comunications Events
// <i> Monitor Comunications events in the Event Bus monitor.
#define CONFIG_EVENT_MONITOR_TYPES 0x07
// </e>

// <e> Enable Memory Allocation Debugging
// <i> Check whether memory is allocated correctly. Enabling this option turns on simple memory allocation debugging procedures on the RTT console.
// <i> The other options in this section offer additional debugging but consume more resources.
#define CONFIG_BALLOC_DEBUG_ENABLED (0 && CONFIG_DEBUG_ENABLED)

// <q> Enable Double Free Check
// <i> Check whether memory is not double-freed in your code.
#define CONFIG_BALLOC_ENABLE_DOUBLE_FREE_CHECK 1

// <q> Enable Data Trashing
// <i> Check whether allocated memory is not used after it has been freed.
#define CONFIG_BALLOC_ENABLE_DATA_TRASHING 0

// <o> Head Guard Words <0-255>
// <i> Set how many head guard words will be added to allocated elements. One word is 4 bytes.
#define CONFIG_BALLOC_HEAD_GUARD_WORDS  0

// <o> Tail Guard Words <0-255>
// <i> Set how many tail guard words will be added to allocated elements. One word is 4 bytes.
#define CONFIG_BALLOC_TAIL_GUARD_WORDS  0
// </e>

// <q> Enable CPU Usage Monitor
// <i> Once per second, the CPU Usage Monitor displays the CPU usage for the last second.
#define CONFIG_CPU_USAGE_MONITOR_ENABLED (0 && CONFIG_DEBUG_ENABLED)

// <q> Enable Stack Usage Profiler
// <i> When the system switches to sleep mode, Stack Usage Profiler displays the stack usage report in the given run.
#define CONFIG_STACK_PROFILER_ENABLED (0 && CONFIG_DEBUG_ENABLED)

// <q> Enable J-Link Monitor
// <i> When monitor mode debugging is enabled, CPU could service time-critical tasks when application is halted.
// <i> As result wireless link could be maintained during debugging session.
#define CONFIG_JLINK_MONITOR_ENABLED (0 && CONFIG_DEBUG_ENABLED)

// </e>

// <e> RTT Print
// <i> Enable RTT Print.
/**@brief Debug Build */
#define CONFIG_RTT_DEBUG_ENABLED 1

// <q> Enable Protocol Daily Sport
// <i> Enable m_protocol_daily_sport.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_DAILY_SPORT_ENABLED 1

// <q> Enable Protocol Dev Info
// <i> Enable m_protocol_devinfo.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_DEVINFO_ENABLED 1

// <q> Enable Protocol Interact
// <i> Enable m_protocol_interact.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_INTERACT_ENABLED 1

// <q> Enable Protocol DEV
// <i> Enable m_protocol_devinfo.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_DEVINFO_ENABLED 1


// <q> Enable Protocol PACK
// <i> Enable m_protocol_pack_trans.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_PACK_ENABLED 1


// <q> Enable Protocol Link
// <i> Enable m_protocol_link.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_LINK_ENABLED 1

// <q> Enable Protocol Pack Translate
// <i> Enable m_protocol_pack_trans.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_PACK_TRANS_ENABLED 1

// <q> Enable Protocol
// <i> Enable protocol.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_ENABLED 1

// <q> Enable UTC
// <i> Enable utc.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_UTC_ENABLED 1

// <q> Enable AD_DataStore
// <i> Enable AD_DataStore.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_AD_DATASTORE_ENABLED 1

// <q> Enable App Alarm Clock
// <i> Enable app_alarm_clock.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_APP_ALARM_CLOCK_ENABLED 1

// <q> Enable AD Shake
// <i> Enable AD_ShakeHandAlgorithm.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_AD_SHAKE_ENABLED 1

// <q> Enable AD SLepp
// <i> Enable AD_SleepAlgorithm.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_AD_SLEEP_ENABLED 1

// <q> Enable Idle Judge
// <i> Enable idle_judge.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_IDLE_JUDGE_ENABLED 1

// <q> Enable gSensor
// <i> Enable m_gsensor.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_GSENSOR_ENABLED 1

// <q> Enable Protocol Sensor Translate
// <i> Enable m_protocol_sensor_trans.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_SENSOR_TRANS_ENABLED 1

// <q> Enable Protocol interact 
// <i> Enable m_protocol_interact.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PROTOCOL_INTERACT_ENABLED 1

// <q> Enable ANCS
// <i> Enable m_ble_ancs_c.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_ANCS_C_ENABLED 1

// <q> Enable DFU
// <i> Enable m_ble_dfu.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_DFU_ENABLED 1

// <q> Enable LOWSPP
// <i> Enable m_ble_lowspp.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_LOWSPP_ENABLED 1

// <q> Enable MTU
// <i> Enable m_ble_mtu.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_MTU_ENABLED 1

// <q> Enable BATT
// <i> Enable m_batt_meas.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_BATT_ENABLED 1

// <q> Enable BTN_ACT
// <i> Enable m_drv_btn_action.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_BTN_ACT_ENABLED 1

// <q> Enable Button
// <i> Enable m_button.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_BUTTON_ENABLED 1

// <q> Enable WatchHand
// <i> Enable m_watch_hand.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_WATCH_HAND_ENABLED 1

// <q> Enable DrvWatchHand
// <i> Enable drv_watch_hand.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_DRV_WATCH_HAND_ENABLED 1



// <q> Enable Driver Leds
// <i> Enable m_drv_led.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_DRV_LED_ENABLED 1





// <q> Enable SPI
// <i> Enable m_drv_spi.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_SPI_ENABLED 1

// <q> Enable W25QXX Flash
// <i> Enable m_drv_w25x80.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_W25X80_ENABLED 1

// <q> Enable Leds
// <i> Enable m_leds.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_LEDS_ENABLED 1




// <q> Enable Flash Operation
// <i> Enable m_flash_op.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_FLASH_OP_ENABLED 1

// <q> Enable FS
// <i> Enable m_fs.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_FS_ENABLED 1

// <q> Enable protocol.c
// <i> Enable proto.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PORTO_ENABLED 1

// <q> Enable Pack Send
// <i> Enable m_pack_send.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_PACK_SEND_ENABLED 1

// <q> Enable Event Bus
// <i> Enable event_bus.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_EVENT_BUS_ENABLED 1

// <q> Enable M_COMS
// <i> Enable m_coms.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_COMS_ENABLED 1

// <q> Enable M_COMS_BLE
// <i> Enable m_coms_ble.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_COMS_BLE_ENABLED 1

// <q> Enable DataBase
// <i> Enable m_database.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_DATABASE_ENABLED 1

// <q> Enable SYS
// <i> Enable m_system_state.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_SYS_ENABLED 1

// <q> Enable Main
// <i> Enable main.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_MAIN_ENABLED 1

// <q> Enable system_state
// <i> Enable m_system_state.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_SYSTEM_STATE_ENABLED 1


// <q> Enable BLE CONN PARAMS
// <i> Enable ble_conn_params.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_CONN_PARAMS_ENABLED 1

// <q> Enable Driver Magnet
// <i> Enable m_drv_qmc7983.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_QMC7983_ENABLED 1

// <q> Enable Driver UV
// <i> Enable drv_ltr390.c RTT.
/**@brief Debug Build */
#define CONFIG_RTT_DRV_UV_ENABLED 1

// </e>

// <<< end of configuration section >>>

//#define WH_IND_POS_ALARM_CLOCK      00

//#define WH_IND_POS_MSG     20
//#define WH_IND_POS_CALL    40

//#define WH_IND_STEPS_TIME           5000
//#define WH_IND_STEPS_POS_START      40
//#define WH_IND_STEPS_POS_END        20
//#define WH_IND_STEPS_INTV_NUM       20
//#define WH_IND_STEPS_INTV           5 //(100/WH_IND_STEPS_INTV_NUM)

#endif /* _c003s_config_P1011_H */
