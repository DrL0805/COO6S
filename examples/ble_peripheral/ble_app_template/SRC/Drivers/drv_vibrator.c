/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "sdk_common.h"

#include "sr3_config.h"
#include "drv_vibrator.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_assert.h"
#include "app_timer.h"

#if CONFIG_VIBRATOR_ENABLED



typedef enum
{
    DRV_VIBRATOR_OP_IDLE,
    DRV_VIBRATOR_OP_START,
    DRV_VIBRATOR_OP_ON,
    DRV_VIBRATOR_OP_OFF,
    DRV_VIBRATOR_OP_FLASH_COUNT,
    DRV_VIBRATOR_OP_INTERVAL,
    DRV_VIBRATOR_OP_COUNT,
    DRV_VIBRATOR_OP_END,
}drv_vibrator_op_t;

typedef struct
{
    uint16_t            on;
    uint16_t            off;
    uint16_t            flash_count;
    uint16_t            interval;
    uint16_t            count;
    drv_vibrator_op_t   operation;
}drv_vibrator_cb_t;

static drv_vibrator_cb_t            m_cb = {0};
static drv_vibrator_event_handler_t m_handler = NULL;
static nrf_drv_state_t              m_state = NRF_DRV_STATE_UNINITIALIZED;

APP_TIMER_DEF(m_drv_vibrator_timer_id);


__STATIC_INLINE void motor_on(void)
{
    if (CONFIG_IO_VIBRATOR_ACT_HIGH)
    {
        nrf_gpio_pin_set(CONFIG_IO_VIBRATOR);
    }
    else
    {
        nrf_gpio_pin_clear(CONFIG_IO_VIBRATOR);
    }
}

__STATIC_INLINE void motor_off(void)
{
    if (CONFIG_IO_VIBRATOR_ACT_HIGH)
    {
        nrf_gpio_pin_clear(CONFIG_IO_VIBRATOR);
    }
    else
    {
        nrf_gpio_pin_set(CONFIG_IO_VIBRATOR);
    }
}

__STATIC_INLINE void motor_init(void)
{
    nrf_gpio_cfg_output(CONFIG_IO_VIBRATOR);
    motor_off();
}

static void vbrt_timeout_handler(void * p_context)
{
    drv_vibrator_op_t cur_op = m_cb.operation;
    uint8_t volatile cyc_cnt = 50;
    static uint8_t flash_count = 0;

    while (cyc_cnt--)
    {
        if (cur_op == DRV_VIBRATOR_OP_START)
        {
            flash_count = m_cb.flash_count;
            cur_op = DRV_VIBRATOR_OP_ON;
        }

        if (cur_op == DRV_VIBRATOR_OP_ON)
        {
            if (m_cb.on > 0)
            {
                motor_on();
                app_timer_start(m_drv_vibrator_timer_id, APP_TIMER_TICKS(m_cb.on), NULL);
                m_cb.operation = DRV_VIBRATOR_OP_OFF;
                break;
            }
            else
            {
                cur_op = DRV_VIBRATOR_OP_OFF;
                continue;
            }
        }

        if (cur_op == DRV_VIBRATOR_OP_OFF)
        {
            motor_off();
            if (m_cb.off > 0)
            {
                app_timer_start(m_drv_vibrator_timer_id, APP_TIMER_TICKS(m_cb.off), NULL);
                m_cb.operation = DRV_VIBRATOR_OP_FLASH_COUNT;
                break;
            }
            else
            {
                cur_op = DRV_VIBRATOR_OP_FLASH_COUNT;
                continue;
            }
        }

        if (cur_op == DRV_VIBRATOR_OP_FLASH_COUNT)
        {
            if (flash_count > 1)
            {
                flash_count--;
                cur_op = DRV_VIBRATOR_OP_ON;
            }
            else
            {
                cur_op = DRV_VIBRATOR_OP_INTERVAL;
            }
            continue;
        }

        if (cur_op == DRV_VIBRATOR_OP_INTERVAL)
        {
            if (m_cb.interval > 0)
            {
                motor_off();
                app_timer_start(m_drv_vibrator_timer_id, APP_TIMER_TICKS(m_cb.interval), NULL);
                m_cb.operation = DRV_VIBRATOR_OP_COUNT;
                break;
            }
            else
            {
                cur_op = DRV_VIBRATOR_OP_COUNT;
                continue;
            }
        }

        if (cur_op == DRV_VIBRATOR_OP_COUNT)
        {
            if (m_cb.count > 1)
            {
                m_cb.count--;
                flash_count = m_cb.flash_count;
                cur_op = DRV_VIBRATOR_OP_ON;
            }
            else
            {
                cur_op = DRV_VIBRATOR_OP_END;
            }
            continue;
        }

        if (cur_op == DRV_VIBRATOR_OP_END)
        {
            if (m_handler != NULL)
            {
                m_handler(DRV_VIBRATOR_EVT_DONE, 0);
            }
            cur_op = DRV_VIBRATOR_OP_IDLE;
            break;
        }
    }
}

uint32_t drv_vibrator_init(drv_vibrator_event_handler_t handler)
{
    ret_code_t err_code;
    
    if (m_state != NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        return err_code;
    }

    err_code = app_timer_create(&m_drv_vibrator_timer_id, APP_TIMER_MODE_SINGLE_SHOT, vbrt_timeout_handler);

    if (err_code == NRF_SUCCESS)
    {
        m_handler = handler ? handler : NULL;
        m_state = NRF_DRV_STATE_INITIALIZED;
        motor_init();
    }

    return err_code;
}

uint32_t drv_vibrator_uninit(void)
{
    uint32_t err_code;

    if (m_state == NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        return err_code;
    }

    err_code = app_timer_stop(m_drv_vibrator_timer_id);

    if (err_code == NRF_SUCCESS)
    {
        memset((uint8_t *)&m_cb, 0, sizeof(m_cb));
        m_state = NRF_DRV_STATE_UNINITIALIZED;
        m_handler = NULL;
    }

    return err_code;
}

uint32_t drv_vibrator_start(drv_vibrator_config_t p_config)
{   
    ret_code_t err_code;
    if (m_state != NRF_DRV_STATE_INITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        return err_code;
    }

    if ( (p_config.flash_count == 0) || 
         ((p_config.count + p_config.on + p_config.off + p_config.interval) == 0) )
    {
        err_code = NRF_ERROR_INVALID_PARAM;
        return err_code;
    }

    motor_off();
    
    m_cb.on             = p_config.on;
    m_cb.off            = p_config.off;
    m_cb.flash_count    = p_config.flash_count;
    m_cb.interval       = p_config.interval;
    m_cb.count          = p_config.count;
    m_cb.operation      = DRV_VIBRATOR_OP_START;

    if (m_handler != NULL)
    {
        uint16_t time = ((m_cb.on + m_cb.off) * m_cb.flash_count + m_cb.interval) * m_cb.count;
    
        m_handler(DRV_VIBRATOR_EVT_STARTED, time);
    }

    vbrt_timeout_handler(NULL);

    return NRF_SUCCESS;
}

uint32_t drv_vibrator_stop(void)
{
    uint32_t err_code;

    err_code = app_timer_stop(m_drv_vibrator_timer_id);

    if (err_code == NRF_SUCCESS)
    {
        motor_off();

        m_cb.on             = 0;
        m_cb.off            = 0;
        m_cb.flash_count    = 0;
        m_cb.interval       = 0;
        m_cb.count          = 0;
        m_cb.operation      = DRV_VIBRATOR_OP_IDLE;
        m_state = NRF_DRV_STATE_INITIALIZED;

        err_code = NRF_SUCCESS;
    }

    return err_code;
}


#endif // NRF_MODULE_ENABLED(VIBRATOR)


