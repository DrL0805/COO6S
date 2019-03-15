


#include "sdk_common.h"

#include "app_timer.h"
#include "app_debug.h"
#include "app_alarm_clock.h"

#include "m_database.h"

#include "includes.h"

#if CONFIG_RTT_APP_ALARM_CLOCK_ENABLED
#define CLOCK_Print(M, ...)	SEGGER_Print("CLOCK", M, ##__VA_ARGS__)
#else
#define CLOCK_Print(M, ...)
#endif


#define CUR_YEAR        (*(&system_database.utc.year))
#define CUR_HOUR        ((*m_trigger_time.p_hour) % 24)
#define CUR_MINUTE      ((*m_trigger_time.P_minute) % 60)
#define CUR_WEEK        ((*m_trigger_time.p_week) % 7)

static app_alarm_clock_trigger_time_t       m_trigger_time;
static app_alarm_clock_param_t             *m_clock_param_list;
static app_alarm_clock_evt_handler_t        m_evt_handler;
static bool                                 m_is_enabled = false;

uint32_t app_alarm_clock_init(app_alarm_clock_config_t * p_config, app_alarm_clock_evt_handler_t handler)
{
    if ((p_config->p_param_list == NULL) || (handler == NULL))
    {
        CLOCK_Print("alarm_clock: Init Fail");
        return NRF_ERROR_INVALID_PARAM;
    }

    m_clock_param_list = p_config->p_param_list;
    m_trigger_time     = p_config->trigger_time;
    m_evt_handler      = handler;

    ////DBG0_EVT0("alarm_clock: Init ok");

    app_alarm_clock_enable();

    return NRF_SUCCESS;
}

static void print_clock_param(app_alarm_clock_param_t * param)
{
    CLOCK_Print("alarm_clock: num: %d hour: %d minute: %d repeat: %d weeks 0x%02x", 
              param->num, param->hour, param->minute, param->week.repeat, param->week.weeks);
    CLOCK_Print("alarm_clock: Cur year: %d hour: %d minute: %d weeks: %d", 
              CUR_YEAR, CUR_HOUR, CUR_MINUTE, CUR_WEEK);
}

static bool check_if_triggered(app_alarm_clock_param_t * p_param)
{
    if ((p_param->valid.valid == 0) || (p_param->valid.enabled == 0))           // 检查有效性
    {
        return false;
    }

    print_clock_param(p_param);

    if ( !((0 < p_param->num) && (p_param->num <= APP_ALARM_CLOCK_COUNT_MAX)) ) // 检查序号是否在范围内
    {
        return false;
    }

    if (p_param->week.repeat == 1)
    {
        uint8_t weeks = p_param->week.weeks;
    
        if (((weeks >> CUR_WEEK) & 0x01) && (p_param->hour == CUR_HOUR) && (p_param->minute == CUR_MINUTE))
        {
            return true;
        }
    }
    else
    {
        if ((p_param->hour * 60 + p_param->minute) > (CUR_HOUR * 60 + CUR_MINUTE)) // 闹钟时间已过，第二天触发
        {
            p_param->valid.enabled = true;
            return false;
        }

        if ((p_param->hour == CUR_HOUR) && (p_param->minute == CUR_MINUTE)) // 触发后关闭
        {
            p_param->valid.enabled = false;
            return true;
        }
    }

    return false;
}

uint32_t app_alarm_clock_delete(uint8_t alarm_clock_num)
{
    if ( (alarm_clock_num == 0) || (alarm_clock_num > APP_ALARM_CLOCK_COUNT_MAX) )
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset((uint8_t *)&m_clock_param_list[alarm_clock_num-1], 0, sizeof(app_alarm_clock_param_t));
        
        return NRF_SUCCESS;
}

uint32_t app_alarm_clock_enable(void)
{
    m_is_enabled = true;
    
    return NRF_SUCCESS;
}

uint32_t app_alarm_clock_disable(void)
{
    m_is_enabled = false;
    
      return NRF_SUCCESS;
}

uint32_t app_alarm_clock_minute_process(void)
{
    if (!m_is_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    uint8_t i;

    for (i = 0; i < APP_ALARM_CLOCK_COUNT_MAX; i++)
    {
        app_alarm_clock_param_t * p_param = &m_clock_param_list[i];

        if (check_if_triggered(p_param))
        {
            app_alarm_clock_evt_t   evt;

            evt.evt_id                  = APP_ALARM_CLOCK_EVT_TRIGGERED;
            evt.param.alarm_clock_index = p_param->num;
            m_evt_handler(&evt);
        }
    }

    return NRF_SUCCESS;
}













































