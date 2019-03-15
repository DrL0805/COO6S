

#include "sdk_common.h"
#include "utc_dox_config.h"
#if NRF_MODULE_ENABLED(UTC)
#include "utc.h"

#include "app_timer.h"

//#include "m_gsensor.h"
//#include "m_charge.h"

#include "includes.h"

#if CONFIG_RTT_UTC_ENABLED
#define UTC_Print(M, ...)    SEGGER_Print("UTC", M, ##__VA_ARGS__)
#else
#define UTC_Print(M, ...)
#endif


#define BEGIN_UTC           \
{                           \
    .week   = 4,            \
    .second = 0,            \
    .minute = 0,            \
    .hour   = 0,            \
    .day    = 1,            \
    .month  = 1,            \
    .year   = BEGIN_YEAR,   \
}

#define SECONDS_PER_MIN             60U                     // seconds per minute
#define SECONDS_PER_HOUR            (SECONDS_PER_MIN * 60)  // seconds per hour
#define SECONDS_PER_DAY             (SECONDS_PER_HOUR * 24) // seconds per day 


APP_TIMER_DEF(utc_timer_id);

static utc_evt_handler_t    m_handler   = NULL;
static uint32_t             m_second    = 0;
static utc_time_t           m_utc       = BEGIN_UTC;
static bool m_utc_is_run = false;

__STATIC_INLINE bool is_leap_year(uint16_t year)
{
    return (!(year % 400) || ((year % 100) && !(year % 4))) ? true : false;
}

__STATIC_INLINE uint16_t days_per_year(uint16_t year)  
{
    return (is_leap_year(year) ? 366 : 365);
}

__STATIC_INLINE uint8_t days_per_month(bool is_leap_pyear, uint8_t month)
{  
    uint8_t days = 31;  

    if (month == 1)
    {  
        days = (28 + is_leap_pyear);  
    }  
    else  
    {  
        if (month > 6)
        {
            month--;
        }  

        if (month & 1)
        {  
            days = 30;
        }  
    }

    return days;
}

uint8_t	second_to_week(uint32_t sec)
{
	uint8_t		weeks;
	sec                 += SECONDS_PER_DAY;
	weeks        = ((sec / (SECONDS_PER_DAY) - 1) + (BEGIN_WEEK)) % 7;
	return weeks;
}

utc_time_t second_to_utc(uint32_t sec)
{
    utc_time_t ret_utc  = {0};
    uint16_t day_cnt    = 0;
    uint32_t last_day = 0;

    sec                 += SECONDS_PER_DAY;
    last_day            = sec % SECONDS_PER_DAY;

    ret_utc.week        = ((sec / (SECONDS_PER_DAY) - 1) + (BEGIN_WEEK)) % 7;

    ret_utc.second      = last_day % SECONDS_PER_MIN;
    ret_utc.minute      = (last_day % SECONDS_PER_HOUR) / SECONDS_PER_MIN;
    ret_utc.hour        = last_day / SECONDS_PER_HOUR;
    day_cnt             = sec / SECONDS_PER_DAY;

    ret_utc.year        = BEGIN_YEAR;
    while (day_cnt > days_per_year(ret_utc.year))
    {
        day_cnt -= days_per_year(ret_utc.year);
        ret_utc.year++;
    }

    ret_utc.month = 0;
    while (day_cnt > days_per_month(is_leap_year(ret_utc.year), ret_utc.month))  
    {  
        day_cnt -= days_per_month(is_leap_year(ret_utc.year), ret_utc.month);  
        ret_utc.month++;  
    }  
    ret_utc.month++;  
    ret_utc.day = day_cnt;
    
    return ret_utc;
}

uint32_t utc_to_second(utc_time_t utc)
{
    uint32_t ret_seconds = 0;  

    ret_seconds = (((utc.hour * 60UL) + utc.minute) * 60UL) + utc.second;  

    uint16_t days = utc.day;
    int8_t month  = utc.month - 1;

    while (--month >= 0)  
    {  
        days += days_per_month(is_leap_year(utc.year), month);  
    }

    uint16_t year = utc.year;

    while (--year >= BEGIN_YEAR)  
    {  
        days += days_per_year(year);
    }

    ret_seconds += (days * SECONDS_PER_DAY); 

    return (ret_seconds >= SECONDS_PER_DAY) ? (ret_seconds - SECONDS_PER_DAY) : ret_seconds; 
}




static void utc_timer_handler(void * p_context)
{
    utc_evt_t evt;
//	uint32_t		rtc = NRF_RTC1->COUNTER;
	
//	return;


    m_second++;
    m_utc = second_to_utc(m_second);

    if ((!m_utc.hour) && (!m_utc.minute) && (!m_utc.second))
    {
        evt = UTC_EVT_DAY;
    }
    else if ((!m_utc.minute) && (!m_utc.second))
    {
        evt = UTC_EVT_HOUR;
    }
    else if (!m_utc.second)
    {
        evt = UTC_EVT_MINUTE;
		UTC_Print("%.4d-%.2d-%.2d\t\t%.2d:%.2d:%.2d\n\r", m_utc.year, m_utc.month, m_utc.day, m_utc.hour, m_utc.minute, m_utc.second);
    }
    else
    {
        evt = UTC_EVT_SECOND;
    }

    if (m_handler) // 注意，分钟事件产生时的那一秒的秒事件不会发送，时事件，日事件类�?
    {
        m_handler(evt);
    }
	
//	nrf_delay_ms(10);
	
//	UTC_Print("utc seconds: %d ms\n\r", ((NRF_RTC1->COUNTER - rtc) * 1000) >> 15);
}

utc_time_t utc_get_cur_time(void)
{
    return m_utc;
}

uint32_t utc_get_cur_seconds(void)
{
    return m_second;
}


void utc_set_cur_time(utc_time_t utc)
{
    m_utc = utc;
    m_second = utc_to_second(utc);
}

void utc_set_cur_seconds(uint32_t seconds)
{
    m_second = seconds;
    m_utc = second_to_utc(seconds);
}

uint32_t utc_init(utc_evt_handler_t handler)
{
    uint32_t err_code;

    err_code = app_timer_create(&utc_timer_id, APP_TIMER_MODE_REPEATED, utc_timer_handler);

    if (err_code == NRF_SUCCESS)
    {
        m_handler = handler ? handler : NULL;
        m_second  = utc_to_second(m_utc);
    }

    return err_code;
}

uint32_t utc_enabled(void)
{
	ret_code_t 	ret_code;
	if (true == m_utc_is_run)
		return NRF_SUCCESS;

	
    ret_code = app_timer_start(utc_timer_id, APP_TIMER_TICKS(1000), NULL);
	if (ret_code == NRF_SUCCESS)
		m_utc_is_run = true;
	
//	nrf_pwr_mgmt_timeout_stop();

	return ret_code;
}

uint32_t utc_disabled(void)
{
	ret_code_t	ret_code;

	if (false == m_utc_is_run)
		return NRF_SUCCESS;
	
    ret_code = app_timer_stop(utc_timer_id);
	if (ret_code == NRF_SUCCESS)
		m_utc_is_run = false;
	
//	pwr_mgmt_timer_start();

	return ret_code;
}

#endif // NRF_MODULE_ENABLED(UTC)

