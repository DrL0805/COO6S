#include "AD_ShakeHandAlgorithm.h"
#include "m_database.h"
#include "m_drv_kx022.h"
//#include "app_debug_event_logger.h"
//#include "nrf_log.h"
#include "idle_judge.h"

#include "includes.h"

#if CONFIG_RTT_IDLE_JUDGE_ENABLED
#define JUDGE_Print(M, ...)	SEGGER_Print("JUDGE", M, ##__VA_ARGS__)
#else
#define JUDGE_Print(M, ...)
#endif

#define IDLE_JUDGE_TIME_INTV     36000  //seconds
#define THRESHOLD_VALUE          80


typedef void (*app_idle_judge_evt_handler_t)(app_idle_judge_evt_t evt);

app_idle_judge_evt_handler_t app_idle_judge_evt_handler = NULL;

static uint32_t idle_judge_time_count = 0;   //时间累加值



unsigned int app_idle_judge_init(app_idle_judge_evt_handler_t judge_evt_handler )
{
    app_idle_judge_evt_handler = judge_evt_handler;
    return NRF_SUCCESS;
}

unsigned char IdleJudgeCountClone(void) // 更新的时候调用
{
    idle_judge_time_count = 0;
    return NRF_SUCCESS;
}

static signed char UseAbs(signed char a,signed char b)
{
    if(a>b) return a-b;
    else return b-a;
}

extern sAXIS GsensorDate[82];
#define THRESHOLD           2  //2

unsigned char IdleJudgeProcess(void)//500ms
{
    static unsigned char Count =0;
    static signed char DataOld[3] = {0,0,0};
    static unsigned char Tpy =1;
    uint32_t one_second_sum = 0;
    static uint32_t last_sum = 0;
    
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
        
        one_second_sum +=  UseAbs(DataOld[0],data[0]) < THRESHOLD ? 0 : (UseAbs(DataOld[0],data[0]));
        one_second_sum +=  UseAbs(DataOld[1],data[1]) < THRESHOLD ? 0 : (UseAbs(DataOld[1],data[1]));
        one_second_sum +=  UseAbs(DataOld[2],data[2]) < THRESHOLD ? 0 : (UseAbs(DataOld[2],data[2]));
        
        DataOld[0] = data[0] ;
        DataOld[1] = data[1] ;
        DataOld[2] = data[2] ;
             
        switch(system_database.cur_idle_state)
        {
            case NOT_IDLE_ENTER://非空闲状态
            {
                idle_judge_time_count ++ ;  //时间累加
                if(one_second_sum < THRESHOLD_VALUE)
                {
                    if(idle_judge_time_count > IDLE_JUDGE_TIME_INTV) //静置/空闲状态 一小时
                    {
                        idle_judge_time_count = 0;
                        if(app_idle_judge_evt_handler !=0)
                        {
                                JUDGE_Print("IDLE_EVT_ENTER");
                                app_idle_judge_evt_handler(IDLE_EVT_ENTER);                               
                        }
                    }
                }
                else
                {
                    idle_judge_time_count = 0;
                }
            }
            break;
            
            case IDLE_ENTER://空闲状态
            {
                if(last_sum > THRESHOLD_VALUE)
                {
                    if(one_second_sum > (THRESHOLD_VALUE-20))  //连续两次的值大于一定范围
                    {
                        if(app_idle_judge_evt_handler !=0)
                        {
                            JUDGE_Print("IDLE_EVT_EXIT");
                            app_idle_judge_evt_handler(IDLE_EVT_EXIT);
                        }
                    }
                }                   
                last_sum = one_second_sum;
            }
            break;
            
            default:
                break;
        }           
    }
    return NRF_SUCCESS;
}



