

#include "sr3_config.h"

#if CONFIG_BUTTON_ENABLED

#include "nrf_drv_common.h"
#include "app_button.h"
#include "app_timer.h"
#include "m_drv_btn_action.h"
#include "app_debug.h"
#include "nrf_gpio.h"
#include "app_gpiote.h"
#include "nrf_delay.h"

#include "includes.h"

#if CONFIG_RTT_BTN_ACT_ENABLED
#define DRV_BUTTON_Print(M, ...)    SEGGER_Print("DRV_BUTTON", M, ##__VA_ARGS__)
#else
#define MOTOR_Print(M, ...)
#endif


#define DRV_BTN_ACT_TIME_DETECTION          (30)                                    /* 按键消抖延时，单位为: ms                  */
#define DRV_BTN_ACT_TIME_SHORT              (50 - DRV_BTN_ACT_TIME_DETECTION)       /* 短按触发时间，单位为: ms                  */
#define DRV_BTN_ACT_TIME_LONG               (3000 - 50)                             /* 长按按触发时间，单位为: ms                */
#define DRV_BTN_ACT_TIME_6SLONG             (6000 - 3000)                           /* 6s长按触发时间，单位为: ms                */
#define DRV_BTN_ACT_TIME_12SLONG            (12000 - 6000)                          /* 12s长按触发时间，单位为: ms               */
#define DRV_BTN_ACT_TIME_DOUBLE             (200)                                   /* 双击触发窗口时间，单位为: ms              */
#define DRV_BTN_ACT_TIME_DOUBLE_RELEASE     (600)                                   /* 双击触发后未释放超时时间，单位为: ms      */
#define DRV_BTN_ACT_TIME_12SLONG_RELEASE    (3000)                                  /* 12s长按触发后未释放超时时间，单位为: ms   */

#define JUMP_TO_OPERATION(OP)   \
do                              \
{                               \
    first_in_flag = true;       \
    operation = (OP);           \
    loop_flag = true;           \
}while(0)                               

typedef enum
{
    DRV_BTN_ACT_OP_FIRST           = 0U, // 不能让有效 OPERATION 为 0，以避免跟 NULL 混淆
    DRV_BTN_ACT_OP_IDLE,
    DRV_BTN_ACT_OP_START,
    DRV_BTN_ACT_OP_SHORT,
    DRV_BTN_ACT_OP_WAIT_DOUBLE,
    DRV_BTN_ACT_OP_DOUBLE,
    DRV_BTN_ACT_OP_LONG,
    DRV_BTN_ACT_OP_6SLONG,
    DRV_BTN_ACT_OP_12SLONG,
}drv_btn_act_op_t;

typedef struct
{
    drv_btn_act_op_t        operation;
    drv_btn_act_op_t        timeout_operation;
    uint16_t                button;
    uint16_t                button_action;
}drv_btn_act_cb_t;

static drv_btn_act_cb_t             m_cb;
static drv_btn_act_event_handler_t  m_handler = NULL;
static bool                         m_enabled = false;

APP_TIMER_DEF(m_drv_btn_act_timer_id);

void button_event_handler(uint8_t pin_no, uint8_t button_action);

static void button_event_handler(uint8_t pin_no, uint8_t button_action);

static const app_button_cfg_t drv_btn_act_buttons[DRV_BTN_NUMS] =
{
#if (CONFIG_IO_BUTTON_0 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_0, !(CONFIG_BUTTON_ACT_LOW_MASK & 0x01), NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
#if (CONFIG_IO_BUTTON_1 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_1, !(CONFIG_BUTTON_ACT_LOW_MASK & 0x02), NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
#if (CONFIG_IO_BUTTON_2 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_2, !(CONFIG_BUTTON_ACT_LOW_MASK & 0x03), NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
#if (CONFIG_IO_BUTTON_3 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_3, !(CONFIG_BUTTON_ACT_LOW_MASK & 0x04), NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
};

uint32_t btn_op_timer_start(drv_btn_act_op_t operation)
{
    uint32_t err_code;
    uint32_t time = 0;

    err_code = app_timer_stop(m_drv_btn_act_timer_id);

    if (err_code == NRF_SUCCESS)
    {
        switch (operation)
        {
            case DRV_BTN_ACT_OP_START:          time = DRV_BTN_ACT_TIME_SHORT;          break;
            case DRV_BTN_ACT_OP_SHORT:          time = DRV_BTN_ACT_TIME_LONG;           break;
            case DRV_BTN_ACT_OP_WAIT_DOUBLE:    time = DRV_BTN_ACT_TIME_DOUBLE;         break;
            case DRV_BTN_ACT_OP_LONG:           time = DRV_BTN_ACT_TIME_6SLONG;         break;
            case DRV_BTN_ACT_OP_6SLONG:         time = DRV_BTN_ACT_TIME_12SLONG;        break;
            case DRV_BTN_ACT_OP_DOUBLE:         time = DRV_BTN_ACT_TIME_DOUBLE_RELEASE; break;
            case DRV_BTN_ACT_OP_12SLONG:        time = DRV_BTN_ACT_TIME_12SLONG_RELEASE;break;
            default:
                ASSERT(false);
            break;
        }
        m_cb.timeout_operation = operation;
        err_code = app_timer_start(m_drv_btn_act_timer_id, APP_TIMER_TICKS(time), &m_cb.timeout_operation);
    }

    return err_code;
}

bool is_btn_op_timer_timeout(drv_btn_act_op_t *p_timeout_operation, drv_btn_act_op_t cur_operation)
{
    bool ret = false;

    if (*p_timeout_operation == DRV_BTN_ACT_OP_IDLE)
    {
        ret = false;
    }
    else if (*p_timeout_operation == cur_operation)
    {
        ret = true;
    }

    *p_timeout_operation = DRV_BTN_ACT_OP_IDLE;

    return ret;
}

static void btn_act_timeout_handler(void * p_context)
{
//    DBG0_EVT0("btn_act_timeout_handler");
    ASSERT(m_cb.button < DRV_BTN_NUMS);

    uint16_t                button = m_cb.button;
    uint16_t                action = m_cb.button_action;
    drv_btn_act_op_t        operation = m_cb.operation;

    bool                    loop_flag = true;
    static bool             first_in_flag = false;
    drv_btn_act_evt_type_t  act_evt = DRV_BTN_ACT_EVT_IDLE;

//    DRV_BUTTON_Print("time out operation %d", *((drv_btn_act_op_t*)p_context));

    while (loop_flag)
    {
        //DBG0_EVT1("operation %d", operation);
        loop_flag = false;
        switch (operation)
        {
            case DRV_BTN_ACT_OP_IDLE:
                if (first_in_flag)
                {
                    first_in_flag = false;
                    app_timer_stop(m_drv_btn_act_timer_id);
                }
                else if (action == APP_BUTTON_PUSH)
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_START);
                }
            break;

            case DRV_BTN_ACT_OP_START:
                if (first_in_flag)
                {
                    first_in_flag = false;
                    btn_op_timer_start(operation);
                }
                else if (action == APP_BUTTON_RELEASE)
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
                else if (is_btn_op_timer_timeout((drv_btn_act_op_t*)p_context, operation))
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_SHORT);
                }
            break;

            case DRV_BTN_ACT_OP_SHORT:
                if (first_in_flag)
                {
                    first_in_flag = false;
                    act_evt = DRV_BTN_ACT_EVT_SHORT;
                    btn_op_timer_start(operation);
                }
                else if (action == APP_BUTTON_RELEASE)
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_WAIT_DOUBLE);
                }
                else if (is_btn_op_timer_timeout((drv_btn_act_op_t*)p_context, operation))
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_LONG);
                }
            break;

            case DRV_BTN_ACT_OP_WAIT_DOUBLE:
                if (first_in_flag)
                {
                    first_in_flag = false;
                    btn_op_timer_start(operation);
                }
                else if (action == APP_BUTTON_PUSH)
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_DOUBLE);
                }
                else if (is_btn_op_timer_timeout((drv_btn_act_op_t*)p_context, operation))
                {
                    act_evt = DRV_BTN_ACT_EVT_SHORT_RELEASED;
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
            break;

            case DRV_BTN_ACT_OP_DOUBLE:
                if (first_in_flag)
                {
                    first_in_flag = false;
                    act_evt = DRV_BTN_ACT_EVT_DOUBLE;
                    /* 该定时器防止双击后一直按着按键不放，或者丢失 RELEASE 事件，导致出不去状态 */
                    btn_op_timer_start(operation);
                }
                else if (action == APP_BUTTON_RELEASE)
                {
                    act_evt = DRV_BTN_ACT_EVT_DOUBLE_RELEASED;
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
                /* 该定时器防止双击后一直按着按键不放，或者丢失 RELEASE 事件，导致出不去状态 */
                else if (is_btn_op_timer_timeout((drv_btn_act_op_t*)p_context, operation))
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
            break;

            case DRV_BTN_ACT_OP_LONG:
                if (first_in_flag)
                {
                    act_evt = DRV_BTN_ACT_EVT_LONG;
                    first_in_flag = false;
                    btn_op_timer_start(operation);
                }
                else if (action == APP_BUTTON_RELEASE)
                {
                    act_evt = DRV_BTN_ACT_EVT_LONG_RELEASED;
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
                else if (is_btn_op_timer_timeout((drv_btn_act_op_t*)p_context, operation))
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_6SLONG);
                }
            break;

            case DRV_BTN_ACT_OP_6SLONG:
                if (first_in_flag)
                {
                    first_in_flag = false;
                    act_evt = DRV_BTN_ACT_EVT_6S_LONG;
                    btn_op_timer_start(operation);
                }
                else if (action == APP_BUTTON_RELEASE)
                {
                    act_evt = DRV_BTN_ACT_EVT_6S_LONG_RELEASED;
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
                else if (is_btn_op_timer_timeout((drv_btn_act_op_t*)p_context, operation))
                {
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_12SLONG);
                }
            break;

            case DRV_BTN_ACT_OP_12SLONG:
                if (first_in_flag)
                {
                    first_in_flag = false;
                    act_evt = DRV_BTN_ACT_EVT_12S_LONG;
                    /* 该定时器防止10S长按后一直按着按键不放，或者丢失 RELEASE 事件，导致出不去状态 */
                    btn_op_timer_start(operation);
                }
                else if (action == APP_BUTTON_RELEASE)
                {
                    act_evt = DRV_BTN_ACT_EVT_12S_LONG_RELEASED;
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
                /* 该定时器防止10S长按后一直按着按键不放，或者丢失 RELEASE 事件，导致出不去状态 */
                else if (is_btn_op_timer_timeout((drv_btn_act_op_t*)p_context, operation))
                {
                    act_evt = DRV_BTN_ACT_EVT_12S_LONG_RELEASED;
                    JUMP_TO_OPERATION(DRV_BTN_ACT_OP_IDLE);
                }
            break;
        }
    }

    m_cb.operation = operation;

    if ((act_evt != DRV_BTN_ACT_EVT_IDLE) && (m_handler != NULL))
    {
        m_handler(button, act_evt);
    }
    
    
}

static uint16_t pin_to_button_idx(uint32_t pin_no)
{
    uint32_t i;
    uint32_t ret = DRV_BTN_NUM_INVALID;
    for(i = 0; i < DRV_BTN_NUMS; ++i)
    {
        if (drv_btn_act_buttons[i].pin_no == pin_no)
        {
            ret = i;
            break;
        }
    }
    return ret;
}

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    uint16_t button = pin_to_button_idx(pin_no);

    if (button < DRV_BTN_NUMS)
    {
//        DRV_BUTTON_Print("Button %d, action %d\r\n", button, button_action);
        if (button != m_cb.button)
        {
            m_cb.operation = DRV_BTN_ACT_OP_IDLE;
        }
        m_cb.button = button;
        m_cb.button_action = button_action;
        m_cb.timeout_operation = DRV_BTN_ACT_OP_IDLE;
        btn_act_timeout_handler(&m_cb.timeout_operation);
    }
}

uint32_t drv_btn_act_init(drv_btn_act_event_handler_t handler)
{
    ret_code_t err_code;

    m_enabled = false;

    err_code = app_timer_create(&m_drv_btn_act_timer_id, APP_TIMER_MODE_SINGLE_SHOT, btn_act_timeout_handler);

    if (err_code == NRF_SUCCESS)
    {
        err_code = app_button_init((app_button_cfg_t *)drv_btn_act_buttons, DRV_BTN_NUMS, DRV_BTN_ACT_TIME_DETECTION);
    }

    if (err_code == NRF_SUCCESS)
    {
        m_cb.button = DRV_BTN_NUM_INVALID;
        m_cb.button_action = APP_BUTTON_RELEASE;
        m_cb.operation = DRV_BTN_ACT_OP_IDLE;
        m_cb.timeout_operation = DRV_BTN_ACT_OP_IDLE;
        
        m_handler = handler ? handler : NULL;
    }

    return err_code;
}

uint32_t drv_btn_act_enable(void)
{
    if (m_enabled == true)
    {
        return NRF_SUCCESS;
    }

    m_enabled = true;
    
    return app_button_enable();
}

uint32_t drv_btn_act_disable(void)
{
    uint32_t err_code;

    if (m_enabled == false)
    {
        return NRF_SUCCESS;
    }

    m_enabled = false;

    err_code = app_button_disable();

    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_stop(m_drv_btn_act_timer_id);
    }

    if (err_code == NRF_SUCCESS)
    {
        m_cb.operation          = DRV_BTN_ACT_OP_IDLE;
        m_cb.timeout_operation  = DRV_BTN_ACT_OP_IDLE;
        m_cb.button             = DRV_BTN_NUM_INVALID;
        m_cb.button_action      = APP_BUTTON_RELEASE;
    }

    return err_code;
}

bool drv_btn_action_wakeup_prepare(bool wakeup)
{
    uint32_t status = NRF_SUCCESS;

    if (wakeup)
    {
        status = app_button_enable();
    }

    if (status == NRF_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool drv_btn_action_check_enter_normal_state_button(void)
{
    if (app_button_is_pushed(pin_to_button_idx(CONFIG_IO_BUTTON_WAKEUP)))
    {
        if (app_button_is_pushed(pin_to_button_idx(CONFIG_IO_BUTTON_WAKEUP)))
        {
            return true;
        }
    }

    return false;
}


#endif // #if CONFIG_BUTTON_ENABLED


