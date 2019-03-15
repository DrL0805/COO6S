
#include "algorithm_abc.h"
#include <math.h>
#include <string.h>
#include "sr3_config.h"
#include "includes.h"


#if CONFIG_GEOMAGNETIC_ENABLED

#if CONFIG_RTT_MAG_ENABLED
#define MAG_Print(M, ...)    SEGGER_Print("ALGO", M, ##__VA_ARGS__)
#else
#define MAG_Print(M, ...)
#endif
/****************************************
***** 指南针相关 ************************
****************************************/
#define PI 3.14159265f
#define CALIBRATION_FLAG 0x5A
#define CALIBRATION_MASK 0x0FFF
#define CALIBRATION_TIMEOUT 1500 /* * 1500 * 40 ms  = 60s*/

static uint16_t CalCnt, CalFlagR, CalFlagL, oldAngle; /* 校准计数,校准过程标志 */
static uint8_t CalState;
static CalParm_t CalParm; /* 校准参数 */
static int16_t Xmax = -32767;
static int16_t Xmin = 32767;
static int16_t Ymax = -32767;
static int16_t Ymin = 32767;

/* 校准平滑 */
#define SMOOTH_NUM 12
static uint8_t smoothCnt = 0;
static int16_t smoothArrX[SMOOTH_NUM];
static int16_t smoothArrY[SMOOTH_NUM];


static void send_mag_measure_raw_data(int16_t data[3],int16_t x_data,int16_t y_data)  //发送mag raw数据
{
    static uint8_t buf[8];
    buf[0] =  (data[0] >> 8) & 0xff;
    buf[1] =  data[0] & 0xff;
    
    buf[2] =  (data[1] >> 8) & 0xff;
    buf[3] =  data[1] & 0xff;
    
    buf[4] =  (x_data >> 8) & 0xff;
    buf[5] =  x_data & 0xff;
    
    buf[6] =  (y_data >> 8) & 0xff;
    buf[7] =  y_data & 0xff;
    protocol_send_data(buf,8);
}

/*********************************
**指南针初始化，读取保存的校准参数
**input: 保存的校准参数 uint8_t len:17 byte
**output: null
**********************************/
uint8_t get_cal_flag(void)
{
    return CalParm.Flag;
}

void alg_compass_init(uint8_t *calParm)
{
    memcpy(&CalParm, calParm, sizeof(CalParm_t));
    if (CalParm.Flag != CALIBRATION_FLAG)
    {
        CalParm.Flag = 0;
        CalState = COMPASS_CAL_NONE;
        CalParm.Xa = 1;
        CalParm.Xb = 0;
        CalParm.Ya = 1;
        CalParm.Yb = 0;
    }
    else
    {
        CalState = COMPASS_CAL_DONE;
    }
}

/*******************************
**指南针校准初始化,校准前调用
**input: null
**output: null
********************************/
void alg_compass_calibration_init(void)
{
    CalParm.Flag = 0;
    CalState = COMPASS_CAL_NONE;
    CalParm.Xa = 1;
    CalParm.Xb = 0;
    CalParm.Ya = 1;
    CalParm.Yb = 0;
    CalCnt = 0;
    CalFlagR = 0;
	CalFlagL = 0;
    oldAngle = 0;

    Xmax = -32767;
    Xmin = 32767;
    Ymax = -32767;
    Ymin = 32767;
	
	smoothCnt = 0;
	for(uint8_t i = 0; i < SMOOTH_NUM; i++)
	{		
		smoothArrX[i] = 0;
		smoothArrY[i] = 0;
	}
}

/*******************************
**指南针校准，校准方法：进入校准模式后，提示用户按特定姿势转动手表，
**校准前先调用初始化函数，再按10HZ频率调用校准函数，传入地磁传感器数据，校准成功后返回01, 外部读取并保存校准参数至flash.校准失败返回02
**input: magValue 地磁三轴数据
**output: 校准结果 00:校准中，01:校准成功，02:校准失败。calParm[17]:校准参数，校准完成后保存至flash
********************************/
calState_e alg_compass_calibration(int16_t *magValue, uint8_t *calParm)
{
    float magX, magY;
    uint16_t angle;
	
	/* 对地磁数据滤波 */
	int32_t xSum = 0, ySum = 0;
	if (smoothCnt >= SMOOTH_NUM)
    {
        for (uint8_t i = 0; i < (SMOOTH_NUM - 1); i++)
        {
            smoothArrX[i] = smoothArrX[i + 1];
			smoothArrY[i] = smoothArrY[i + 1];
        }
        smoothCnt = SMOOTH_NUM - 1;
    }
    smoothArrX[smoothCnt] = magValue[0];
	smoothArrY[smoothCnt] = magValue[1];
    smoothCnt++;
    for (uint8_t i = 0; i < smoothCnt; i++)
    {
        xSum += smoothArrX[i];
        ySum += smoothArrY[i];
    }
    magValue[0] = xSum / smoothCnt;
    magValue[1] = ySum / smoothCnt;
	
    CalCnt++;
    /* 持续更新X，Y轴最大值最小值 */
    if (magValue[0] > Xmax)
    {
        Xmax = magValue[0];
    }
    if (magValue[1] > Ymax)
    {
        Ymax = magValue[1];
    }
    if (magValue[0] < Xmin)
    {
        Xmin = magValue[0];
    }
    if (magValue[1] < Ymin)
    {
        Ymin = magValue[1];
    }
    /* 将变化范围大的轴系数设为1 */
    if ((Xmax - Xmin) > (Ymax - Ymin))
    {
        CalParm.Xa = 1;
        CalParm.Ya = (float)(Xmax - Xmin) / (float)(Ymax - Ymin);
    }
    else
    {
        CalParm.Ya = 1;
		CalParm.Xa = (float)(Ymax - Ymin) / (float)(Xmax - Xmin);
    }
    CalParm.Xb = CalParm.Xa * ((Xmax - Xmin) * 0.5 - Xmax);
    CalParm.Yb = CalParm.Ya * ((Ymax - Ymin) * 0.5 - Ymax);


    /* 计算角度 */
    magX = CalParm.Xa * magValue[0] + CalParm.Xb;
    magY = CalParm.Ya * magValue[1] + CalParm.Yb;
    

    double heading = atan2(magX, magY);
    if (heading < 0)
    {
        heading += 2 * PI;
    }
    if (heading > 2 * PI)
    {
        heading -= 2 * PI;
    }
    angle = heading * 180.0 / PI;

    /* 角度变大 */
    if (angle >= oldAngle)
    {
        CalFlagR |= 1 << ((angle + 1) / 30); /* 当转到30度的倍数角度时，将对应位置1，全置1时即覆盖到0-360度后即校准成功 */
    }
    else
    {
		CalFlagR = 0;
    }
	/* 角度变小 */
    if (angle <= oldAngle)
    {
        CalFlagL |= 1 << ((angle + 1) / 30); /* 当转到30度的倍数角度时，将对应位置1，全置1时即覆盖到0-360度后即校准成功 */
    }
    else
    {
		CalFlagL = 0;
    }
    /* 顺时针或逆时针旋转一圈，能一次遍历到0-360度即为校准成功 */
    if (((CalFlagR & CALIBRATION_MASK) == CALIBRATION_MASK) || ((CalFlagL & CALIBRATION_MASK) == CALIBRATION_MASK))
    {
        CalParm.Flag = CALIBRATION_FLAG;
        memcpy(calParm, &CalParm, sizeof(CalParm_t));
        CalState = COMPASS_CAL_DONE;
        return COMPASS_CAL_DONE;
    }
    /* 超时未遍历到0-360度，校准失败 */
    else if (CalCnt > CALIBRATION_TIMEOUT)
    {
        CalState = COMPASS_CAL_FAIL;
        return COMPASS_CAL_FAIL;
    }

    oldAngle = angle;

    CalState = COMPASS_CAL_ING;
    return COMPASS_CAL_ING;
}

/**************************************************************
**0-359度, 获取与北极的角度
**input: magValue 地磁三轴数据
**output: compassValue:angle:与北极的夹角,calState:校准状态
***************************************************************/
calState_e alg_compass_getAngle(int16_t *magValue, uint16_t *angle)
{
    float magX, magY,temp_magX, temp_magY;

    if (CalParm.Flag != CALIBRATION_FLAG)
    {
        /* 未校准 */
        *angle = 0;
        if (CalState == COMPASS_CAL_ING)
        {
            return COMPASS_CAL_ING;
        }
        return COMPASS_CAL_NONE;
    }
	/* 计算角度 */
    magX = CalParm.Xa * magValue[0] + CalParm.Xb;
    magY = CalParm.Ya * magValue[1] + CalParm.Yb;
  
    
    //  45°
    temp_magX = 0.707f*(magX - magY);
    temp_magY = 0.707f*(magX + magY) /1.56f;
   

	

    
    send_mag_measure_raw_data(magValue,temp_magX,temp_magY);
    
    
    double heading = atan2(temp_magX, temp_magY);
    if (heading < 0)
    {
        heading += 2 * PI;
    }
    if (heading > 2 * PI)
    {
        heading -= 2 * PI;
    }
    uint16_t angle_temp = (heading * 180.0 / PI) + 45;
    *angle = angle_temp % 360;

    return COMPASS_CAL_DONE;
}

#endif
