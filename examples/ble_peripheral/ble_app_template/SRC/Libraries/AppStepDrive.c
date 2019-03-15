
#include "sdk_common.h"

//#define NRF_LOG_MODULE_NAME "S"
//#include "nrf_log.h"
//#include "nrf_log_ctrl.h"

#include "m_drv_kx022.h"

#include "m_database.h"


//#include "app_debug_event_logger.h"
#include "event_bus.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "m_drv_kx022.h"
#include "AppStepDrive.h"

#include "app_timer.h"
#include "m_gsensor.h"
#include "nrf_pwr_mgmt.h"
#include "m_database.h"
#include "app_time_slot.h"

#include "resources.h"
//#include "sr3_config.h"
#include "AD_DataStore.h"
#include "AD_SleepAlgorithm.h"
#include "AD_ShakeHandAlgorithm.h"
#include "SitLongTime.h"

#define StepUp1                     125//125
#define StepUp2                     55//55
#define StepDown1                   110//115
#define StepDown2                   45//45

typedef struct
{
    unsigned char     up:1;
    unsigned char     down:1;
    unsigned char     ninestep:1;
    unsigned char     faststep:1;
    unsigned char     finishstep:1;
} __Lis3DStepFlag;

static __Lis3DStepFlag  Lis3DStepFlag = {0};
static uint32_t gSteps = 0;

extern sAXIS GsensorDate[82];
static unsigned char StepGsensorSamplingRate = 0;//1是需要快的，0需要是慢的

static unsigned int X_axis[4] = {0};
static unsigned int Y_axis[4] = {0};
static unsigned int Z_axis[4] = {0};

static unsigned char StepInternalTimer[24] = {0};
static unsigned char DifferenceTime[24] = {0};

static unsigned int x2y2z2[5] = {0};

static unsigned char ReadDataCount =0;
static unsigned char ReplaceCount =0;
static unsigned char NineStepCount =0;
static unsigned char Count_46_7ms_Lis3dh =1;
static unsigned char BiggerCount =0;

static unsigned int CompareData =0;
static unsigned int MaxAcceleration =0, MinAcceleration =0;
static unsigned char InvalidTime =0;

unsigned int gSteps_get(void)
{
    return gSteps;
}


void gSteps_set(unsigned int data)
{
    gSteps = data;
}

void CalculateStepDataProcess(unsigned char xyzData[3])
{
    unsigned int    Ave_Xaxis, Ave_Yaxis, Ave_Zaxis;
    if(xyzData[0] & 0x80){xyzData[0] = ~xyzData[0] + 1; }
    if(xyzData[1] & 0x80){xyzData[1] = ~xyzData[1] + 1; }
    if(xyzData[2] & 0x80){xyzData[2] = ~xyzData[2] + 1; }
    X_axis[ReadDataCount] = xyzData[0] / 2;
    Y_axis[ReadDataCount] = xyzData[1] / 2;
    Z_axis[ReadDataCount] = xyzData[2] / 2;
    if(Lis3DStepFlag.faststep)
    {
       if(ReadDataCount == 0)
       {
           Ave_Xaxis = (X_axis[0] + X_axis[3]) >> 1;
           Ave_Yaxis = (Y_axis[0] + Y_axis[3]) >> 1;
           Ave_Zaxis = (Z_axis[0] + Z_axis[3]) >> 1;
       }
       else 
       {
           Ave_Xaxis = (X_axis[ReadDataCount] + X_axis[ReadDataCount - 1]) >> 1;
           Ave_Yaxis = (Y_axis[ReadDataCount] + Y_axis[ReadDataCount - 1]) >> 1;
           Ave_Zaxis = (Z_axis[ReadDataCount] + Z_axis[ReadDataCount - 1]) >> 1;
       }
    }
    else
    {
        Ave_Xaxis = (X_axis[0] + X_axis[1] + X_axis[2] + X_axis[3]) >> 2;
        Ave_Yaxis = (Y_axis[0] + Y_axis[1] + Y_axis[2] + Y_axis[3]) >> 2;
        Ave_Zaxis = (Z_axis[0] + Z_axis[1] + Z_axis[2] + Z_axis[3]) >> 2;
    }
    Y_axis[ReadDataCount] = (unsigned char)(Ave_Yaxis);
    Z_axis[ReadDataCount] = (unsigned char)(Ave_Zaxis);

    x2y2z2[ReadDataCount] = (Ave_Xaxis * Ave_Xaxis + Ave_Yaxis * Ave_Yaxis + Ave_Zaxis * Ave_Zaxis);
    x2y2z2[4] = (x2y2z2[0] / 4 + x2y2z2[1] / 4 + x2y2z2[2] / 4 + x2y2z2[3] / 4);
}
void CalculateStepTimeOut1(void)
{
    if(Count_46_7ms_Lis3dh) {Count_46_7ms_Lis3dh++; }
    if(Count_46_7ms_Lis3dh > 38)
    {
        Count_46_7ms_Lis3dh = 0;
        Lis3DStepFlag.ninestep = 0;
        Lis3DStepFlag.faststep = 0;
        NineStepCount = 0;
        ReplaceCount = 0;
    }
    if(Lis3DStepFlag.finishstep)
     {
        InvalidTime++;
        if(InvalidTime > 4)
        {
            Lis3DStepFlag.finishstep = 0;
        }
    }
}
void CalculateStepDataComp(unsigned char *timecount,unsigned char *tempflag)
{
		
    if(x2y2z2[4] > CompareData)
    {
        if(MaxAcceleration < x2y2z2[4])
        {
            MaxAcceleration = x2y2z2[4];
        }
        if(Lis3DStepFlag.up)
        {        //if has find up
            if(x2y2z2[4] > CompareData + StepUp2)
            {
                CompareData = x2y2z2[4];
            }
        } 
        else
        {
            if(x2y2z2[4] > CompareData + StepUp1)
            {
                CompareData = x2y2z2[4];
                MaxAcceleration = x2y2z2[4];
                Lis3DStepFlag.up = 1;
                Lis3DStepFlag.down = 0;
                *timecount = 0;          //special operate
            }
        }
    }
    else
    {
        if(x2y2z2[4] < MinAcceleration)
        {
            MinAcceleration = x2y2z2[4];
        }
        if(Lis3DStepFlag.down)
        {
            if(CompareData > x2y2z2[4] + StepDown2)
            {
                CompareData = x2y2z2[4];
                *tempflag = 1;
            }
        }
        else
        {
            if(CompareData > x2y2z2[4] + StepDown1)
             {
                CompareData = x2y2z2[4];
                MinAcceleration = x2y2z2[4];
                Lis3DStepFlag.down = 1;
                InvalidTime = 0;
                *tempflag = 1;
                *timecount = 0;          //special operate
            }
        }
    }
    if(*tempflag) 
    {
        if(Lis3DStepFlag.up && (Lis3DStepFlag.finishstep == 0)) //finish one step
        {   
            *tempflag = 0;       //finish one step flag
            Lis3DStepFlag.up = 0;
            Lis3DStepFlag.down = 0;
            Lis3DStepFlag.finishstep = 1;
            InvalidTime = 0;
        }
    }
    else 
    {
        *tempflag = 1;
    }
}
void CalculateStepTimeOut2(unsigned char *timecount,int *stepdata)
{
    ReadDataCount++;
    if(ReadDataCount > 3) 
    {
        ReadDataCount = 0;
    }
    if(*stepdata == 0) 
    {
        (*timecount)++;
    } 
    else 
    {
        *timecount = 0;
    }
    if(*timecount >= 250) 
    {
        (*stepdata) = -2;
        *timecount = 0;
    }
}

static unsigned char StepAbs(unsigned char a,unsigned char b)
{
    if(a > b)
    {
        return a-b;
    }
    else
    {
        return b-a;
    }
}



int CalculateStep(unsigned char xyzData[3])
{
    unsigned char   i;
    unsigned char   tempflag = 0;
    unsigned long   TempData_16;
    int             stepdata = 0;
    static unsigned char timecount = 0;

    unsigned char CalculateStepParameterTime = 0;
    
    if((system_database.utc.hour >0)&&(system_database.utc.hour <6))
    {
            CalculateStepParameterTime =1;
    }
//Lis3dh_Read_xyz(xyzData);
    CalculateStepDataProcess(xyzData);
    CalculateStepTimeOut1();
    CalculateStepDataComp(&tempflag,&tempflag);


    if(tempflag == 0)
    {
        if(Lis3DStepFlag.ninestep) 
        {
            ReplaceCount++;
            if(ReplaceCount > 11) 
            {
                ReplaceCount = 1;
            }
            StepInternalTimer[ReplaceCount] = Count_46_7ms_Lis3dh;
            StepInternalTimer[0] = 0;
            for(i = 1; i < 12; i++) 
            {
                StepInternalTimer[0] += StepInternalTimer[i];
            }
            StepInternalTimer[0] = StepInternalTimer[0] / 11;
            if(Count_46_7ms_Lis3dh > 8 && Count_46_7ms_Lis3dh < 12)
            {
                Lis3DStepFlag.faststep = 1;
            }
            else 
            {
                Lis3DStepFlag.faststep = 0;
            }
            stepdata = 1;
        } 
        else 
        {
            if(ReplaceCount == 0)
            {
                StepInternalTimer[NineStepCount] = Count_46_7ms_Lis3dh;
            } 
            else 
            {
                StepInternalTimer[ReplaceCount] = Count_46_7ms_Lis3dh;
            }
            NineStepCount++;
            if(NineStepCount == (12 <<CalculateStepParameterTime)) 
            {
                BiggerCount = 0;
                TempData_16 = 0;
                for(i = 1; i < (12 <<CalculateStepParameterTime); i++) 
                {
                    TempData_16 += StepInternalTimer[i];
                }
                StepInternalTimer[0] = (unsigned char)(TempData_16 /((11 <<CalculateStepParameterTime)+CalculateStepParameterTime));
        
                for(i = 1; i < (12 <<CalculateStepParameterTime); i++) 
                {
                    DifferenceTime [i] = (StepAbs(StepInternalTimer[i],StepInternalTimer[0]))*4;
                    if(DifferenceTime[i] > StepInternalTimer[0]) 
                    {
                        BiggerCount++;
                    }
                    DifferenceTime [i] = (StepAbs(StepInternalTimer[i],StepInternalTimer[0]))*2;
                    if(DifferenceTime[i] > StepInternalTimer[0])
                    {
                        BiggerCount++;
                    }
                }
                if(StepInternalTimer[0] > (8 <<CalculateStepParameterTime) && StepInternalTimer[0] < (12 <<CalculateStepParameterTime)) 
                {
                    Lis3DStepFlag.faststep = 1;
                }
                else
                {
                    Lis3DStepFlag.faststep = 0;
                }
                if(BiggerCount < (5 <<CalculateStepParameterTime)) 
                {
                    Lis3DStepFlag.ninestep = 1;
                    NineStepCount = 0;
/**************************finish targe step enable every step calculate********************/
					stepdata = (12 <<CalculateStepParameterTime);
/**************************finish targe step enable every step calculate********************/
                } 
                else 
                {
                    NineStepCount--;
                    ReplaceCount++;
                    if(ReplaceCount > 23) 
                    {
                        ReplaceCount = 1;
                    }
                }
            }
        }
        Count_46_7ms_Lis3dh = 1;
    }
    CalculateStepTimeOut2(&timecount,&stepdata);
    return stepdata;
}

void GsensorModeCheck(unsigned char Data)
{
    static char StepModeDetectCount =0;
    if(Data<10) StepModeDetectCount++;
    else StepModeDetectCount --;
    if(StepModeDetectCount > 10)
    {
        StepGsensorSamplingRate = 0;
    }
    else
    {
        StepGsensorSamplingRate = 1;
    }
}

#include "m_drv_w25x80.h"
#include "app_error.h"
#include "app_isched.h"
#include "resources.h"

//static bool save_intv_time_log_init_flag = true;

//#define STEP_INTV_TIME_BUF_LEN                  (sizeof(StepInternalTimer)+4)
//#define STEP_INTV_TIME_FLASH_START_ADDR         0x00020000


//uint8_t log_buf[STEP_INTV_TIME_BUF_LEN] = {0};


//static void save_intv_time_log_sched_handler(void *p_context)
//{
//    static uint32_t flash_addr = STEP_INTV_TIME_FLASH_START_ADDR;

//    if (save_intv_time_log_init_flag)
//    {
//        save_intv_time_log_init_flag = false;
//        w25x80_erase_sector(STEP_INTV_TIME_FLASH_START_ADDR);
//        flash_addr = STEP_INTV_TIME_FLASH_START_ADDR;
//    }

//    uint32_big_encode(system_database.utc_seconds, log_buf);            // 前 4字节，utc时间戳
//    memcpy(&log_buf[4], StepInternalTimer, STEP_INTV_TIME_BUF_LEN);     // 后24字节，周期

//    if (flash_addr >= (STEP_INTV_TIME_FLASH_START_ADDR + SECTOR_SIZE)) // 写满扇区时，需要先擦除扇区
//    {
//        w25x80_erase_sector(STEP_INTV_TIME_FLASH_START_ADDR);
//        flash_addr = STEP_INTV_TIME_FLASH_START_ADDR;
//    }

//    w25x80_write(flash_addr, log_buf, STEP_INTV_TIME_BUF_LEN);

//    flash_addr += STEP_INTV_TIME_BUF_LEN;
//}

//static bool is_save_log_time_valid(void)
//{
//    uint32_t cur_hms = system_database.utc_seconds % 86400; // 当前时分秒(24小时制)

//    if ((cur_hms >= (1 * 3600)) && (cur_hms <= (8 * 3600))) // 凌晨 1 点到 8 点
//    {
//        return true;
//    }

//    return false;
//}

//#include "nrf_log.h"


//static uint8_t page_buf[PAGE_SIZE] = {0};

//void print_intv_time_log(void)
//{
//    uint32_t read_addr = STEP_INTV_TIME_FLASH_START_ADDR;

//    while (read_addr < (STEP_INTV_TIME_FLASH_START_ADDR + SECTOR_SIZE))
//    {
//        APP_ERROR_CHECK(w25x80_read(read_addr, page_buf, PAGE_SIZE));

//        if ( (page_buf[0] == 0xFF) && (page_buf[1] == 0xFF)
//           && (page_buf[2] == 0xFF) && (page_buf[3] == 0xFF) )
//        {
//            break;
//        }

//        NRF_LOG_RAW_HEXDUMP_INFO(page_buf, PAGE_SIZE);
//        read_addr += PAGE_SIZE;
//    }
//}


void stepPro(uint32_t *steps)
{
    unsigned char i = 0;
    int ret;

    if((StepGsensorSamplingRate)||(GetGsensorSamplingRateTpy() == 2))
    {
        while(GsensorDate[i].len > 0)
        {
            uint8_t Temp[3] = {GsensorDate[i].y,GsensorDate[i].x,GsensorDate[i].z};
            ret = CalculateStep(Temp);
            switch (ret)
            {
                case -2:
                {
                    StepGsensorSamplingRate =0;
                    break;
                }
                case -1:break;
                case 0:break;
                default:
                {
                    if (ret > 0)
                    {
//                        if ((is_save_log_time_valid()) && (ret == 12))
//                        {
//                            APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, save_intv_time_log_sched_handler, NULL));
//                        }
                        gSteps += ret;
                        *steps += ret;
                    }
                    break;
                }
            }
            i++;
        }
    }
}
