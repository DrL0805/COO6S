#include "AD_SleepAlgorithm.h"
#include <stdint.h>
//#include "app_debug_event_logger.h"

#include "includes.h"

#if CONFIG_RTT_AD_SLEEP_ENABLED
#define SLEEP_Print(M, ...)	SEGGER_Print("SLEEP", M, ##__VA_ARGS__)
#else
#define SLEEP_Print(M, ...)
#endif

extern sAXIS GsensorDate[82];


#define DATA_WINDOWS_LENA    4
#define DATA_YWA             2
#define DATA_WINDOWS_LENB    2
#define DATA_YWB             1
#define DATA_MAG            10
#define THRESHOLD           2  //2

void getGsensorDiffData(uint32_t * data);//?????G_sensor???
void setGsensorDiffData(uint32_t data);//??G_sensor???

void startOutput_GsensorSumData(void); 
//static uint32_t diff_Gsensor = 0; 



uint32_t ChangeSum = 0;

void getGsensorDiffData(uint32_t * data) 
{
    *data = ChangeSum;
}
void setGsensorDiffData(uint32_t data) 
{
    ChangeSum=data ;
}
void gensor_set_1hz(void)
{
    uint8_t GsensorState = GetGsensorSamplingRateTpy();
    if(GsensorState == 2)
    {
        GsensorSwitch(1);//切换为1hz采样率
        SLEEP_Print("GsensorSwitch: 1hz");
    }
}
int32_t SmoothnessB(int8_t *p)  //平滑
{
    int32_t DataTempx = 0;
    int32_t DataTempy = 0;
    int32_t DataTempz = 0;
    static int8_t DataWindowsX[DATA_WINDOWS_LENB] = { 0 };
    static int8_t DataWindowsY[DATA_WINDOWS_LENB] = { 0 };
    static int8_t DataWindowsZ[DATA_WINDOWS_LENB] = { 0 };
    static uint8_t DataWindowsHand = 0;       
    static uint8_t Count = 0;                 
    int32_t i = 0;

    DataWindowsX[DataWindowsHand] = p[0];
    DataWindowsY[DataWindowsHand] = p[1];
    DataWindowsZ[DataWindowsHand] = p[2];

    if (Count < (DATA_WINDOWS_LENB -1))
    {
        Count++;
        DataWindowsHand++;
        if (DataWindowsHand == DATA_WINDOWS_LENA) DataWindowsHand = 0;
        return 1;
    }

    for (i = 0; i < DATA_WINDOWS_LENB; i++)
    {
        DataTempx += DataWindowsX[i];
        DataTempy += DataWindowsY[i];
        DataTempz += DataWindowsZ[i];
    }
    DataTempx = DataTempx / 2;// >> DATA_YWB;
    DataTempy = DataTempy / 2;// >> DATA_YWB;
    DataTempz = DataTempz / 2;// >> DATA_YWB;

    DataWindowsX[DataWindowsHand] = DataTempx;
    p[0] = DataTempx;
    DataWindowsY[DataWindowsHand] = DataTempy;
    p[1] = DataTempy;
    DataWindowsZ[DataWindowsHand] = DataTempz;
    p[2] = DataTempz;
    DataWindowsHand++;
    if (DataWindowsHand == DATA_WINDOWS_LENB) DataWindowsHand = 0;
    return 0;
}

static uint32_t abs(int8_t x) 
{ 
    return (x >= 0 ? x : -x); 
}

void dataSleepProcess(int8_t *p)
{
    static uint8_t StartTpy = 0;
    static int8_t DataOld[3] = { 0 };
    uint32_t ChangeDataTemp = 0;
    
#define debug 0
#if debug
    uint8_t x_temp = 0;
    uint8_t y_temp = 0;
    uint8_t z_temp = 0;
#endif
  
 
    if (SmoothnessB(p) != 0) return ;

    if (StartTpy == 0)
    {
        DataOld[0] = p[0];
        DataOld[1] = p[1];
        DataOld[2] = p[2];
        StartTpy = 1;
    }

#if debug
    x_temp = (abs(p[0] - DataOld[0])) < THRESHOLD ? 0 : (abs(p[0] - DataOld[0]));
    y_temp = (abs(p[1] - DataOld[1])) < THRESHOLD ? 0 : (abs(p[1] - DataOld[1]));
    z_temp = (abs(p[2] - DataOld[2])) < THRESHOLD ? 0 : (abs(p[2] - DataOld[2]));
    ChangeDataTemp = x_temp + y_temp + z_temp;
    ChangeSum += ChangeDataTemp;
    SLEEP_Print("ChangeSum: %d , ChangeDataTemp : %d ,x_temp :%d , y_temp : %d ,z_temp :%d", ChangeSum,ChangeDataTemp,x_temp,y_temp,z_temp);
#else    
    ChangeDataTemp += (abs(p[0] - DataOld[0])) < THRESHOLD ? 0 : (abs(p[0] - DataOld[0]));
    ChangeDataTemp += (abs(p[1] - DataOld[1])) < THRESHOLD ? 0 : (abs(p[1] - DataOld[1]));
    ChangeDataTemp += (abs(p[2] - DataOld[2])) < THRESHOLD ? 0 : (abs(p[2] - DataOld[2]));
    ChangeSum += ChangeDataTemp;    
#endif
    
#if 1
    uint8_t GsensorState = GetGsensorSamplingRateTpy();
    if((ChangeDataTemp > 0x24)&&(GsensorState == 1))
    {          
        GsensorSwitch(2);//切换成25hz采样率
        SLEEP_Print("GsensorSwitch: 25hz");
    }
//    else if((ChangeDataTemp < 0x06)&&(GsensorState == 2))
//    {
//        GsensorSwitch(1);//切换为1hz采样率
//        SLEEP_Print("GsensorSwitch: 1hz");
//    }
#endif
    
    
    DataOld[0] = p[0];
    DataOld[1] = p[1];
    DataOld[2] = p[2];
}

void event_GsensorSumData(void) 
{
    static uint8_t TimerTick1s = 0; 
    TimerTick1s++; 
    int8_t p[3] = {0,0,0};

    p[0] = GsensorDate[0].x;
    p[1] = GsensorDate[0].y;
    p[2] = GsensorDate[0].z;
    if(GsensorDate[0].len > 0)
    {
            dataSleepProcess(p);
    }
}
