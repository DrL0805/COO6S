#include "m_drv_kx022.h"

extern void event_GsensorSumData(void);//������ٶ��ۼ��¼�
void getGsensorDiffData(unsigned int * data);//���ص�ǰ��G_sensor�ۼӺ�
void setGsensorDiffData(unsigned int data);//����G_sensor�ۼӺ�
void gensor_set_1hz(void);
