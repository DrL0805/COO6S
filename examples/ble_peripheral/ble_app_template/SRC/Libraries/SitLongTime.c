#include "AD_ShakeHandAlgorithm.h"
#include "m_database.h"
#include "m_drv_kx022.h"
#include "SitLongTime.h"

static unsigned long SitLongTimeCount =0;
static unsigned int SitSitLongTime = 0; 
static unsigned long SitLongTimeSum =0;
void (*app_long_sit_evt_handler_t)(app_long_sit_evt_t evt) = 0;

unsigned int app_long_sit_stop(void)
{
    SitSitLongTime = 0;
    return NRF_SUCCESS;
}

unsigned int app_long_sit_start(unsigned short minutes)
{
    SitSitLongTime = minutes *60;
    SitLongTimeCount = 0;
    SitLongTimeSum = 0;
  //DBG1_EVT1("SitSitLongTime %d",SitSitLongTime);
    return NRF_SUCCESS;
}

unsigned int app_long_sit_init(void (*app_long_sit_evt_handler_t_x)(app_long_sit_evt_t evt))
{
    app_long_sit_evt_handler_t = app_long_sit_evt_handler_t_x;
    return NRF_SUCCESS;
}

unsigned char SitLongTimeCountClone(void) // 步数更新的时候调用
{
    SitLongTimeCount = 0;
    SitLongTimeSum = 0;
    return NRF_SUCCESS;
}

static signed char UseAbs(signed char a,signed char b)
{
    if(a>b) return a-b;
    else return b-a;
}

extern sAXIS GsensorDate[82];
unsigned char SitLongTimeProcess(void)
{
    static unsigned char Count =0;
    static signed char DataOld[3] = {0,0,0};
    static unsigned char Tpy =1;
    Count++;
    if(Count % 2)
    {       
        signed char data[3] = {GsensorDate[0].x,GsensorDate[0].y,GsensorDate[0].z};
        if(Tpy)
        {
            Tpy = 0;
            DataOld[0] = data[0] ;
            DataOld[1] = data[1] ;
            DataOld[2] = data[2] ;
        }
        
        
        SitLongTimeSum +=  UseAbs(DataOld[0],data[0]);
        SitLongTimeSum +=  UseAbs(DataOld[1],data[1]);
        SitLongTimeSum +=  UseAbs(DataOld[2],data[2]);
        
        DataOld[0] = data[0] ;
        DataOld[1] = data[1] ;
        DataOld[2] = data[2] ;
        
        SitLongTimeCount ++;
        //DBG1_EVT4("SitSitLongTime %d  SitLongTimeCount *3 %d  SitLongTimeCount*10/3 %d SitLongTimeSum %d",SitSitLongTime,SitLongTimeCount*3,SitLongTimeCount*10/3,SitLongTimeSum);
        
        if((SitLongTimeCount > SitSitLongTime)&&(SitSitLongTime != 0))
        {//时间到了 
            if(SitLongTimeSum  > (SitLongTimeCount*3))
            {
                if(app_long_sit_evt_handler_t !=0)
                {
                    app_long_sit_evt_t temp = LONGSIT_EVT_TRIGGERED;
                    app_long_sit_evt_handler_t(temp);
                }
            }
            SitLongTimeSum = 0;
            SitLongTimeCount = 0;
        }
    }
    return NRF_SUCCESS;
}



