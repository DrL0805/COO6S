

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
#define DRV_BUTTON_Print(M, ...)
#endif

typedef enum 
{
	KEY_PRESS 	= 0,
	KEY_RELEASE = 1,
}key_state_t;


#define DRV_BTN_ACT_TIME_DETECTION          (30)                                    /* 按键消抖延时，单位为: ms                  */
#define DRV_BTN_ACT_TIME_SHORT              (50 - DRV_BTN_ACT_TIME_DETECTION)       /* 短按触发时间，单位为: ms                  */
#define DRV_BTN_ACT_TIME_LONG               (3000 - 50)                             /* 长按按触发时间，单位为: ms                */
#define DRV_BTN_ACT_TIME_6SLONG             (6000 - 3000)                           /* 6s长按触发时间，单位为: ms                */
#define DRV_BTN_ACT_TIME_12SLONG            (12000 - 6000)                          /* 12s长按触发时间，单位为: ms               */
#define DRV_BTN_ACT_TIME_DOUBLE             (200)                                   /* 双击触发窗口时间，单位为: ms              */
#define DRV_BTN_ACT_TIME_DOUBLE_RELEASE     (600)                                   /* 双击触发后未释放超时时间，单位为: ms      */
#define DRV_BTN_ACT_TIME_12SLONG_RELEASE    (3000)                                  /* 12s长按触发后未释放超时时间，单位为: ms   */

          

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
APP_TIMER_DEF(m_drv_btn_check_timer_id);


void button_event_handler(uint8_t pin_no, uint8_t button_action);

static void button_event_handler(uint8_t pin_no, uint8_t button_action);

static const app_button_cfg_t drv_btn_act_buttons[DRV_BTN_NUMS] =
{
#if (CONFIG_IO_BUTTON_0 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_0, 0, NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
#if (CONFIG_IO_BUTTON_1 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_1, 0, NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
#if (CONFIG_IO_BUTTON_2 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_2, 0, NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
#if (CONFIG_IO_BUTTON_3 != 0xFFFFFFFF)
    {CONFIG_IO_BUTTON_3, !(CONFIG_BUTTON_ACT_LOW_MASK & 0x04), NRF_GPIO_PIN_PULLUP, button_event_handler},
#endif
};


void btn_check_timer_start(void)
{
    APP_ERROR_CHECK(app_timer_start(m_drv_btn_check_timer_id, APP_TIMER_TICKS(50), NULL));
}

void btn_check_timer_stop(void)
{
    APP_ERROR_CHECK(app_timer_stop(m_drv_btn_check_timer_id));
}


#define BTN_3S_LONG_KEY_TIME     3000 / 50
#define BTN_6S_LONG_KEY_TIME     6000 / 50
#define BTN_12S_LONG_KEY_TIME   12000 / 50

static void btn_check_timeout_handler(void * p_context)
{
    static uint16_t debug_flag = 0;
    static drv_btn_act_evt_type_t act_evt = DRV_BTN_ACT_EVT_IDLE;
    static uint16_t times_cnt;
    if(m_cb.button < DRV_BTN_NUMS)
    {
//        DRV_BUTTON_Print("btn_check_timeout_handler %d",m_cb.button);
        uint8_t button_pin = drv_btn_act_buttons[m_cb.button].pin_no;
        /*按键释放*/
        if(KEY_RELEASE == nrf_drv_gpiote_in_is_set(button_pin))
        {
            if(times_cnt < BTN_3S_LONG_KEY_TIME)
            {
                 /*短按事件*/
                act_evt = DRV_BTN_ACT_EVT_SHORT_RELEASED;
                DRV_BUTTON_Print("DRV_BTN_ACT_EVT_SHORT_RELEASED %d",m_cb.button);
                debug_flag  = debug_flag  | (0x01 << 1);             
            }
            else if(times_cnt < BTN_6S_LONG_KEY_TIME)
            {
                /* 3s事件*/
                act_evt = DRV_BTN_ACT_EVT_LONG_RELEASED;
                DRV_BUTTON_Print("DRV_BTN_ACT_EVT_LONG_RELEASED %d",m_cb.button);
            }
            else if(times_cnt < BTN_12S_LONG_KEY_TIME)
            {
                /* 6s事件*/
                act_evt = DRV_BTN_ACT_EVT_6S_LONG_RELEASED;
                DRV_BUTTON_Print("DRV_BTN_ACT_EVT_6S_LONG_RELEASED %d",m_cb.button);
            }
            /*关闭定时*/
            btn_check_timer_stop(); 
            times_cnt = 0;           
        }
        /*按键按下*/
        else
        {
            if(times_cnt < BTN_3S_LONG_KEY_TIME)
            {
//                act_evt = DRV_BTN_ACT_EVT_SHORT;
//                DRV_BUTTON_Print("DRV_BTN_ACT_EVT_SHORT");
                debug_flag = 0;
            }
            else if(times_cnt == BTN_3S_LONG_KEY_TIME)
            {
                act_evt = DRV_BTN_ACT_EVT_LONG;
                DRV_BUTTON_Print("DRV_BTN_ACT_EVT_LONG ");
                
                debug_flag  = debug_flag  | (0x01 << 2);             

            }
            else if(times_cnt == BTN_6S_LONG_KEY_TIME)
            {
                act_evt = DRV_BTN_ACT_EVT_6S_LONG;
                DRV_BUTTON_Print("DRV_BTN_ACT_EVT_6S_LONG ");
                debug_flag  = debug_flag  | (0x01 << 3);

            }
            else if(times_cnt == BTN_12S_LONG_KEY_TIME)
            {
                /*12s事件*/
                act_evt = DRV_BTN_ACT_EVT_12S_LONG;
                btn_check_timer_stop(); 
                times_cnt = 0;
                DRV_BUTTON_Print("DRV_BTN_ACT_EVT_12S_LONG");
                debug_flag  = debug_flag  | (0x01 << 4);

            }
            times_cnt++;                
        }
        
        if(act_evt == DRV_BTN_ACT_EVT_12S_LONG)
        {
            uint8_t debug_info[2] = {0};
            debug_info[0] = (debug_flag >> 8) & 0xff;
            debug_info[1] = debug_flag & 0xff;
            ret_code_t    err = w25x80_erase_sector(0x0003000);        
            if (NRF_SUCCESS == err)
            {
                err = w25x80_write(0x0003000, (uint8_t *)&debug_info, 2);
            }
        }
        /*发送事件*/
        if ((act_evt != DRV_BTN_ACT_EVT_IDLE) && (m_handler != NULL))
        {
            m_handler(m_cb.button, act_evt);
            act_evt = DRV_BTN_ACT_EVT_IDLE;
        }
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
        if (button != m_cb.button)
        {
            m_cb.operation = DRV_BTN_ACT_OP_IDLE;
        }
        if(button_action != APP_BUTTON_RELEASE)
        {
            m_cb.button = button;
            m_cb.button_action = button_action;
            m_cb.timeout_operation = DRV_BTN_ACT_OP_IDLE;
//            btn_act_timeout_handler(&m_cb.timeout_operation);
            btn_check_timer_start();
        }
        
    }
}

uint32_t drv_btn_act_init(drv_btn_act_event_handler_t handler)
{
    ret_code_t err_code;

    m_enabled = false;

    err_code = app_timer_create(&m_drv_btn_check_timer_id, APP_TIMER_MODE_REPEATED, btn_check_timeout_handler);
    
//    if (err_code == NRF_SUCCESS)
//    {
//        err_code = app_timer_create(&m_drv_btn_act_timer_id, APP_TIMER_MODE_SINGLE_SHOT, btn_act_timeout_handler);
//    }

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


