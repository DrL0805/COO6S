#include "AD_ShakeHandAlgorithm.h"
#include "m_database.h"
#include "m_drv_kx022.h"
//#include "app_debug_event_logger.h"

typedef enum
{
        LONGSIT_EVT_TRIGGERED,
}app_long_sit_evt_t;

unsigned int app_long_sit_stop(void);
unsigned int app_long_sit_start(unsigned short minutes);
unsigned int app_long_sit_init(void (*app_long_sit_evt_handler_t_x)(app_long_sit_evt_t evt));
unsigned char SitLongTimeCountClone(void); // 步数更新的时候调用
unsigned char SitLongTimeProcess(void);

