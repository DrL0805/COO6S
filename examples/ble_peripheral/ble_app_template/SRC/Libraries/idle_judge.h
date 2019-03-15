#include "AD_ShakeHandAlgorithm.h"
#include "m_database.h"
#include "m_drv_kx022.h"
//#include "app_debug_event_logger.h"

typedef enum
{
     IDLE_EVT_EXIT,
     IDLE_EVT_ENTER,
}app_idle_judge_evt_t;


unsigned int app_idle_judge_init(void (*app_idle_judge_evt_handler_t_x)(app_idle_judge_evt_t evt));
unsigned char IdleJudgeCountClone(void); // 步数更新的时候调用
unsigned char IdleJudgeProcess(void);

