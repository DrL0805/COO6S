
#include "sdk_common.h"
#include "sr3_config.h"


#include "m_drv_led.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_assert.h"

#include "app_timer.h"
#include "app_debug.h"
#include "nrf_log.h"



#define APP_TIMER_PRESCALER             0                                                       /**< Value of the RTC1 PRESCALER register. */
#define MAX_LEDS                        ((CONFIG_IO_LED_0 != 0xFFFFFFFF)  \
                                        + (CONFIG_IO_LED_1 != 0xFFFFFFFF) \
                                        + (CONFIG_IO_LED_2 != 0xFFFFFFFF) \
                                        + (CONFIG_IO_LED_3 != 0xFFFFFFFF))

static const uint8_t m_led_to_pin_map[MAX_LEDS] = {
#if (CONFIG_IO_LED_0 != 0xFFFFFFFF)
    (uint8_t)(CONFIG_IO_LED_0),
#endif
#if (CONFIG_IO_LED_1 != 0xFFFFFFFF)
    (uint8_t)(CONFIG_IO_LED_1),
#endif
#if (CONFIG_IO_LED_2 != 0xFFFFFFFF)
    (uint8_t)(CONFIG_IO_LED_2),
#endif
#if (CONFIG_IO_LED_3 != 0xFFFFFFFF)
    (uint8_t)(CONFIG_IO_LED_3),
#endif
};

typedef enum
{
    DRV_LED_OP_FIRST        = 0U,   
    DRV_LED_OP_IDLE         = DRV_LED_OP_FIRST,
    DRV_LED_OP_START,
    DRV_LED_OP_ON,
    DRV_LED_OP_OFF,
    DRV_LED_OP_COUNT,
}drv_led_op_t;

typedef struct
{
    drv_led_op_t    operation;
    uint16_t        on;
    uint16_t        off;
    uint16_t        count;
    uint16_t        cur_on_off;
}led_op_param_t;

typedef struct
{
    bool            timer_active_flag;
    led_op_param_t  led_opp[MAX_LEDS];
}drv_led_cb_t;

static drv_led_cb_t             m_cb;
static drv_led_event_handler_t  m_handler = NULL;
static drv_led_error_handler_t  m_error_handler = NULL;
static nrf_drv_state_t          m_state = NRF_DRV_STATE_UNINITIALIZED;

APP_TIMER_DEF(m_drv_led_timer_id);

// 指定相应的 LED 编号亮。编号从 0 开始
ret_code_t led_on(uint8_t led_num)
{
    uint8_t pin;

    if (led_num >= MAX_LEDS)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    pin = m_led_to_pin_map[led_num];

    if (!IS_IO_VALID(pin))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (CONFIG_LED_ACT_LOW_MASK & (1 << led_num))
    {
        nrf_gpio_pin_clear(pin);
    }
    else
    {
        nrf_gpio_pin_set(pin);
    }

    return NRF_SUCCESS;
}


// 指定相应的 LED 编号灭。编号从 0 开始
ret_code_t led_off(uint8_t led_num)
{
    uint8_t pin;

    if (led_num >= MAX_LEDS)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    pin = m_led_to_pin_map[led_num];

    if (!IS_IO_VALID(pin))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    if (CONFIG_LED_ACT_LOW_MASK & (1 << led_num))
    {
        nrf_gpio_pin_set(pin);
    }
    else
    {
        nrf_gpio_pin_clear(pin);
    }

    return NRF_SUCCESS;
}



static void led_operation_process(uint16_t led)
{
    uint16_t      count          = m_cb.led_opp[led].count;
    uint16_t      cur_on_off     = m_cb.led_opp[led].cur_on_off;
    drv_led_op_t  operation      = m_cb.led_opp[led].operation;
    bool volatile loop_flag      = true;
    bool volatile first_in_flag  = false;
    uint8_t loop_cnt = 0;

    while (loop_flag)
    {
        loop_cnt++;
        if (loop_cnt > 10) // 做循环次数最大值限制，防止死循环
        {
            if (m_error_handler)
            {
                m_error_handler(NRF_ERROR_INTERNAL);
            }
            return;
        }

        loop_flag = false;
        //NRF_LOG_INFO("operation: %d\r\n", operation);
        //NRF_LOG_INFO("count: %d\r\n", count);
        //NRF_LOG_INFO("cur_on_off: %d\r\n", cur_on_off);
        switch (operation)
        {
            case DRV_LED_OP_START:
                cur_on_off      = m_cb.led_opp[led].on;
                loop_flag       = true;
                first_in_flag   = true;
                operation       = DRV_LED_OP_ON;
            break;

            case DRV_LED_OP_ON:
                if (cur_on_off == 0)
                {
                    cur_on_off      = m_cb.led_opp[led].off;
                    loop_flag       = true;
                    first_in_flag   = true;
                    operation       = DRV_LED_OP_OFF;
                }
                else
                {
                    if (first_in_flag)
                    {
                        first_in_flag = false;
                        led_on(led);
                    }
                    cur_on_off--;
                }
            break;

            case DRV_LED_OP_OFF:
                if (cur_on_off == 0)
                {
                    loop_flag       = true;
                    first_in_flag   = true;
                    operation       = DRV_LED_OP_COUNT;
                }
                else
                {
                    if (first_in_flag)
                    {
                        first_in_flag = false;
                        led_off(led);
                    }
                    cur_on_off--;
                }
            break;

            case DRV_LED_OP_COUNT:
                loop_flag = true;
                first_in_flag = true;

                if (count > 1)
                {
                    count--;
                    cur_on_off = m_cb.led_opp[led].on;
                    operation  = DRV_LED_OP_ON;
                }
                else if (count == 1)
                {
                    operation  = DRV_LED_OP_IDLE;
                }
                else // count 为 0，表示不停的闪烁
                {
                    cur_on_off = m_cb.led_opp[led].on;
                    operation  = DRV_LED_OP_ON;
                }
            break;

            case DRV_LED_OP_IDLE:
                led_off(led);
            break;

            default:
                ASSERT(0);
            break;
        }
    }
    m_cb.led_opp[led].count = count;
    m_cb.led_opp[led].operation = operation;
    m_cb.led_opp[led].cur_on_off = cur_on_off;
}

static void led_timeout_handler(void * p_context)
{
    uint32_t err_code;
    bool     all_idle = true;
    uint16_t led = 0;

    for (led = 0; led < MAX_LEDS; led++)
    {
        if (m_cb.led_opp[led].operation != DRV_LED_OP_IDLE)
        {
            led_operation_process(led);
            all_idle = false;
        }
    }

    if (all_idle) // 全部 LED 的操作状态都是 IDLE，即，没有 LED 闪烁任务时，停止定时器
    {
        err_code = app_timer_stop(m_drv_led_timer_id);

        if (err_code == NRF_SUCCESS)
        {
            m_cb.timer_active_flag = false;
            if (NULL != m_handler)
            {
                m_handler(DRV_LED_EVT_DONE);
            }
        }
    }
    else
    {
        m_cb.timer_active_flag = true;
    }

    if (err_code != NRF_SUCCESS)
    {
        if (m_error_handler != NULL)
        {
            m_error_handler(err_code);
        }
    }
}

uint32_t drv_led_init(drv_led_event_handler_t handler, drv_led_error_handler_t error_handler)
{
    ret_code_t err_code;
    
    if (m_state != NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_WARNING("Function: %s, error code: %s.\r\n", (uint32_t)__func__, (uint32_t)ERR_TO_STR(err_code));
        return err_code;
    }

    err_code = app_timer_create(&m_drv_led_timer_id, APP_TIMER_MODE_REPEATED, led_timeout_handler);

    if (err_code == NRF_SUCCESS)
    {
        memset((uint8_t *)&m_cb, 0, sizeof(m_cb));
        m_error_handler = error_handler ? error_handler : NULL;
        m_handler = handler ? handler : NULL;
        m_state = NRF_DRV_STATE_INITIALIZED;
//        NRF_LOG_INFO("Function: %s, error code: %s.\r\n", (uint32_t)__func__, (uint32_t)ERR_TO_STR(err_code));
    }

    return err_code;
}

uint32_t drv_led_start(drv_led_config_t config)
{
    if (m_state == NRF_DRV_STATE_UNINITIALIZED)
    {
//        NRF_LOG_WARNING("Function: %s, error code: %s.\r\n", (uint32_t)__func__, (uint32_t)ERR_TO_STR(NRF_ERROR_INVALID_STATE));
        return NRF_ERROR_INVALID_STATE;
    }

    if (config.led >=  MAX_LEDS)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    uint16_t led = config.led;

    m_cb.led_opp[led].on        = config.on;
    m_cb.led_opp[led].off       = config.off;
    m_cb.led_opp[led].count     = config.count;

    if ((config.on != 0) && (config.off == 0) && (config.count == 0)) // 常亮
    {
        led_on(led);
        m_cb.led_opp[led].operation = DRV_LED_OP_IDLE;
    }
    else if ((config.on == 0) && (config.count == 0)) // 常灭
    {
        led_off(led);
        m_cb.led_opp[led].operation = DRV_LED_OP_IDLE;
    }
    else
    {
        led_off(led);
        m_cb.led_opp[led].operation = DRV_LED_OP_START;
        if (!m_cb.timer_active_flag)
        {
            uint32_t err_code = app_timer_start(m_drv_led_timer_id, APP_TIMER_TICKS(DRV_LED_TIMEOUT), NULL);
            if (err_code == NRF_SUCCESS)
            {
                m_cb.timer_active_flag = true;
            }
        }
    }

    if (m_handler != NULL)
    {
        m_handler(DRV_LED_EVT_STARTED);
    }

    return NRF_SUCCESS;
}

uint32_t drv_led_stop(void)
{
//    DBG1_EVT0(" drv_led_stop");
    uint32_t err_code;

    if (m_state == NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_WARNING("Function: %s, error code: %s.\r\n", (uint32_t)__func__, (uint32_t)ERR_TO_STR(err_code));
        return err_code;
    }

    err_code = app_timer_stop(m_drv_led_timer_id);

    if (err_code == NRF_SUCCESS)
    {
        for (uint16_t i = 0; i < MAX_LEDS; i++)
        {
            led_off(i);
        }
    
        memset((uint8_t *)&m_cb, 0, sizeof(m_cb));
    }

    return err_code;
}






