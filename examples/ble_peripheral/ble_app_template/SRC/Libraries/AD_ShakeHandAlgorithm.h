#ifndef AD_WRISTMOTIONDETECT_H_
#define AD_WRISTMOTIONDETECT_H_

typedef enum
{
    SHAKE_HAND_EVT_TRIGGERED, //触发甩手事件
}shake_hand_evt_t;

/*
功能：甩手Gsensor数据处理函数，每0.5s调用一次
参数：上一个0.5s 更新的gsensor数据条数
返回：当甩手功能未使能的时候返回0.使能后返回1*/
unsigned char  wristMotionDetectProcess(unsigned char idx);

/*
功能：甩手功能初始化
参数：甩手触发的回调函数
返回：无*/
void shake_hand_init(void (*handler)(shake_hand_evt_t p_evt));

/*
功能：甩手功能使能
参数：无
返回：无*/
void shake_hand_enable(void);

/*
功能：甩手功能关闭使能
参数：无
返回：无*/
void shake_hand_disable(void);

/*
功能：设置马达震动超时时间，（马达震动期间，甩手功能无效）超时后自动再次自动生效
参数：超时时间，单位0.5s,传入震动时间+1S
返回：无*/
void SetVibraTimeout(unsigned char Count);

/*
功能：设置计步超时时间，（计步期间，甩手功能无效）超时后自动再次自动生效
参数：超时时间，单位0.5s, 固定传入1S
返回：无*/
void SetSetp2STpyTimeout(unsigned char Count);
#endif


