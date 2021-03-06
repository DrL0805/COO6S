#ifndef __M_DRV_QMC7983_H__
#define __M_DRV_QMC7983_H__

#include <stdint.h>

//**********************************************************************
// 函数功能:    地磁传感器初始化
//**********************************************************************
uint8_t drv_mag_init(void);

//**********************************************************************
// 函数功能:  读取地磁传感器3轴数据，，数据为二进补码形式
//**********************************************************************
uint8_t drv_mag_read(int16_t *magData);

//**********************************************************************
// 函数功能:    地磁传感器唤醒
//**********************************************************************
uint8_t drv_mag_wake(void);

//**********************************************************************
// 函数功能:    设置地磁传感器进入睡眠状态
//**********************************************************************
uint8_t drv_mag_sleep(void);

uint32_t	m_mag_init(void);

#endif
