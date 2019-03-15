#include "AD_ShakeHandAlgorithm.h"
#include "m_database.h"
#include "m_drv_kx022.h"
//#include "app_debug_event_logger.h"

#include "includes.h"

#if CONFIG_RTT_AD_SHAKE_ENABLED
#define SHAKE_Print(M, ...)	SEGGER_Print("SHAKE", M, ##__VA_ARGS__)
#else
#define SHAKE_Print(M, ...)
#endif

void WristMotionDetect(unsigned char idx);
static unsigned char WristMotionDetectKey = 0;//非0开，0关

void shake_hand_enable(void)
{
        if(WristMotionDetectKey == 0)
        {
                WristMotionDetect(0xff);
        }
        WristMotionDetectKey =1;
}

void shake_hand_disable(void)
{
        WristMotionDetectKey = 0;
}
    
void (*shake_hand_evt_handler_t_kk)(shake_hand_evt_t p_evt) = 0;

void shake_hand_init(void (*handler)(shake_hand_evt_t p_evt))
{
        shake_hand_evt_handler_t_kk = handler;
}
unsigned char VibraTimeoutCount =0;
void SetVibraTimeout(unsigned char Count)
{
            VibraTimeoutCount = Count+1;
}

unsigned char Setp2STpyTimeoutCount =0;
void SetSetp2STpyTimeout(unsigned char Count)
{
        Setp2STpyTimeoutCount = Count;
}

/*static int UseAbs(unsigned short a,unsigned short b)
{
        if(a>b) return a-b;
        else return b-a;
}*/

#define DATA_SMOOTHNESS_WINDOWS_LEN  4
unsigned int DataSmoothnessProcess(unsigned short data,unsigned char key)//????
{
        unsigned int Ltemp = 0;
        unsigned char i = 0;
        static unsigned char pp = 255;
        static unsigned short TempSumOld[DATA_SMOOTHNESS_WINDOWS_LEN] ={0};

        if(key)
        {
                pp =255;
                return 0;
        }
        if(pp == 255)
        {
                for(i =0 ;i<DATA_SMOOTHNESS_WINDOWS_LEN; i++)
                    TempSumOld[i] = data;
        }

        pp++;
        if(pp > (DATA_SMOOTHNESS_WINDOWS_LEN-1)) pp=0;
        TempSumOld[pp] =data;

        for(i=0;i<DATA_SMOOTHNESS_WINDOWS_LEN;i++)
            Ltemp += TempSumOld[i];
        return (Ltemp /DATA_SMOOTHNESS_WINDOWS_LEN);
}
#define DATA_UP_WINDOWS_LEN   3 //???3,
#define UP_THRESHOLD_VALUE_A 10000
#define UP_THRESHOLD_VALUE_B 10000
#define UP_THRESHOLD_VALUE_LOW 1000
//#define UP_THRESHOLD_VALUE_2A 15000
//#define UP_THRESHOLD_VALUE_1A 15000

int DifferenceAB(unsigned short DataA,unsigned short DataB)
{
        if(DataA > DataB)
        {
                if((DataA - DataB)<30000)
                    return (DataA - DataB);
                else
                    return (30000);
        }
        else
        {
                if((DataB - DataA)<30000)
                    return (DataA - DataB);
                else
                    return (-30000);
        }
}

void DifferenceValueProcess(unsigned short data ,unsigned char key,short *p)
{
        static unsigned short DataOldx = 0;
        static short DataOld[DATA_UP_WINDOWS_LEN] = {0};
        static unsigned char pp = 255;
        short temp = 0;
        unsigned char i = 0;

        if(key == 1)
        {
                pp=255;
                return;
        }
        if(pp == 255)
        {
                DataOldx = data;
                for(i=0;i<DATA_UP_WINDOWS_LEN;i++)
                {
                        DataOld[i] = 0;
                }
                pp = 0;
        }
        temp = DifferenceAB(data,DataOldx);
        DataOldx = data;

        if(temp < 1000 && temp >0) temp = 0;
        if(temp > -500 && temp <0 ) temp = 0;
        if(pp >2) pp =0;
        DataOld[pp] = temp;
        pp++;

        p[0] = DataOld[0];
        p[1] = DataOld[1];
        p[2] = DataOld[2];

    //return DataOld;
}

unsigned char DataProcess(short *data)
{
//      char p[20]={0};
//      unsigned long int Ltemp = 0;
        unsigned char i = 0;
        unsigned char k = 0;

        for(i=0;i<DATA_UP_WINDOWS_LEN;i++)
        {
                if(data[i] >0) k++;
        }
        if(k == 3)
        {
                if((data[0]+data[1]+data[2])>UP_THRESHOLD_VALUE_A)
                {
                        return 2;
                }else if((data[0]+data[1]+data[2])>UP_THRESHOLD_VALUE_B)
                {
                        return 1;
                }
                else return 0;
        }
        if(k == 2)
        {
            if(data[1] <= 0)
            {
                    if(data[0]>UP_THRESHOLD_VALUE_A||data[2]>UP_THRESHOLD_VALUE_A)
                    {
                            return 2;
                    }
                    else if(data[0]>UP_THRESHOLD_VALUE_B||data[2]>UP_THRESHOLD_VALUE_B)
                    {
                            return 1;
                    }
                    else
                            return 0;
            }
            if(data[0] <= 0)
            {
                    if((data[1]+data[2])>UP_THRESHOLD_VALUE_A)
                    {
                            return 2;
                    }
                    else if((data[1]+data[2])>UP_THRESHOLD_VALUE_B)
                    {
                            return 1;
                    }
                    else return 0;
            }
            if(data[2] <= 0)
            {
                    if((data[1]+data[0])>UP_THRESHOLD_VALUE_A)
                    {
                            return 2;
                    }
                    else if((data[1]+data[0])>UP_THRESHOLD_VALUE_B)
                    {
                            return 1;
                    }
                    else return 0;
            }
        }
        if(k == 1)
        {
                if((data[0]>UP_THRESHOLD_VALUE_A)||(data[1]>UP_THRESHOLD_VALUE_A)||(data[2]>UP_THRESHOLD_VALUE_A))
                {
                        return 2;
                }
                else if((data[0]>UP_THRESHOLD_VALUE_B)||(data[1]>UP_THRESHOLD_VALUE_B)||(data[2]>UP_THRESHOLD_VALUE_B))
                {
                        return 1;
                }
                else
                        return 0;
        }
        return 0;
}

#define DATA_RESET_TIME_THRESHOLD_VALUE 5
unsigned char DataReset(short *data)
{
    //  char p[20]={0};
    //  unsigned long int Ltemp = 0;
        unsigned char i = 0;
        unsigned char k = 0;

        for(i=0;i<3;i++)
        {   
                if(data[i] < (-1000)) k++;
        }
        if(k == 3)
        {
                if((data[0]+data[1]+data[2])<(-2000))
                        return 0;
                else return 1;
        }
        if(k == 2)
        {
                if(data[1] >= 0)
                {
                        if(data[0]<(-2000)||data[2]<(-2000))
                                return 0;
                        else
                                return 1;
                }
                if(data[0] >= 0)
                {
                        if((data[1]+data[2])<(-2000))
                                return 0;
                        else return 1;
                }
                if(data[2] >= 0)
                {
                        if((data[1]+data[0])<(-2000))
                                return 0;
                        else return 1;
                }
        }
        if(k == 1)
        {
                if((data[0]<(-2000))||(data[1]<(-2000))||(data[2]<(-2000)))
                        return 0;
                else
                        return 1;
        }
        return 1;
}

unsigned char GesturesProcess(unsigned char *data,unsigned char key)
{
        static unsigned char ResetTpy =0;
        unsigned char temp = 0;
        unsigned short TempSum =0;
        unsigned short Ltemp = 0;
        short DataBuff[3];
        unsigned char i = 0,k=0;

        if(key == 1)
        {
                ResetTpy =0;
                return 0;
        }
        for(i=0;i<3;i++)
        {
                temp = data[i];
                if(temp&0x80)
                {
                        temp = ((~temp)+1);
                }
                TempSum +=(temp*temp);
        }

        Ltemp = DataSmoothnessProcess(TempSum,0);
        DifferenceValueProcess(Ltemp,0,DataBuff);

        if(ResetTpy == 0)
        {
                ResetTpy = DataProcess(DataBuff);
                k = ResetTpy;
        }
        else
        {
                ResetTpy = DataReset(DataBuff);
        }

        return k;
}

void GesturesProcessRead(void)
{
        DataSmoothnessProcess(0,1);
        DifferenceValueProcess(0,1,0);
        GesturesProcess(0,1);
}

extern sAXIS GsensorDate[82];
void WristMotionDetect(unsigned char idx)
{
        unsigned char i = 0;
        static short index = -2;
        unsigned char data[3] = {0};
        shake_hand_evt_t Temp= SHAKE_HAND_EVT_TRIGGERED;

        if (idx == 0xFF) //初始化
        {
                index = -14;
                return;
        }
        
        for (i = 0; i < idx - 1; i++)
        {
                //SHAKE_Print("Tets1: %d", idx);
                //SHAKE_Print("Tets2: %d", index);
                if(index < 0||VibraTimeoutCount > 0||Setp2STpyTimeoutCount>0)
                {
                        GesturesProcessRead();
                        //Count= 0;
                        if((index < 0)&&((index + idx)>=0))
                            index =0;
                        else if(index <0)
                            index += idx;
                        return;
                }
                
                data[0] =  GsensorDate[i].x;
                data[1] =  GsensorDate[i].y;
                data[2] =  GsensorDate[i].z;

                if(GesturesProcess(data,0)) //产生一个甩手事件
                {
                        SHAKE_Print("shake_hand: %d", 0);
                        if(shake_hand_evt_handler_t_kk !=0)
                        {
                                shake_hand_evt_handler_t_kk(Temp);
                        }
                        
                }
        }

        
}
unsigned char  wristMotionDetectProcess(unsigned char idx)
{
        if(VibraTimeoutCount > 0)
        {
                VibraTimeoutCount --;
        }
        if(Setp2STpyTimeoutCount>0)
        {
                Setp2STpyTimeoutCount --;
        }
        if(WristMotionDetectKey == 0)//开关是关的
        {
                
                return 0;
        }
        else
        {
                
                WristMotionDetect(idx);
                return 1;
        }
        
}
