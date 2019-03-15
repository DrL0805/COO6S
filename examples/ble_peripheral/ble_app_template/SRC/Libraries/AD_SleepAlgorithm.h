#include "m_drv_kx022.h"

extern void event_GsensorSumData(void);//计算加速度累加事件
void getGsensorDiffData(unsigned int * data);//返回当前的G_sensor累加和
void setGsensorDiffData(unsigned int data);//设置G_sensor累加和
void gensor_set_1hz(void);
