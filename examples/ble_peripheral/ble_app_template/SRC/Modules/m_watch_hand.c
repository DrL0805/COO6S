
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


#include "event_bus.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "app_timer.h"

#include "app_timer.h"
#include "drv_watch_hand.h"
#include "m_database.h"
#include "resources.h"
#include "sr3_config.h"
#include "includes.h"

#if CONFIG_WATCH_HAND_ENABLED



#if CONFIG_RTT_WATCH_HAND_ENABLED
#define WATCH_HAND_Print(M, ...)    SEGGER_Print("WATCH_HAND", M, ##__VA_ARGS__)
#else
#define WATCH_HAND_Print(M, ...)
#endif


#define TEST_WATCH_HAND 0

#define HOUR_SYSTEM             12
#define WATCH_HAND_0_POS_MAX      (60) 
#define WATCH_HAND_1_POS_MAX      (HOUR_SYSTEM * 360) 
#define WATCH_HAND_0_POS          (system_database.utc_seconds % WATCH_HAND_0_POS_MAX)
#define WATCH_HAND_1_POS          ( (system_database.utc_seconds /10) % WATCH_HAND_1_POS_MAX)
enum ind_type_e
{
    IND_TYPE_ALARM_CLOCK,
    IND_TYPE_MSG,
    IND_TYPE_CALL,
    IND_TYPE_COMPASS,
    IND_TYPE_STEPS_GOAL,
    IND_TYPE_STEPS_GOAL_FINISHED,
};

typedef enum 
{
    ENTER_CHECK_MODE,
    SET_WATCH_HAND_PARAM,
    EXIT_CHECK_MODE,
}check_mode_state_e;

typedef struct
{
//    uint8_t     ind_enable;                 /* 指示使能 */
    uint8_t     ind_type;                   /* 指示类型 */
    uint32_t    ind_type_pos_time;          /* 指示类型指示停留时间，单位为ms */
    uint8_t     ind_type_pos;               /* 指示类型的位置, 0 ~ 59 */
    uint32_t    ind_data_pos_time;          /* 指示数据指示停留时间，单位为ms */
    uint8_t     ind_data_start_pos;         /* 指示数据的起始位置, 0 ~ 59 */
    uint8_t     ind_data_step_intv;         /* 指示数据位置的间隔 */
}ind_config_t;

#ifdef WH_IND_POS_ALARM_CLOCK
static const ind_config_t ind_config_alarm_clock      = {IND_TYPE_ALARM_CLOCK,  10000,   WH_IND_POS_ALARM_CLOCK,   0,      0,      0  };
#endif
#ifdef WH_IND_POS_MSG
static const ind_config_t ind_config_msg              = {IND_TYPE_MSG,          5000,   WH_IND_POS_MSG,             0,      0,      0  };
#endif
#ifdef WH_IND_POS_CALL
static const ind_config_t ind_config_callin           = {IND_TYPE_CALL,         5000,   WH_IND_POS_CALL,            0,      0,      0  };
#endif
#ifdef WH_IND_POS_STEPS_GOAL_FINISHED
static const ind_config_t ind_steps_goal_finished     ={IND_TYPE_STEPS_GOAL_FINISHED,5000,WH_IND_POS_STEPS_GOAL_FINISHED,0,0,0 };
#endif

#ifdef WH_IND_STEPS_POS_START
static  ind_config_t ind_config_sport_target_test     = {IND_TYPE_STEPS_GOAL,   5000,   WH_IND_STEPS_POS_START,     0 ,                  WH_IND_STEPS_POS_START,     0};
#endif



ind_config_t ind_config_compass_target     = {IND_TYPE_STEPS_GOAL,   1000,   0,     0 ,    0,     0};

static ind_config_t ind_config_compass     = {IND_TYPE_COMPASS,      2000,   0,     0 ,    0,     0};



APP_TIMER_DEF(m_watch_hand_timer_id);

static bool     m_is_ble_connected                  = false;

static bool     m_is_enabled                        = false;  // 模式已经使能
static bool     m_is_watch_hand_0_fast_going        = false;  // 正在追针
static bool     m_is_watch_hand_1_fast_going        = false;  // 正在追针
static bool     m_is_watch_hand_second_moving_active= false;  // 正在走秒
static bool     m_ind_wait_for_second_move_done     = false;  // 等待走秒结束
static bool     m_is_watch_hand_indicating          = false;  // 表针当前正处于指示信息的状态
static bool     m_is_watch_hand_indicating_stay     = false;  // 表针当前正处于指示信息停留状态
static bool     m_is_watch_hand_enter_check_mode    = false;  // 表针当前正处于校针模式
static bool     watch_hand_start_check_timer        = false;  // 表针开始校时
static bool     calibrate_seconds_echo_flag         = false;  // 校时是否需要通知app
static bool     m_photo_mode_enter                  = false;  // 拍照请求标志
static bool     watch_hand_compass_mode             = false;  // 表针当前正处于指北针模式

static bool     set_watch_hand_0_pos                = false;  // 校准小表盘


static bool     m_test_mode_enter                   = false;

static void watch_hand_indication(ind_config_t const * config);
static void calibrate_process(check_mode_state_e state);


static void error_hanlder(uint32_t err_code)
{
    WATCH_HAND_Print("err_code %d", err_code);
}

/*
static void send_mag_pos_data(uint16_t data,uint8_t pos)  //发送mag测量数据
{
      uint8_t cur_seconds_pos = ((uint8_t)(system_database.watch_hand_0_pos % 60));
      static uint8_t buf[4];
      buf[0] = (data >> 8) & 0xff;
      buf[1] =  data & 0xff;
      buf[2] =  DEC_TO_BCD(pos & 0xff);
      buf[2] =  DEC_TO_BCD(cur_seconds_pos & 0xff);
      protocol_send_data(buf,4);
}



static void send_cur_pos_data()  //发送mag测量数据
{
      uint8_t cur_seconds_pos = (uint8_t)(system_database.watch_hand_0_pos % 60);
      static uint8_t buf[2];
      buf[0] =  DEC_TO_BCD(WATCH_HAND_0_POS & 0xff);
      buf[1] =  DEC_TO_BCD(cur_seconds_pos & 0xff);
      protocol_send_data(buf,2);
}
*/


/* 表针驱动事件处理 */
static void watch_hand_event_handler(uint8_t watch_hand_id,drv_wh_evt_type_t wh_evt)
{
//    WATCH_HAND_Print("watch_hand_id %d ; wh_evt : %d", watch_hand_id,wh_evt);
    uint16_t volatile pos =0;
    switch(watch_hand_id)
    {
        case 0:
            pos = system_database.watch_hand_0_pos;
            if (DRV_WH_EVT_STEP_FORWARD == wh_evt)
            {
                pos++;
            }
            else if (DRV_WH_EVT_STEP_BACKWARD == wh_evt)
            {
                if (pos)
                {
                    pos--;
                }
                else
                {
                    pos = (WATCH_HAND_0_POS_MAX) - 1;
                }
                WATCH_HAND_Print("DRV_WH_EVT_STEP_BACKWARD; pos : %d",pos);
            }
            pos %= WATCH_HAND_0_POS_MAX;
            if (DRV_WH_EVT_DONE == wh_evt)
            {
//                WATCH_HAND_Print("m_is_watch_hand_0_fast_going = false; pos : %d",pos);
                m_is_watch_hand_0_fast_going = false;
                m_is_watch_hand_second_moving_active = false;
                if (m_ind_wait_for_second_move_done)
                {
                    m_ind_wait_for_second_move_done = false;
                    watch_hand_indication(NULL);
                }
            }
            system_database.watch_hand_0_active_pin = drv_wh_get_active_pin(0);
            system_database.watch_hand_0_pos = pos;
            break;
            
        case 1:
            pos = system_database.watch_hand_1_pos;
            if (DRV_WH_EVT_STEP_FORWARD == wh_evt)
            {
                pos++;
            }
            else if (DRV_WH_EVT_STEP_BACKWARD == wh_evt)
            {
                if (pos)
                {
                    pos--;
                }
                else
                {
                    pos = WATCH_HAND_1_POS_MAX - 1;
                }
            }
            pos %= WATCH_HAND_1_POS_MAX;
//            WATCH_HAND_Print("watch_hand_1_pos = %d",pos);
            if (DRV_WH_EVT_DONE == wh_evt)
            {
//                WATCH_HAND_Print("m_is_watch_hand_1_fast_going = false");
                m_is_watch_hand_1_fast_going = false;
                m_is_watch_hand_second_moving_active = false;
            }
            system_database.watch_hand_1_active_pin = drv_wh_get_active_pin(1);
            system_database.watch_hand_1_pos = pos;
            break;
            
        default:
            break;
    }
   
    
}

__STATIC_INLINE uint32_t watch_hand_step_forward(uint8_t watch_hand_id,uint32_t steps)
{
    drv_wh_config_t const wh_config = {watch_hand_id,steps, DRV_WH_DIR_FORWARD};

    return drv_wh_start(wh_config);
}

__STATIC_INLINE uint32_t watch_hand_step_backward(uint8_t watch_hand_id,uint32_t steps)
{
    drv_wh_config_t const wh_config = {watch_hand_id,steps, DRV_WH_DIR_BACKWARD};

    return drv_wh_start(wh_config);
}

//__STATIC_INLINE uint32_t watch_hand_step_stay(void)
//{
//    return drv_wh_stop();
//}

static void send_calibrate_expect_seconds(uint16_t seconds)  //发送校时预期用时
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_CALIBRATE_EXPECT_SECONDS;
    send_evt.header.evt_len = 2;
    send_evt.evt_params.interact.calibrate_expect_seconds.expect_seconds = __REV16(seconds);

    protocol_send_evt(send_evt);
}

static void echo_calibrate_time_state(calibrate_time_state_t  state)   //发送校时状态，开始校时/完成校时
{
    if (!m_is_ble_connected) return;

    protocol_evt_t send_evt;

    send_evt.header.evt_id = PROT_EVT_ID_ECHO_CALIBRATE_TIME_STATE;
    send_evt.header.evt_len = 1;
    send_evt.evt_params.interact.calibrate_time_state.calibrate_time_state = state;


    protocol_send_evt(send_evt);
}



/*
 * 计算追针时需要的额外的步数。
 * 计算方法为: 不断的累加追针时间所需的步数，直到追针所需的时间小于1s时，停止计算
 * steps: 追针步数
 * time:  追针每一步所需的时间，单位为 ms
*/
static uint32_t count_catch_added_steps(uint8_t id ,uint32_t steps, uint32_t step_time)
{
    uint32_t catch_up_time  = steps * step_time; // 追针时间
    uint32_t catchup_steps  = 0;
    uint32_t added_steps    = 0;
    if(id == 1)
    {
        catch_up_time = catch_up_time / 10;
    }
    while (catch_up_time >= 1000) // 追针所需时间小于 1s 时，计算完毕
    {
        catchup_steps = catch_up_time / 1000;
        added_steps  += catchup_steps;
        catch_up_time = (catch_up_time % 1000) + (catchup_steps * step_time);
    }

    return added_steps;
}

#ifdef WH_IND_STEPS_POS_END
/*  */
static uint8_t cur_steps_offset(void)
{

    uint16_t cur_steps = system_database.steps_today;
    uint16_t goal_steps = system_database.steps_goal_today;
    
//    DBG1_EVT2("cur_steps :%d goal_steps :%d",cur_steps,goal_steps);

    uint8_t  offset    = 0;



    if (goal_steps>0) 
    {
        offset = cur_steps * 100 / goal_steps / WH_IND_STEPS_INTV ;   //  WH_IND_STEPS_INTV 5
//        DBG1_EVT1("offset %d",offset);
        offset = (offset >  WH_IND_STEPS_INTV_NUM) ?  WH_IND_STEPS_INTV_NUM   : offset;
    }
    
    return offset;
}
#endif
static uint32_t get_data_pos_from_ind_type(ind_config_t * config, uint8_t * p_pos)
{
    switch (config->ind_type)
    {
        case IND_TYPE_STEPS_GOAL:
        {
            #ifdef WH_IND_STEPS_POS_END
            if(WH_IND_STEPS_POS_END > WH_IND_STEPS_POS_START)
            {
                *p_pos = config->ind_data_start_pos + cur_steps_offset();
            }
            else
            {
                *p_pos = config->ind_data_start_pos - cur_steps_offset();
            }
            #endif
            break;
        }

        default:
            break;
    }

    return NRF_SUCCESS;
}

static int count_steps(uint8_t watch_hand_id,uint32_t pos_from, uint32_t pos_to, uint32_t pos_max)
{
    int      steps = 0;
    uint32_t steps_forward  = 0; // 正转应该追针的步数
    uint32_t steps_backward = 0; // 反转应该追针的步数

    pos_from %= pos_max;
    pos_to   %= pos_max;

    
    if (pos_to > pos_from)
    {
        steps_forward  = pos_to - pos_from;
        steps_backward = pos_max - steps_forward;
    }
    else if (pos_to < pos_from)
    {
        steps_backward = pos_from - pos_to;
        steps_forward  = pos_max - steps_backward;
    }
    else
    {
        return steps;
    }
    

    
#if 1
    steps_forward  += count_catch_added_steps(watch_hand_id,steps_forward, DRV_WH_FORWARD_STEP_INTV);
    steps_backward -= count_catch_added_steps(watch_hand_id,steps_backward, DRV_WH_BACKWARD_STEP_INTV);
#endif


    
//    WATCH_HAND_Print("steps_forward :%d steps_backward :%d",steps_forward,steps_backward);
    if ((DRV_WH_FORWARD_STEP_INTV * steps_forward) <= (DRV_WH_BACKWARD_STEP_INTV * steps_backward))
    {
        steps = (int)steps_forward;
    }
    else
    {
        steps = -(int)steps_backward;
    }

    if ((steps > 1) || (steps < -1))
    {
        WATCH_HAND_Print("Watch hand from pos: %d to pos: %d", pos_from, pos_to);
        WATCH_HAND_Print("Watch hand steps: %d", steps);
    }

        /*如果是大表盘，没有反转*/
    if(watch_hand_id == 1)
    {
        if((steps < 0) && (steps > -6))
        {
            return  (-(int)steps_backward);
        }

        else
        {
             return  ((int)steps_forward);
        }
    }
    /*如果是小表盘，可能有反转*/
    else 
    {
        #if (!CONFIG_WATCH_HAND_ROLLBACK_ENABLED) 
        if((steps < 0) && (steps > -6))
        {
            return  (-(int)steps_backward);
        }
        #else
        if(steps < 0)
        {
            return  (-(int)steps_backward);
        }   
        #endif

        else
        {
             return  ((int)steps_forward);
        }
    }    
}

static uint32_t watch_hand_stop(void)
{
//    WATCH_HAND_Print("m_is_watch_hand_1_fast_going = false");
//    DBG0_EVT0("watch_hand_stop");
    m_is_watch_hand_0_fast_going          = false;
    m_is_watch_hand_1_fast_going          = false;
    m_is_watch_hand_indicating            = false;
    APP_ERROR_CHECK(drv_wh_stop(0));
    APP_ERROR_CHECK(drv_wh_stop(1));
    return NRF_SUCCESS;
}

/*小表盘*/
static void watch_hand_move_to_utcpos(void)
{
//    WATCH_HAND_Print("system_database.watch_hand_0_pos = %d  UTC_POS = %d",system_database.watch_hand_0_pos,WATCH_HAND_0_POS);
#if TEST_WATCH_HAND
    return;
#endif

    int steps = 0;

    
    //在追针中，直接退出
    if (m_is_watch_hand_0_fast_going)
    {       
        return;
    }
    
    

//    WATCH_HAND_Print("system_database.watch_hand_0_pos = %d  UTC_POS =%d",system_database.watch_hand_0_pos,WATCH_HAND_0_POS);
    steps = count_steps(0,system_database.watch_hand_0_pos, WATCH_HAND_0_POS, WATCH_HAND_0_POS_MAX);
//    WATCH_HAND_Print("watch_hand_move_to_utcpos steps = %d ",steps);

    if (steps != 0)
    {
        //正在追针
        m_is_watch_hand_0_fast_going = true;
//        WATCH_HAND_Print("m_is_watch_hand_0_fast_going = true;");
    }

    if (steps == 1)
    {   
        m_is_watch_hand_second_moving_active = true;
        m_is_watch_hand_indicating_stay = false;
    }
   
    if (steps > 0)
    {
        APP_ERROR_CHECK(watch_hand_step_forward(0,steps));
    }
    else if (steps < 0)
    {
        #if (!CONFIG_WATCH_HAND_ROLLBACK_ENABLED)   //没有反转
//        DBG0_EVT0("watch_hand_move_to_utcpos watch_hand_move_to_logicpos m_is_watch_hand_fast_going = false");
 
//        m_is_watch_hand_0_fast_going        = false;
//        m_is_watch_hand_indicating_stay     = true;
//        m_is_watch_hand_indicating          = false;
        
        steps += 60;
        if(steps > 0 )
        {
//            DBG1_EVT1("watch_hand_step_forward  :: steps :%d",steps);
            APP_ERROR_CHECK(watch_hand_step_forward(0,steps));
        }
        #else     
        APP_ERROR_CHECK(watch_hand_step_backward(0,-steps));
        #endif
    }
    

            
}

/*大表盘*/
static void watch_hand_1_move_to_utcpos(void)
{
//    WATCH_HAND_Print("system_database.watch_hand_1_pos = %d  UTC_POS = %d",system_database.watch_hand_1_pos,WATCH_HAND_1_POS);
#if TEST_WATCH_HAND
    return;
#endif
    
    int steps = 0;
    int expect_seconds = 0;
    
    if (m_is_watch_hand_1_fast_going)
    {
        
        return;
    }

//    WATCH_HAND_Print("system_database.watch_hand_1_pos = %d  UTC_POS = %d",system_database.watch_hand_1_pos,WATCH_HAND_1_POS);
    steps = count_steps(1,system_database.watch_hand_1_pos, WATCH_HAND_1_POS, WATCH_HAND_1_POS_MAX);
//    WATCH_HAND_Print("watch_hand_move_to_utcpos steps = %d ",steps);

    

    if (steps != 0)
    {
        //正在追针
        m_is_watch_hand_1_fast_going = true;
//        WATCH_HAND_Print("m_is_watch_hand_1_fast_going = true");
    }

    if (steps == 1)
    {
        if ((watch_hand_start_check_timer) &&(calibrate_seconds_echo_flag)) //校时状态且校时过程超过了三秒
        {
            WATCH_HAND_Print("complete  Sync  time ");
            watch_hand_start_check_timer = false;
            calibrate_seconds_echo_flag  = false;
            echo_calibrate_time_state(COMPLETE_CALIBRATE_TIME);
        }      
    }
   
    if (steps > 0)
    {
        if (watch_hand_start_check_timer) 
        {            
            expect_seconds = DRV_WH_FORWARD_STEP_INTV *steps / 1000;        
        }
//        DBG1_EVT1("utc--watch_hand_step_forward = steps %d" ,steps);
        APP_ERROR_CHECK(watch_hand_step_forward(1,steps));
    }
    else if (steps < 0)
    {
        m_is_watch_hand_1_fast_going        = false;
    }
  
    if((expect_seconds >= 3)&&(watch_hand_start_check_timer))  //处于校时过程，校时过程不足3秒的，无需发送
    {
        calibrate_seconds_echo_flag = true;
        WATCH_HAND_Print("start  Sync  time ");
        echo_calibrate_time_state(START_CALIBRATE_TIME);
        WATCH_HAND_Print("expect_seconds %d ",expect_seconds);
        send_calibrate_expect_seconds(expect_seconds);
    }
            
}

static void watch_hand_move_to_logicpos(uint8_t pos)
{
    WATCH_HAND_Print("watch_hand_move_to_logicpos :%d",pos);
#if TEST_WATCH_HAND
    return;
#endif
    int steps = 0;
    uint8_t cur_seconds_pos = (uint8_t)(system_database.watch_hand_0_pos % 60);
    
    if (m_is_watch_hand_0_fast_going)
    {
        return;
    }
    
    steps = count_steps(0,cur_seconds_pos, pos, 60);
//    DBG1_EVT1("watch_hand_move_to_logicpos  :: cur_seconds_pos :%d",cur_seconds_pos);
//    DBG1_EVT1("watch_hand_move_to_logicpos  :: steps :%d",steps);

    if (steps != 0)
    {
//        WATCH_HAND_Print("m_is_watch_hand_0_fast_going = true;");   
        m_is_watch_hand_0_fast_going = true;
    }

    if (steps > 0)
    {

        APP_ERROR_CHECK(watch_hand_step_forward(0,steps));
        
    }
    else if (steps < 0)
    {
        #if (!CONFIG_WATCH_HAND_ROLLBACK_ENABLED)   //没有反转
        steps += 60;
        if(steps > 0 )
        {
//            DBG1_EVT1("watch_hand_step_forward  :: steps :%d",steps);
            APP_ERROR_CHECK(watch_hand_step_forward(0,steps));
        }
        #else
         APP_ERROR_CHECK(watch_hand_step_backward(0,-steps));
        #endif
    }
}

static void watch_hand_timeout_handler(void * p_context)
{
    WATCH_HAND_Print("watch_hand_timeout_handler");
    ind_config_t * config = (ind_config_t *)p_context;

    /* 校针模式下，追针状态下，不继续进行指示 */
    if (m_is_watch_hand_enter_check_mode || m_is_watch_hand_0_fast_going) 
    {
        m_is_watch_hand_indicating = false;
        return;
    }

    if (config == NULL) // 没有需要继续指示的任务
    {
        m_is_watch_hand_indicating = false;
        return;
    }

    if (config->ind_data_pos_time != 0)
    {
        uint8_t  pos = 0;

        if (NRF_SUCCESS == get_data_pos_from_ind_type(config, &pos))
        {
            WATCH_HAND_Print("get_data_pos_from_ind_type :pos %d",pos);
            watch_hand_move_to_logicpos(pos);
            app_timer_start(m_watch_hand_timer_id, APP_TIMER_TICKS(config->ind_data_pos_time), NULL);
        }
        else
        {
            error_hanlder(NRF_ERROR_INVALID_PARAM);
        }
    }
    else
    {
        m_is_watch_hand_indicating = false;
    }
}

static void watch_hand_indication(ind_config_t const * config)
{
//    WATCH_HAND_Print("config->ind_type_pos_time %d",config->ind_type_pos_time);

    static ind_config_t const * pre_config = NULL;

    /* 走秒和指示事件同时(在同一个消息队列的紧邻的前后位置)到达时，需要等待走秒结束后进行指示，没有该判断条件会导致指示事件不能执行 走秒完成后调用
       watch_hand_indication，并传递一个NULL参数*/
    if (m_is_watch_hand_second_moving_active) 
    {

        m_ind_wait_for_second_move_done = true;
        /*保存要指示的参数，下次调用*/
        pre_config = config;
        return;
    }
    else
    {
        m_ind_wait_for_second_move_done = false;
    }

    /* 校针模式下，追针状态下，当前正处于指示流程时，当前处于指示后停留阶段，不进行新的指示 */
    if (m_is_watch_hand_enter_check_mode || m_is_watch_hand_0_fast_going || m_is_watch_hand_indicating  || m_is_watch_hand_indicating_stay) 
    {
        return;
    }

    m_is_watch_hand_indicating = true;
    
    
    if (config != NULL)
    {
        pre_config = config;
    }

    if (pre_config == NULL)
    {
        WATCH_HAND_Print("Indication error!!!!!!");
        m_is_watch_hand_indicating = false;
        m_ind_wait_for_second_move_done = false;
        return;
    }

    if (pre_config->ind_type_pos_time != 0)
    {
        watch_hand_move_to_logicpos(pre_config->ind_type_pos);
//        WATCH_HAND_Print("config->ind_type_pos_time %d",pre_config->ind_type_pos_time);
        app_timer_start(m_watch_hand_timer_id, APP_TIMER_TICKS(pre_config->ind_type_pos_time), (void *)pre_config);
    }
    else
    {
        watch_hand_timeout_handler((void *)pre_config);
    }
}

static void watch_hand_recv_msg_process(void)
{
    uint32_t msg_bitmap = 0;

    msg_bitmap = system_database.new_msg_bitmap & system_database.msg_switch_bitmap;

    if (  CHECK_VALID_BIT(msg_bitmap, MSG_BIT_CALLIN)
       || CHECK_VALID_BIT(msg_bitmap, MSG_BIT_CONTEXT_CALLIN))
    {
        #ifdef WH_IND_POS_CALL
        watch_hand_indication(&ind_config_callin);
        #endif
    }
    else
    {
        #ifdef WH_IND_POS_MSG
        watch_hand_indication(&ind_config_msg);
        #endif
    }
}

void mag_compass_pos_get(uint32_t angel,uint8_t *pos)
{
    *pos = (angel / 6) % 60;
}


/* 表针模块与事件总线的接口 */
bool m_watch_hand_event_handler(const event_t * p_event)
{

    if (p_event->type == EVT_SYSTEM_STATE)
    {
        if (p_event->system.data == SYSTEM_STATE_NORMAL)
        {
            m_is_enabled = true;
        }
    }

    if (!m_is_enabled)
    {
        return false;
    }
    
    switch(p_event->type)
    {
        case EVT_SYSTEM_BT_CONN_STATE:
            if (p_event->system.ble_conn_state < BT_CONN_STATE_CONNECTED)
            {
                m_is_ble_connected = false;
                m_is_watch_hand_enter_check_mode = false; // 断开时退出校针模式(不影响追针)
            }           
            else 
            {
                m_is_ble_connected = true;
            }

            break;
            
        /*指北动作*/
        case  EVT_SYSTEM_MAG_COMPASS_ANGLE:
            mag_compass_pos_get(p_event->system.data,&ind_config_compass_target.ind_type_pos);
            WATCH_HAND_Print("angel : %d  ,pos :%d ",p_event->system.data,ind_config_compass_target.ind_type_pos);
//            send_mag_pos_data(p_event->system.data,ind_config_compass_target.ind_type_pos);
            watch_hand_indication(&ind_config_compass_target);
            break;
        /*退出指北针模式*/
        case  EVT_SYSTEM_MAG_COMPASS_EXIT:
            watch_hand_compass_mode = false;
            break;
        /*进入指北针模式*/
        case EVT_SYSTEM_MAG_COMPASS_ENTER:
            watch_hand_compass_mode = true;
            break;

        case EVT_COMS_APP_RECV:
            if (APP_RECV_EVT_WATCH_HAND_ENTER_MODE == p_event->coms.app_recv_event)
            {
                WATCH_HAND_Print("ENTER_CHECK_MODE");
                APP_ERROR_CHECK(watch_hand_stop());
                m_is_watch_hand_enter_check_mode = true;
                calibrate_process(ENTER_CHECK_MODE);
                
            }
            else if (APP_RECV_EVT_WATCH_HAND_SET_PARAM == p_event->coms.app_recv_event)
            {
                WATCH_HAND_Print("SET_WATCH_HAND_PARAM");
                calibrate_process(SET_WATCH_HAND_PARAM);                
                
            }
            else if (APP_RECV_EVT_WATCH_HAND_EXIT_MODE == p_event->coms.app_recv_event)
            {
                WATCH_HAND_Print("EXIT_CHECK_MODE");
                m_is_watch_hand_enter_check_mode = false;
                calibrate_process(EXIT_CHECK_MODE);
                if(set_watch_hand_0_pos)
                {
                    set_watch_hand_0_pos = false ;
                    system_database.watch_hand_0_pos = 0;
                }
            }
            else if (APP_RECV_EVT_WATCH_HAND_CHECK == p_event->coms.app_recv_event)
            {
                set_watch_hand_0_pos = true ;
            }
            
            else if (APP_RECV_EVT_PHOTO_ENTER_MODE == p_event->coms.app_recv_event)
            {
                WATCH_HAND_Print("m_photo_mode_enter = true");
                m_photo_mode_enter = true;
            }
            else if (APP_RECV_EVT_PHOTO_EXIT_MODE == p_event->coms.app_recv_event)
            {
                WATCH_HAND_Print("m_photo_mode_enter = false");
                m_photo_mode_enter = false;
            }
                              
            else if (APP_RECV_EVT_MSG_NEW == p_event->coms.app_recv_event)
            {
                if(!m_photo_mode_enter)
                {
                    watch_hand_recv_msg_process();
                }
            }
            break;

        case EVT_COMS_PROT_EXEC:
            if (PROT_EXEC_EVT_ALARM_TRIGGERED == p_event->coms.prot_execution)
            {
                #ifdef WH_IND_POS_ALARM_CLOCK
                DBG1_EVT0("Watch hand: alarm clock triggered");
                DBG0_EVT1("ind_type_pos_time %d",ind_config_alarm_clock.ind_type_pos_time);
                watch_hand_indication(&ind_config_alarm_clock);
                #endif
            }
            break;
         case EVT_SYSTEM_STEPS_GOAL_FINISHED:
             #ifdef WH_IND_POS_STEPS_GOAL_FINISHED
             watch_hand_indication(&ind_steps_goal_finished);
             #endif
            break;
    
        case EVT_SYSTEM_UTC_SECOND:
//            WATCH_HAND_Print("EVT_SYSTEM_UTC_SECOND");
            if(!m_is_watch_hand_enter_check_mode)
            {
//                send_cur_pos_data();
                if ((!m_is_watch_hand_indicating) && (!watch_hand_compass_mode))
                {
                    watch_hand_move_to_utcpos();                   
                }
                watch_hand_1_move_to_utcpos();    
            }
            break;
        
        case EVT_SYSTEM_UTC_10SECOND:

            break;
        
      
        case EVT_BUTTON_PRESSED:
        {           
            if (BUTTON_EVT_WATCH_HAND_STEP_FORWARD == p_event->button.button_event)
            {

                watch_hand_step_forward(0,1);
            }
            else if (BUTTON_EVT_INDICATE_CALLIN == p_event->button.button_event)
            {
                #ifdef WH_IND_POS_CALL
                watch_hand_indication(&ind_config_callin);
                #endif
            }
            else if (BUTTON_EVT_INDICATE_NEW_MSG == p_event->button.button_event)
            {
                #ifdef WH_IND_POS_MSG
                watch_hand_indication(&ind_config_msg);
                #endif
            }
            else if (BUTTON_EVT_INDICATE_STEPS_GOAL == p_event->button.button_event)
            {
                #ifdef WH_IND_STEPS_POS_START
                uint8_t pos;
                if (NRF_SUCCESS == get_data_pos_from_ind_type(&ind_config_sport_target_test, &pos))
                {
                     ind_config_sport_target_test.ind_type_pos = pos;
                }
                watch_hand_indication(&ind_config_sport_target_test);

                //watch_hand_indication(&ind_config_sport_target);
                #endif
            }
            else if (BUTTON_EVT_COMPASS == p_event->button.button_event)
            {         
                 WATCH_HAND_Print("BUTTON_EVT_COMPASS");
                 /*打断其他指示，例如运动完成度*/
                 m_is_watch_hand_indicating = false;                
                 watch_hand_indication(&ind_config_compass);
            }
        #if TEST_WATCH_HAND
            else if (BUTTON_EVT_BTN0_SHORT_REL == p_event->button.button_event)
            {
                static uint8_t cnt = 0;
                if(cnt == 1)
                {
                    APP_ERROR_CHECK(watch_hand_stop());
                    cnt = 0;
                }
                else if(cnt == 0)
                {                    
                    APP_ERROR_CHECK(watch_hand_stop());
                    APP_ERROR_CHECK(watch_hand_step_backward(0,3600));
                    cnt =  1;
                }
            }
         #endif
            break;
        }
        case EVT_TEST_MODE_IN:
            m_test_mode_enter = true;
            break;
        
        
        case EVT_TEST_MOVT_HAND_0_FORWARD:
            if(m_test_mode_enter)
            {
                APP_ERROR_CHECK(watch_hand_stop());
                APP_ERROR_CHECK(watch_hand_step_forward(0,10800));
            }
            break;
        
        case EVT_TEST_MOVT_HAND_1_FORWARD:
            if(m_test_mode_enter)
            {
                APP_ERROR_CHECK(watch_hand_stop());
                APP_ERROR_CHECK(watch_hand_step_forward(1,10800));
            }
            break;
        default:
            break;
    }
    return false;
}

uint32_t m_watch_hand_init(void)
{
    uint32_t err_code;

    err_code = app_timer_create(&m_watch_hand_timer_id, APP_TIMER_MODE_SINGLE_SHOT, watch_hand_timeout_handler);
    if (err_code == NRF_SUCCESS)
    {
        err_code = drv_wh_init(watch_hand_event_handler);
    }

    /* 关机前会保存行针马达活动脚，开机后恢复 */
    drv_wh_config_active_pin(system_database.watch_hand_0_active_pin,system_database.watch_hand_1_active_pin);

    return err_code;
}



static void calibrate_process(check_mode_state_e state)
{
     static check_mode_state_e index = ENTER_CHECK_MODE;
     switch (index)
     {
         case ENTER_CHECK_MODE:
             if(state == ENTER_CHECK_MODE)
             {
                 index = SET_WATCH_HAND_PARAM;
             }
             break;
         case SET_WATCH_HAND_PARAM:
             if(state == SET_WATCH_HAND_PARAM)
             {
                 index = EXIT_CHECK_MODE;
             }
             break;
         case EXIT_CHECK_MODE:
             if(state == EXIT_CHECK_MODE)
             {
                 index = ENTER_CHECK_MODE;
                 watch_hand_start_check_timer =  true;   //start calibrate time        
             }
             break;
     }
}

bool m_watch_hand_wakeup_prepare(nrf_pwr_mgmt_evt_t shutdown_type)
{
    WATCH_HAND_Print(" %s \r\n", __func__);
    uint32_t err_code;

    err_code = watch_hand_stop();

    if (err_code == NRF_SUCCESS)
    {
        system_database.watch_hand_0_active_pin = drv_wh_get_active_pin(0);
        system_database.watch_hand_1_active_pin = drv_wh_get_active_pin(1);
        return true;
    }
    else
    {
        WATCH_HAND_Print("Watch hand wakeup prepare error : %d", err_code);
        return false;
    }
}

NRF_PWR_MGMT_HANDLER_REGISTER(m_watch_hand_wakeup_prepare, SHUTDOWN_PRIORITY_DEFAULT);



#endif /* CONFIG_WATCH_HAND_ENABLED */



