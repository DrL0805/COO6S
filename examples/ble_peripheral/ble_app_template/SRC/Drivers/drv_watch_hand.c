
#include "sr3_config.h"
#include "sdk_common.h"

#include "app_util_platform.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_assert.h"
#include "nrfx_pwm.h"
#include "drv_watch_hand.h"
#include "app_debug.h"
#include "app_timer.h"

#if 1//CONFIG_WATCH_HAND_ENABLED

#include "includes.h"

#if 0//CONFIG_RTT_DRV_WATCH_HAND_ENABLED
#define WATCH_HAND_Print(M, ...)    SEGGER_Print("DRV_WATCH_HAND", M, ##__VA_ARGS__)
#else
#define WATCH_HAND_Print(M, ...)
#endif



#define UINT                                (8U)                // 频率为 NRF_PWM_CLK_125kHz，单位则为8us
#define USEC_TO_UNITS(TIME)                 ((TIME) / (UINT))   // us换算成seq value的值

#define IO_WATCH_HAND_1_LEFT    (10)
#define IO_WATCH_HAND_1_RIGHT   (9)

#define PWM_0_CONFIG_FORWARD                        \
{                                                   \
    .output_pins =                                  \
    {                                               \
        IO_WATCH_HAND_1_LEFT  | NRFX_PWM_PIN_INVERTED,         \
        IO_WATCH_HAND_1_RIGHT | NRFX_PWM_PIN_INVERTED,         \
        NRFX_PWM_PIN_NOT_USED,                   \
        NRFX_PWM_PIN_NOT_USED,                   \
    },                                              \
    .irq_priority = APP_IRQ_PRIORITY_HIGHEST,       \
    .base_clock   = NRF_PWM_CLK_125kHz,             \
    .count_mode   = NRF_PWM_MODE_UP_AND_DOWN,       \
    .top_value    = USEC_TO_UNITS(450),             \
    .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,        \
    .step_mode    = NRF_PWM_STEP_AUTO               \
}

#define PWM_0_CONFIG_BACKWARD                       \
{                                                   \
    .output_pins =                                  \
    {                                               \
        IO_WATCH_HAND_1_LEFT  | NRFX_PWM_PIN_INVERTED,         \
        IO_WATCH_HAND_1_RIGHT | NRFX_PWM_PIN_INVERTED,         \
        NRFX_PWM_PIN_NOT_USED,                   \
        NRFX_PWM_PIN_NOT_USED,                   \
    },                                              \
    .irq_priority = APP_IRQ_PRIORITY_HIGHEST,       \
    .base_clock   = NRF_PWM_CLK_125kHz,             \
    .count_mode   = NRF_PWM_MODE_UP,                \
    .top_value    = 0,                              \
    .load_mode    = NRF_PWM_LOAD_WAVE_FORM,         \
    .step_mode    = NRF_PWM_STEP_AUTO               \
}


#define PWM_1_CONFIG_FORWARD                        \
{                                                   \
    .output_pins =                                  \
    {                                               \
        CONFIG_IO_WATCH_HAND_1_LEFT  | NRFX_PWM_PIN_INVERTED,         \
        CONFIG_IO_WATCH_HAND_1_RIGHT | NRFX_PWM_PIN_INVERTED,         \
        NRFX_PWM_PIN_NOT_USED,                   \
        NRFX_PWM_PIN_NOT_USED,                   \
    },                                              \
    .irq_priority = APP_IRQ_PRIORITY_HIGHEST,       \
    .base_clock   = NRF_PWM_CLK_125kHz,             \
    .count_mode   = NRF_PWM_MODE_UP_AND_DOWN,       \
    .top_value    = USEC_TO_UNITS(450),             \
    .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,        \
    .step_mode    = NRF_PWM_STEP_AUTO               \
}

#define PWM_1_CONFIG_BACKWARD                       \
{                                                   \
    .output_pins =                                  \
    {                                               \
        CONFIG_IO_WATCH_HAND_1_LEFT  | NRFX_PWM_PIN_INVERTED,         \
        CONFIG_IO_WATCH_HAND_1_RIGHT | NRFX_PWM_PIN_INVERTED,         \
        NRFX_PWM_PIN_NOT_USED,                   \
        NRFX_PWM_PIN_NOT_USED,                   \
    },                                              \
    .irq_priority = APP_IRQ_PRIORITY_HIGHEST,       \
    .base_clock   = NRF_PWM_CLK_125kHz,             \
    .count_mode   = NRF_PWM_MODE_UP,                \
    .top_value    = 0,                              \
    .load_mode    = NRF_PWM_LOAD_WAVE_FORM,         \
    .step_mode    = NRF_PWM_STEP_AUTO               \
}

typedef struct
{
    bool            busy_flag;
    drv_wh_dir_t    dir;        //将要转的方向
    uint16_t        steps;
    drv_wh_dir_t    last_dir;   //上次转的方向
    uint8_t         m_pin_active; //当前活动管脚
}drv_wh_cb_t;

typedef struct
{
    
    uint8_t          cur_watch_hand_id;  // 当前行针id
    drv_wh_cb_t      drv_wh_cb[2];       // 行针马达 1  行针马达2
}m_drv_wh_cb_t;



APP_TIMER_DEF(drv_watch_hand_0_timer_id);
APP_TIMER_DEF(drv_watch_hand_1_timer_id);
APP_TIMER_DEF(drv_watch_hand_0_wait_timer_id);
APP_TIMER_DEF(drv_watch_hand_1_wait_timer_id);

static m_drv_wh_cb_t                m_cb;   
static drv_wh_event_handler_t       m_handler = NULL;
static nrf_drv_state_t              m_state = NRF_DRV_STATE_UNINITIALIZED;

static bool                         m_0_pin_active = true;
static bool                         m_1_pin_active = true;

void pwm_0_start(void);
void pwm_1_start(void);

//static nrf_drv_pwm_t                watch_hand_1_pwm   = NRF_DRV_PWM_INSTANCE(0); // 表盘1
//static nrf_drv_pwm_t                watch_hand_2_pwm   = NRF_DRV_PWM_INSTANCE(1); // 表盘2

static nrfx_pwm_t  watch_hand_pwm[2] = {NRFX_PWM_INSTANCE(0),NRFX_PWM_INSTANCE(1)};

#define HAND_NUM 300 // 占空比 = (450-HAND_NUM) / 450

/*
 * 正推配置，简单序列
*/
//static nrf_pwm_values_individual_t  forward_seq0_value = {USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),  USEC_TO_UNITS(0),USEC_TO_UNITS(0)};

static nrf_pwm_values_individual_t  forward_seq0_value[] = 
{
    {USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),  USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),  USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),  USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),  USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),  USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),  USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    
};


//static nrf_pwm_values_individual_t  forward_seq1_value = {USEC_TO_UNITS(0),  USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),USEC_TO_UNITS(0)};

static nrf_pwm_values_individual_t  forward_seq1_value[] = 
{
    {USEC_TO_UNITS(0),  USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(0),  USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(0),  USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(0),  USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(0),  USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
    {USEC_TO_UNITS(0),  USEC_TO_UNITS(HAND_NUM),USEC_TO_UNITS(0),USEC_TO_UNITS(0)},
};



static const nrf_pwm_sequence_t     forward_seq0 =
{
    .values.p_individual    = forward_seq0_value,
    .length                 = NRF_PWM_VALUES_LENGTH(forward_seq0_value),
    .repeats                = 0,
    .end_delay              = 0
};

static const nrf_pwm_sequence_t     forward_seq1 =
{
    .values.p_individual    = forward_seq1_value,
    .length                 = NRF_PWM_VALUES_LENGTH(forward_seq1_value),
    .repeats                = 0,
    .end_delay              = 0
};

#if 0
/*
 * 反推配置，复杂序列，第二个序列通过中断控制其重复 3 次
*/
static nrf_pwm_values_wave_form_t backward_seq0_values0[] = {
    {USEC_TO_UNITS(278),        0x8000|USEC_TO_UNITS(278),   USEC_TO_UNITS(0),   USEC_TO_UNITS(1303)},
    {USEC_TO_UNITS(1326),       0x8000|USEC_TO_UNITS(1326),  USEC_TO_UNITS(0),   USEC_TO_UNITS(3867)},
    {USEC_TO_UNITS(4020),       USEC_TO_UNITS(0),            USEC_TO_UNITS(0),   USEC_TO_UNITS(4020)},
};

static nrf_pwm_values_wave_form_t backward_seq1_values0[] = {
    {0x8000|USEC_TO_UNITS(492), USEC_TO_UNITS(0),            USEC_TO_UNITS(0),   USEC_TO_UNITS(968)},
};

static nrf_pwm_values_wave_form_t backward_seq0_values1[] = {
    {0x8000|USEC_TO_UNITS(278), USEC_TO_UNITS(278),          USEC_TO_UNITS(0),   USEC_TO_UNITS(1303)},
    {0x8000|USEC_TO_UNITS(1326),USEC_TO_UNITS(1326),         USEC_TO_UNITS(0),   USEC_TO_UNITS(3867)},
    {USEC_TO_UNITS(0),          USEC_TO_UNITS(4020),         USEC_TO_UNITS(0),   USEC_TO_UNITS(4020)},
};

static nrf_pwm_values_wave_form_t backward_seq1_values1[] = {
    {USEC_TO_UNITS(0),          0x8000|USEC_TO_UNITS(492),   USEC_TO_UNITS(0),   USEC_TO_UNITS(968)},
};

#else

#define OFFSET 	0
#define HAND_A	1943 + OFFSET
#define HAND_B	HAND_A + 3446 + OFFSET 
#define HAND_C	1695 + OFFSET 

/*
 * 反推配置，复杂序列，第二个序列通过中断控制其重复 7 次
*/

static nrf_pwm_values_wave_form_t backward_seq0_values0[] = {
    {USEC_TO_UNITS(HAND_A),        0x8000|USEC_TO_UNITS(HAND_A),  USEC_TO_UNITS(0),   USEC_TO_UNITS(HAND_B)},
    {USEC_TO_UNITS(HAND_C),        USEC_TO_UNITS(0),              USEC_TO_UNITS(0),   USEC_TO_UNITS(HAND_C)},
};


static nrf_pwm_values_wave_form_t backward_seq1_values0[] = {
    {0x8000|USEC_TO_UNITS(460),  USEC_TO_UNITS(0),            USEC_TO_UNITS(0),   USEC_TO_UNITS(916)},
};


static nrf_pwm_values_wave_form_t backward_seq0_values1[] = {
    {0x8000|USEC_TO_UNITS(HAND_A), USEC_TO_UNITS(HAND_A),         USEC_TO_UNITS(0),   USEC_TO_UNITS(HAND_B)},
    {USEC_TO_UNITS(0),             USEC_TO_UNITS(HAND_C),         USEC_TO_UNITS(0),   USEC_TO_UNITS(HAND_C)},
};


static nrf_pwm_values_wave_form_t backward_seq1_values1[] = {
    {USEC_TO_UNITS(0),           0x8000|USEC_TO_UNITS(460),   USEC_TO_UNITS(0),   USEC_TO_UNITS(916)},
};


/*

static nrf_pwm_values_wave_form_t backward_seq0_values0[] = {
    {USEC_TO_UNITS(1942),        0x8000|USEC_TO_UNITS(1942),  USEC_TO_UNITS(0),   USEC_TO_UNITS(5387)},
    {USEC_TO_UNITS(1694),        USEC_TO_UNITS(0),            USEC_TO_UNITS(0),   USEC_TO_UNITS(1694)},
};

static nrf_pwm_values_wave_form_t backward_seq1_values0[] = {
    {0x8000|USEC_TO_UNITS(460),  USEC_TO_UNITS(0),            USEC_TO_UNITS(0),   USEC_TO_UNITS(916)},
};

static nrf_pwm_values_wave_form_t backward_seq0_values1[] = {
    {0x8000|USEC_TO_UNITS(1942), USEC_TO_UNITS(1942),         USEC_TO_UNITS(0),   USEC_TO_UNITS(5387)},
    {USEC_TO_UNITS(0),           USEC_TO_UNITS(1694),         USEC_TO_UNITS(0),   USEC_TO_UNITS(1694)},
};

static nrf_pwm_values_wave_form_t backward_seq1_values1[] = {
    {USEC_TO_UNITS(0),           0x8000|USEC_TO_UNITS(460),   USEC_TO_UNITS(0),   USEC_TO_UNITS(916)},
};

*/

#endif

static const nrf_pwm_sequence_t backward_seq00 =
{
    .values.p_wave_form = backward_seq0_values0,
    .length          = NRF_PWM_VALUES_LENGTH(backward_seq0_values0),
    .repeats         = 0,
    .end_delay       = 0
};


static const nrf_pwm_sequence_t backward_seq10 =
{
    .values.p_wave_form = backward_seq1_values0,
    .length          = NRF_PWM_VALUES_LENGTH(backward_seq1_values0),
    .repeats         = 0,
    .end_delay       = 3
};

static const nrf_pwm_sequence_t backward_seq01 =
{
    .values.p_wave_form = backward_seq0_values1,
    .length          = NRF_PWM_VALUES_LENGTH(backward_seq0_values1),
    .repeats         = 0,
    .end_delay       = 0
};

static const nrf_pwm_sequence_t backward_seq11 =
{
    .values.p_wave_form = backward_seq1_values1,
    .length          = NRF_PWM_VALUES_LENGTH(backward_seq1_values1),
    .repeats         = 0,
    .end_delay       = 3
};



nrfx_pwm_config_t  const forward_config[2] = {PWM_0_CONFIG_FORWARD ,PWM_1_CONFIG_FORWARD };
nrfx_pwm_config_t  const backward_config[2]= {PWM_0_CONFIG_BACKWARD,PWM_1_CONFIG_BACKWARD};



__STATIC_INLINE void pwm_0_forward_handler(nrfx_pwm_evt_type_t event)
{
    ASSERT(m_cb.drv_wh_cb[0].dir == DRV_WH_DIR_FORWARD);
    WATCH_HAND_Print("Function: %s event : %d.\r\n", (uint32_t)__func__,event);


    if (event == NRFX_PWM_EVT_STOPPED)
    {
        if (m_handler)
        {
            m_handler(0,DRV_WH_EVT_STEP_FORWARD);
        }
        WATCH_HAND_Print("m_cb.drv_wh_cb[0].steps: %d .\r\n", m_cb.drv_wh_cb[0].steps);
        if (m_cb.drv_wh_cb[0].steps)
        {
            m_cb.drv_wh_cb[0].steps--;
        }
        else
        {
            WATCH_HAND_Print(" m_cb.drv_wh_cb[0].busy_flag = false; \r\n\r\n\r\n");
            app_timer_stop(drv_watch_hand_0_timer_id);
//            m_cb.drv_wh_cb[0].busy_flag = false;
//            if (m_handler)
//            {
//                m_handler(0,DRV_WH_EVT_DONE);
//            }
//            
            app_timer_start(drv_watch_hand_0_wait_timer_id, APP_TIMER_TICKS(DRV_WH_FORWARD_STEP_WAIT_INTV), NULL);                   
        }
    }    
}

__STATIC_INLINE void pwm_1_forward_handler(nrfx_pwm_evt_type_t event)
{
    ASSERT(m_cb.drv_wh_cb[1].dir == DRV_WH_DIR_FORWARD);
//    WATCH_HAND_Print("Function: %s event : %d.\r\n", (uint32_t)__func__,event);

    if (event == NRFX_PWM_EVT_STOPPED)
    {
        if (m_handler)
        {
            m_handler(1,DRV_WH_EVT_STEP_FORWARD);
        }
        WATCH_HAND_Print("m_cb.drv_wh_cb[1].steps: %d .\r\n", m_cb.drv_wh_cb[1].steps);
        if (m_cb.drv_wh_cb[1].steps)
        {
            m_cb.drv_wh_cb[1].steps--;
        }
        else
        {
            WATCH_HAND_Print(" m_cb.drv_wh_cb[1].busy_flag = false; \r\n\r\n\r\n");
            app_timer_stop(drv_watch_hand_1_timer_id);
            m_cb.drv_wh_cb[1].busy_flag = false;
            if (m_handler)
            {
                m_handler(1,DRV_WH_EVT_DONE);
            }
            
//                    app_timer_start(drv_watch_hand_1_wait_timer_id, APP_TIMER_TICKS(DRV_WH_FORWARD_STEP_WAIT_INTV, 0), NULL);                   
        }
    }
           
    
}


__STATIC_INLINE void pwm_0_backward_handler(nrfx_pwm_evt_type_t event)
{
//    ASSERT(m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].dir == DRV_WH_DIR_BACKWARD);
    WATCH_HAND_Print("Function: %s event : %d.\r\n", (uint32_t)__func__,event);
    static bool seq1_started_flag = false;
    static uint8_t period_end_cnt = 0;
    
    if ((event == NRF_DRV_PWM_EVT_STARTED_SEQ1))
    {
        period_end_cnt = 0;
        seq1_started_flag = true;
    }
    
    if ((NRF_DRV_PWM_EVT_PERIOD_END == event) && (seq1_started_flag))
    {
        period_end_cnt++;
        if (period_end_cnt >= 3)
        {
            period_end_cnt = 0;
            seq1_started_flag = false;
            nrfx_pwm_stop(&watch_hand_pwm[0], false);
        }
    }
    
    if (event == NRFX_PWM_EVT_STOPPED)
    {
        period_end_cnt = 0;
        seq1_started_flag = false;

        if (m_handler)
        {
            m_handler(0,DRV_WH_EVT_STEP_BACKWARD);
        }
        WATCH_HAND_Print("m_cb.drv_wh_cb[0].steps: %d .\r\n", m_cb.drv_wh_cb[0].steps);
        if (m_cb.drv_wh_cb[0].steps)
        {
            m_cb.drv_wh_cb[0].steps--;
        }
        else
        {
            app_timer_stop(drv_watch_hand_0_timer_id);
            m_cb.drv_wh_cb[0].busy_flag = false;
            if (m_handler)
            {
                m_handler(0,DRV_WH_EVT_DONE);
            }
        }
    }
}


__STATIC_INLINE void pwm_1_backward_handler(nrfx_pwm_evt_type_t event)
{
//    ASSERT(m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].dir == DRV_WH_DIR_BACKWARD);
    WATCH_HAND_Print("Function: %s event : %d.\r\n", (uint32_t)__func__,event);
    static bool seq1_started_flag = false;
    static uint8_t period_end_cnt = 0;
    
    if ((event == NRF_DRV_PWM_EVT_STARTED_SEQ1))
    {
        period_end_cnt = 0;
        seq1_started_flag = true;
    }
    
    if ((NRF_DRV_PWM_EVT_PERIOD_END == event) && (seq1_started_flag))
    {
        period_end_cnt++;
        if (period_end_cnt >= 3)
        {
            period_end_cnt = 0;
            seq1_started_flag = false;
            nrfx_pwm_stop(&watch_hand_pwm[1], false);
        }
    }
    
    if (event == NRFX_PWM_EVT_STOPPED)
    {
        period_end_cnt = 0;
        seq1_started_flag = false;

        if (m_handler)
        {
            m_handler(1,DRV_WH_EVT_STEP_BACKWARD);
        }
        WATCH_HAND_Print("m_cb.drv_wh_cb[1].steps: %d .\r\n", m_cb.drv_wh_cb[1].steps);
        if (m_cb.drv_wh_cb[1].steps)
        {
            m_cb.drv_wh_cb[1].steps--;
        }
        else
        {
            app_timer_stop(drv_watch_hand_1_timer_id);
            m_cb.drv_wh_cb[1].busy_flag = false;
            if (m_handler)
            {
                m_handler(1,DRV_WH_EVT_DONE);
            }
        }
    }
}
static void pwm_0_handler(nrfx_pwm_evt_type_t event)
{
   WATCH_HAND_Print("Function: %s event : %d.\r\n", (uint32_t)__func__,event);
   
    if (m_cb.drv_wh_cb[0].dir == DRV_WH_DIR_FORWARD)
    {
//        if(m_cb.drv_wh_cb[0].last_dir != m_cb.drv_wh_cb[0].dir)
//        {
//             nrfx_pwm_uninit(&watch_hand_pwm[0]);
//             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[0], &forward_config[0], pwm_0_handler));
//             m_cb.drv_wh_cb[0].last_dir = m_cb.drv_wh_cb[0].dir;
//        }
        pwm_0_forward_handler(event);
    }
    else
    {
//        if(m_cb.drv_wh_cb[0].last_dir != m_cb.drv_wh_cb[0].dir)
//        {
//             nrfx_pwm_uninit(&watch_hand_pwm[0]);
//             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[0], &backward_config[0], pwm_0_handler));
//             m_cb.drv_wh_cb[0].last_dir = m_cb.drv_wh_cb[0].dir;
//        }
        pwm_0_backward_handler(event);
    }
}


static void pwm_1_handler(nrfx_pwm_evt_type_t event)
{

    if (m_cb.drv_wh_cb[1].dir == DRV_WH_DIR_FORWARD)
    {
//        if(m_cb.drv_wh_cb[1].last_dir != m_cb.drv_wh_cb[1].dir)
//        {
//             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[1], &forward_config[1], pwm_1_handler));
//             m_cb.drv_wh_cb[1].last_dir = m_cb.drv_wh_cb[1].dir;
//        }
        pwm_1_forward_handler(event);
    }
    else
    {
//        if(m_cb.drv_wh_cb[1].last_dir != m_cb.drv_wh_cb[1].dir)
//        {
//             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[1], &backward_config[1], pwm_1_handler));
//             m_cb.drv_wh_cb[1].last_dir = m_cb.drv_wh_cb[1].dir;
//        }
        pwm_1_backward_handler(event);
    }
}
/**@brief Application time-out handler type. */
static void drv_wh_0_timer_timeout_handler(void * p_context)
{
    WATCH_HAND_Print("drv_wh_0_timer_timeout_handler");
    ASSERT(m_cb.drv_wh_cb[0].dir != DRV_WH_DIR_IDLE);

    pwm_0_start();

    #if 0
    if (m_handler)
    {
        if (m_cb.dir == DRV_WH_DIR_FORWARD)
        {
            m_handler(DRV_WH_EVT_STEP_FORWARD);
        }
        else if (m_cb.dir == DRV_WH_DIR_BACKWARD)
        {
            m_handler(DRV_WH_EVT_STEP_BACKWARD);
        }
    }
    #endif
}
/**@brief Application time-out handler type. */
static void drv_wh_1_timer_timeout_handler(void * p_context)
{
    WATCH_HAND_Print("drv_wh_1_timer_timeout_handler");
    ASSERT(m_cb.drv_wh_cb[1].dir != DRV_WH_DIR_IDLE);

    pwm_1_start();
    #if 0
    if (m_handler)
    {
        if (m_cb.dir == DRV_WH_DIR_FORWARD)
        {
            m_handler(DRV_WH_EVT_STEP_FORWARD);
        }
        else if (m_cb.dir == DRV_WH_DIR_BACKWARD)
        {
            m_handler(DRV_WH_EVT_STEP_BACKWARD);
        }
    }
    #endif
}
/**@brief Application time-out handler type. */
static void drv_wh_0_wait_timer_timeout_handler(void * p_context)
{

    m_cb.drv_wh_cb[0].busy_flag = false;
    if (m_handler)
    {
        m_handler(0,DRV_WH_EVT_DONE);
    }
}

/**@brief Application time-out handler type. */
static void drv_wh_1_wait_timer_timeout_handler(void * p_context)
{

    m_cb.drv_wh_cb[1].busy_flag = false;
    if (m_handler)
    {
        m_handler(1,DRV_WH_EVT_DONE);
    }
}


static uint32_t pwm_init(void)
{
    uint32_t err_code;

    
    /*默认将两个表针初始化为正转*/
    err_code = nrfx_pwm_init(&watch_hand_pwm[0], &forward_config[0], pwm_0_handler);

    if (err_code == NRF_SUCCESS)
    {
        err_code = nrfx_pwm_init(&watch_hand_pwm[1], &forward_config[1], pwm_1_handler);
    }
    m_cb.drv_wh_cb[0].last_dir = DRV_WH_DIR_FORWARD;
    m_cb.drv_wh_cb[1].last_dir = DRV_WH_DIR_FORWARD;
    m_cb.drv_wh_cb[0].m_pin_active = true;
    m_cb.drv_wh_cb[1].m_pin_active = true;
    return err_code;
}

static void pwm_0_start(void)
{
      
    WATCH_HAND_Print("Function: %s, m_cb.cur_watch_hand_id: %d .\r\n", (uint32_t)__func__, m_cb.cur_watch_hand_id);
    
    WATCH_HAND_Print("m_pin_active: %d, m_cb.cur_watch_hand_id: %d .\r\n", m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].m_pin_active, m_cb.cur_watch_hand_id);
    
    
    if (m_cb.drv_wh_cb[0].dir == DRV_WH_DIR_FORWARD)
    {
        if(m_cb.drv_wh_cb[0].last_dir != m_cb.drv_wh_cb[0].dir)
        {
             nrfx_pwm_uninit(&watch_hand_pwm[0]);
             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[0], &forward_config[0], pwm_0_handler));
             m_cb.drv_wh_cb[0].last_dir = m_cb.drv_wh_cb[0].dir;
        }
    }
    else
    {
        if(m_cb.drv_wh_cb[0].last_dir != m_cb.drv_wh_cb[0].dir)
        {
             nrfx_pwm_uninit(&watch_hand_pwm[0]);
             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[0], &backward_config[0], pwm_0_handler));
             m_cb.drv_wh_cb[0].last_dir = m_cb.drv_wh_cb[0].dir;
        }
    }
    
    if (m_cb.drv_wh_cb[0].m_pin_active)
    {
        if (m_cb.drv_wh_cb[0].dir == DRV_WH_DIR_FORWARD)
        {
            WATCH_HAND_Print("DRV_WH_DIR_FORWARD: 000 .\r\n");
            nrfx_pwm_simple_playback(&watch_hand_pwm[0], &forward_seq0, 1, NRFX_PWM_FLAG_STOP);
        }
        else 
        {
            WATCH_HAND_Print("DRV_WH_DIR_BACKWARD: 000 .\r\n");
            nrfx_pwm_complex_playback(&watch_hand_pwm[0], &backward_seq00, &backward_seq10, 2, 
                                         NRF_DRV_PWM_FLAG_PERIOD_END | NRFX_PWM_FLAG_STOP | NRF_DRV_PWM_FLAG_STARTED_SEQ1);
        }
    }
    else
    {
        if (m_cb.drv_wh_cb[0].dir == DRV_WH_DIR_FORWARD)
        {
            WATCH_HAND_Print("DRV_WH_DIR_FORWARD: 111 .\r\n");
            nrfx_pwm_simple_playback(&watch_hand_pwm[0], &forward_seq1, 1, NRFX_PWM_FLAG_STOP);
        }
        else 
        {
            WATCH_HAND_Print("DRV_WH_DIR_BACKWARD: 111 .\r\n");
            nrfx_pwm_complex_playback(&watch_hand_pwm[0], &backward_seq01, &backward_seq11, 2,
                                         NRF_DRV_PWM_FLAG_PERIOD_END | NRFX_PWM_FLAG_STOP | NRF_DRV_PWM_FLAG_STARTED_SEQ1);
        }
    }
    
    m_cb.drv_wh_cb[0].m_pin_active = !m_cb.drv_wh_cb[0].m_pin_active;

}

static void pwm_1_start(void)
{
      
    WATCH_HAND_Print("Function: %s, m_cb.cur_watch_hand_id: %d .\r\n", (uint32_t)__func__, m_cb.cur_watch_hand_id);
    
//    WATCH_HAND_Print("m_pin_active: %d, m_cb.cur_watch_hand_id: %d .\r\n", m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].m_pin_active, m_cb.cur_watch_hand_id);
    
    
    if (m_cb.drv_wh_cb[1].dir == DRV_WH_DIR_FORWARD)
    {
        if(m_cb.drv_wh_cb[1].last_dir != m_cb.drv_wh_cb[1].dir)
        {
             nrfx_pwm_uninit(&watch_hand_pwm[1]);
             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[1], &forward_config[1], pwm_1_handler));
             m_cb.drv_wh_cb[1].last_dir = m_cb.drv_wh_cb[1].dir;
        }
    }
    else
    {
        if(m_cb.drv_wh_cb[1].last_dir != m_cb.drv_wh_cb[1].dir)
        {
             nrfx_pwm_uninit(&watch_hand_pwm[1]);
             APP_ERROR_CHECK(nrfx_pwm_init(&watch_hand_pwm[1], &backward_config[1], pwm_1_handler));
             m_cb.drv_wh_cb[1].last_dir = m_cb.drv_wh_cb[1].dir;
        }
    }
    
    if (m_cb.drv_wh_cb[1].m_pin_active)
    {
        if (m_cb.drv_wh_cb[1].dir == DRV_WH_DIR_FORWARD)
        {
//            DBG1_EVT0("DRV_WH_DIR_FORWARD: 000 .\r\n");
            nrfx_pwm_simple_playback(&watch_hand_pwm[1], &forward_seq0, 1, NRFX_PWM_FLAG_STOP);
        }
        else 
        {
            nrfx_pwm_complex_playback(&watch_hand_pwm[1], &backward_seq00, &backward_seq10, 2, 
                                         NRF_DRV_PWM_FLAG_PERIOD_END | NRFX_PWM_FLAG_STOP | NRF_DRV_PWM_FLAG_STARTED_SEQ1);
        }
    }
    else
    {
        if (m_cb.drv_wh_cb[1].dir == DRV_WH_DIR_FORWARD)
        {
//            DBG1_EVT0("DRV_WH_DIR_FORWARD: 111 .\r\n");
            nrfx_pwm_simple_playback(&watch_hand_pwm[1], &forward_seq1, 1, NRFX_PWM_FLAG_STOP);
        }
        else 
        {
            nrfx_pwm_complex_playback(&watch_hand_pwm[1], &backward_seq01, &backward_seq11, 2,
                                         NRF_DRV_PWM_FLAG_PERIOD_END | NRFX_PWM_FLAG_STOP | NRF_DRV_PWM_FLAG_STARTED_SEQ1);
        }
    }
    
    m_cb.drv_wh_cb[1].m_pin_active = !m_cb.drv_wh_cb[1].m_pin_active;

}

uint32_t drv_wh_init(drv_wh_event_handler_t handler)
{
    ret_code_t err_code;
    
    if (m_state != NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (uint32_t)(err_code));
        return err_code;
    }
    
    err_code = pwm_init();

    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_create(&drv_watch_hand_0_timer_id, APP_TIMER_MODE_REPEATED, drv_wh_0_timer_timeout_handler);
    }
    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_create(&drv_watch_hand_1_timer_id, APP_TIMER_MODE_REPEATED, drv_wh_1_timer_timeout_handler);
    }
    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_create(&drv_watch_hand_0_wait_timer_id, APP_TIMER_MODE_SINGLE_SHOT, drv_wh_0_wait_timer_timeout_handler);
    }
    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_create(&drv_watch_hand_1_wait_timer_id, APP_TIMER_MODE_SINGLE_SHOT, drv_wh_1_wait_timer_timeout_handler);
    }

    if (err_code == NRF_SUCCESS)
    {
        m_cb.drv_wh_cb[0].busy_flag  = false;
        m_cb.drv_wh_cb[0].dir        = DRV_WH_DIR_IDLE;
        m_cb.drv_wh_cb[0].steps      = 0;
        
        m_cb.drv_wh_cb[1].busy_flag  = false;
        m_cb.drv_wh_cb[1].dir        = DRV_WH_DIR_IDLE;
        m_cb.drv_wh_cb[1].steps      = 0;
        
        
        m_handler       = handler ? handler : NULL;
        m_state         = NRF_DRV_STATE_INITIALIZED;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (uint32_t)(err_code));
    }

    return err_code;
}

uint32_t drv_wh_start(drv_wh_config_t config)
{
    WATCH_HAND_Print("Function: %s, watch_hand_id: %d steps: %d.\r\n", (uint32_t)__func__,config.watch_hand_id,config.steps);
    ret_code_t err_code;
    m_cb.cur_watch_hand_id = config.watch_hand_id;
    
    if (m_state == NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (uint32_t)(err_code));
        return err_code;
    }
    if (m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].busy_flag == true)
    {
        err_code = NRF_ERROR_BUSY;
        WATCH_HAND_Print("Function: %s, error code: %d. hand_id : %d.\r\n", (uint32_t)__func__, (uint32_t)(err_code),m_cb.cur_watch_hand_id);
        return err_code;
    }
    
   
    if ((config.steps == 0) || ((config.direction != (uint8_t)DRV_WH_DIR_FORWARD) && (config.direction != (uint8_t)DRV_WH_DIR_BACKWARD)))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (uint32_t)(err_code));
        return err_code;
    }

    m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].busy_flag  = true;
    m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].dir        = (drv_wh_dir_t)config.direction;
    m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].steps      = config.steps;
    

    
    

    if (m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].dir == DRV_WH_DIR_FORWARD)
    {
        if(m_cb.cur_watch_hand_id == 0)
        {
            err_code = app_timer_start(drv_watch_hand_0_timer_id, APP_TIMER_TICKS(DRV_WH_FORWARD_STEP_INTV), NULL);
        }
        else if(m_cb.cur_watch_hand_id == 1)
        {
            err_code = app_timer_start(drv_watch_hand_1_timer_id, APP_TIMER_TICKS(DRV_WH_FORWARD_STEP_INTV), NULL);
        }
    }
    else
    {
        WATCH_HAND_Print("m_cb.cur_watch_hand_id: %d, DRV_WH_DIR_BACKWARD.\r\n", m_cb.cur_watch_hand_id);

        if(m_cb.cur_watch_hand_id == 0)
        {
            err_code = app_timer_start(drv_watch_hand_0_timer_id, APP_TIMER_TICKS(DRV_WH_BACKWARD_STEP_INTV), NULL);
        }
        else if(m_cb.cur_watch_hand_id == 1)
        {
            err_code = app_timer_start(drv_watch_hand_1_timer_id, APP_TIMER_TICKS(DRV_WH_BACKWARD_STEP_INTV), NULL);
        }
    }

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if(config.watch_hand_id == 0)
    {
        pwm_0_start();
    }
    else if(config.watch_hand_id == 1)
    {
        pwm_1_start();
    }

    m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].steps--;

    if (m_handler != NULL)
    {
        m_handler(m_cb.cur_watch_hand_id,DRV_WH_EVT_STARTED);
    }

    return NRF_SUCCESS;
}

#if 1
uint32_t drv_wh_stop(uint8_t watch_hand_id)
{
    uint32_t err_code;

    if (m_state == NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (uint32_t)(err_code));
        return err_code;
    }
    
    m_cb.cur_watch_hand_id = watch_hand_id;
    
    if(m_cb.cur_watch_hand_id == 0)
    {
        err_code = app_timer_stop(drv_watch_hand_0_timer_id);
    }
    else if(m_cb.cur_watch_hand_id == 1)
    {
        err_code = app_timer_stop(drv_watch_hand_1_timer_id);
    }

    if (err_code == NRF_SUCCESS)
    {
        if (m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].dir == DRV_WH_DIR_FORWARD)
        {
            nrfx_pwm_stop(&watch_hand_pwm[m_cb.cur_watch_hand_id], false);
        }
        else if (m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].dir == DRV_WH_DIR_BACKWARD)
        {
            nrfx_pwm_stop(&watch_hand_pwm[m_cb.cur_watch_hand_id], false);
        }
        WATCH_HAND_Print("m_cb.drv_wh_cb[%d].busy_flag  = false.\r\n",m_cb.cur_watch_hand_id);
        m_cb.drv_wh_cb[m_cb.cur_watch_hand_id].busy_flag  = false;
    }
    
    return err_code;
}
#endif

bool drv_wh_get_active_pin(uint8_t id)
{
    if(id == 0)
    {
        return m_0_pin_active;
    }
    else
    {
        return m_1_pin_active;
    }
    
}



void drv_wh_config_active_pin(bool watch_hand_0_flag,bool watch_hand_1_flag)
{
    m_0_pin_active = watch_hand_0_flag;
    m_1_pin_active = watch_hand_1_flag;
}


#endif // NRF_MODULE_ENABLED(WH)




