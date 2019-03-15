
#include "sr3_config.h"
#include "sdk_common.h"

#include "app_util_platform.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_assert.h"
#include "nrfx_pwm.h"
#include "drv_watch_hand_0.h"
#include "app_debug.h"
#include "app_timer.h"



#include "includes.h"

#if CONFIG_RTT_DRV_WATCH_HAND_ENABLED
#define WATCH_HAND_Print(M, ...)    SEGGER_Print("DRV_WATCH_HAND", M, ##__VA_ARGS__)
#else
#define WATCH_HAND_Print(M, ...)
#endif


#define CONFIG_IO_WATCH_HAND_0_LEFT    (10)
#define CONFIG_IO_WATCH_HAND_0_RIGHT   (9)

#define UINT                                (8U)                // 频率为 NRF_PWM_CLK_125kHz，单位则为8us
#define USEC_TO_UNITS(TIME)                 ((TIME) / (UINT))   // us换算成seq value的值

#define PWM_CONFIG_FORWARD                          \
{                                                   \
    .output_pins =                                  \
    {                                               \
        CONFIG_IO_WATCH_HAND_0_LEFT  | NRFX_PWM_PIN_INVERTED,         \
        CONFIG_IO_WATCH_HAND_0_RIGHT | NRFX_PWM_PIN_INVERTED,         \
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

#define PWM_CONFIG_BACKWARD                         \
{                                                   \
    .output_pins =                                  \
    {                                               \
        CONFIG_IO_WATCH_HAND_0_LEFT  | NRFX_PWM_PIN_INVERTED,         \
        CONFIG_IO_WATCH_HAND_0_RIGHT | NRFX_PWM_PIN_INVERTED,         \
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
    drv_wh_dir_t    dir;
    uint16_t        steps;
}drv_wh_cb_t;

APP_TIMER_DEF(drv_watch_hand_timer_id);
APP_TIMER_DEF(drv_watch_hand_wait_timer_id);

static drv_wh_cb_t                  m_cb;
static drv_wh_event_handler_t       m_handler = NULL;
static nrf_drv_state_t              m_state = NRF_DRV_STATE_UNINITIALIZED;
static bool                         m_pin_active = true;



static nrfx_pwm_t                pwm0 = NRFX_PWM_INSTANCE(0); // 正推

#define HAND_NUM 350

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

#define OFFSET 	200
#define HAND_A	1943 + 320
#define HAND_B	HAND_A + 3446 + OFFSET +  1000
#define HAND_C	1695 + OFFSET + 100

/*
 * 反推配置，复杂序列，第二个序列通过中断控制其重复 7 次
*/
/*
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
*/



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

void pwm_start(void);

__STATIC_INLINE void pwm_forward_handler(nrfx_pwm_evt_type_t event)
{
    ASSERT(m_cb.dir == DRV_WH_DIR_FORWARD);

    if (event == NRFX_PWM_EVT_STOPPED)
    {
        if (m_handler)
        {
            m_handler(DRV_WH_EVT_STEP_FORWARD);
        }
        WATCH_HAND_Print("Function: %s, m_cb.steps: %d.\r\n", (uint32_t)__func__, m_cb.steps);
        if (m_cb.steps)
        {
            m_cb.steps--;
        }
        else
        {
            app_timer_stop(drv_watch_hand_timer_id);
            
            app_timer_start(drv_watch_hand_wait_timer_id, APP_TIMER_TICKS(DRV_WH_FORWARD_STEP_WAIT_INTV), NULL);
            
//            m_cb.busy_flag = false;
//            if (m_handler)
//            {
//                m_handler(DRV_WH_EVT_DONE);
//            }
            
        }
    }
}

__STATIC_INLINE void pwm_backward_handler(nrfx_pwm_evt_type_t event)
{
    ASSERT(m_cb.dir == DRV_WH_DIR_BACKWARD);

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
            nrfx_pwm_stop(&pwm0, false);
        }
    }
    
    if (event == NRFX_PWM_EVT_STOPPED)
    {
        period_end_cnt = 0;
        seq1_started_flag = false;

        if (m_handler)
        {
            m_handler(DRV_WH_EVT_STEP_BACKWARD);
        }

        if (m_cb.steps)
        {
            m_cb.steps--;
        }
        else
        {
            app_timer_stop(drv_watch_hand_timer_id);
            m_cb.busy_flag = false;
            if (m_handler)
            {
                m_handler(DRV_WH_EVT_DONE);
            }
        }
    }
}

static void pwm_handler(nrfx_pwm_evt_type_t event)
{
    if (m_cb.dir == DRV_WH_DIR_FORWARD)
    {
        pwm_forward_handler(event);
    }
    else
    {
        pwm_backward_handler(event);
    }
}

/**@brief Application time-out handler type. */
static void drv_wh_timer_timeout_handler(void * p_context)
{
//    DBG0_EVT0("drv_wh_timer_timeout_handler");
    ASSERT(m_cb.dir != DRV_WH_DIR_IDLE);

    pwm_start();
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
static void drv_wh_wait_timer_timeout_handler(void * p_context)
{

    ASSERT(m_cb.dir != DRV_WH_DIR_IDLE);
    m_cb.busy_flag = false;
    if (m_handler)
    {
        m_handler(DRV_WH_EVT_DONE);
    }
}


static uint32_t pwm_init(void)
{
    uint32_t err_code;

    nrfx_pwm_config_t const forward_config = PWM_CONFIG_FORWARD;
    nrfx_pwm_config_t const backward_config = PWM_CONFIG_BACKWARD;

    err_code = nrfx_pwm_init(&pwm0, &forward_config, pwm_handler);


    return err_code;
}

static void pwm_start(void)
{
    if (m_pin_active)
    {
        if (m_cb.dir == DRV_WH_DIR_FORWARD)
        {
            nrfx_pwm_simple_playback(&pwm0, &forward_seq0, 1, NRFX_PWM_FLAG_STOP);
        }
        else 
        {
            nrfx_pwm_complex_playback(&pwm0, &backward_seq00, &backward_seq10, 2, 
                                         NRF_DRV_PWM_FLAG_PERIOD_END | NRFX_PWM_FLAG_STOP | NRF_DRV_PWM_FLAG_STARTED_SEQ1);
        }
    }
    else
    {
        if (m_cb.dir == DRV_WH_DIR_FORWARD)
        {
            nrfx_pwm_simple_playback(&pwm0, &forward_seq1, 1, NRFX_PWM_FLAG_STOP);
        }
        else 
        {
            nrfx_pwm_complex_playback(&pwm0, &backward_seq01, &backward_seq11, 2,
                                         NRF_DRV_PWM_FLAG_PERIOD_END | NRFX_PWM_FLAG_STOP | NRF_DRV_PWM_FLAG_STARTED_SEQ1);
        }
    }
    
    m_pin_active = !m_pin_active;

}

uint32_t drv_wh_init(drv_wh_event_handler_t handler)
{
    ret_code_t err_code;
    
    if (m_state != NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__,err_code);
        return err_code;
    }

    err_code = pwm_init();

    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_create(&drv_watch_hand_timer_id, APP_TIMER_MODE_REPEATED, drv_wh_timer_timeout_handler);
    }
    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_create(&drv_watch_hand_wait_timer_id, APP_TIMER_MODE_SINGLE_SHOT, drv_wh_wait_timer_timeout_handler);
    }

    if (err_code == NRF_SUCCESS)
    {
        m_cb.busy_flag  = false;
        m_cb.dir        = DRV_WH_DIR_IDLE;
        m_cb.steps      = 0;
        m_handler       = handler ? handler : NULL;
        m_state         = NRF_DRV_STATE_INITIALIZED;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (err_code));
    }

    return err_code;
}

uint32_t drv_wh_start(drv_wh_config_t config)
{
    ret_code_t err_code;

    if (m_state == NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (err_code));
        return err_code;
    }

    if (m_cb.busy_flag == true)
    {
        err_code = NRF_ERROR_BUSY;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (err_code));
        return err_code;
    }

    if ((config.steps == 0) || ((config.direction != (uint8_t)DRV_WH_DIR_FORWARD) && (config.direction != (uint8_t)DRV_WH_DIR_BACKWARD)))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (err_code));
        return err_code;
    }

    m_cb.busy_flag  = true;
    m_cb.dir        = (drv_wh_dir_t)config.direction;
    m_cb.steps      = config.steps;

    if (m_cb.dir == DRV_WH_DIR_FORWARD)
    {
        err_code = app_timer_start(drv_watch_hand_timer_id, APP_TIMER_TICKS(DRV_WH_FORWARD_STEP_INTV), NULL);
    }
    else
    {
        err_code = app_timer_start(drv_watch_hand_timer_id, APP_TIMER_TICKS(DRV_WH_BACKWARD_STEP_INTV), NULL);
    }

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    pwm_start();

    m_cb.steps--;

    if (m_handler != NULL)
    {
        m_handler(DRV_WH_EVT_STARTED);
    }

    return NRF_SUCCESS;
}

#if 1
uint32_t drv_wh_stop(void)
{
    uint32_t err_code;

    if (m_state == NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        WATCH_HAND_Print("Function: %s, error code: %d.\r\n", (uint32_t)__func__, (err_code));
        return err_code;
    }

    err_code = app_timer_stop(drv_watch_hand_timer_id);

    if (err_code == NRF_SUCCESS)
    {
        if (m_cb.dir == DRV_WH_DIR_FORWARD)
        {
            nrfx_pwm_stop(&pwm0, false);
        }
        else if (m_cb.dir == DRV_WH_DIR_BACKWARD)
        {
            nrfx_pwm_stop(&pwm0, false);
        }

        m_cb.busy_flag  = false;
    }

    return err_code;
}
#endif

bool drv_wh_get_active_pin(void)
{
    return m_pin_active;
}

void drv_wh_config_active_pin(bool flag)
{
    m_pin_active = flag;
}



