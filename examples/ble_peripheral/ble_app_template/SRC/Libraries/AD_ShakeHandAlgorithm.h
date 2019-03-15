#ifndef AD_WRISTMOTIONDETECT_H_
#define AD_WRISTMOTIONDETECT_H_

typedef enum
{
    SHAKE_HAND_EVT_TRIGGERED, //����˦���¼�
}shake_hand_evt_t;

/*
���ܣ�˦��Gsensor���ݴ�������ÿ0.5s����һ��
��������һ��0.5s ���µ�gsensor��������
���أ���˦�ֹ���δʹ�ܵ�ʱ�򷵻�0.ʹ�ܺ󷵻�1*/
unsigned char  wristMotionDetectProcess(unsigned char idx);

/*
���ܣ�˦�ֹ��ܳ�ʼ��
������˦�ִ����Ļص�����
���أ���*/
void shake_hand_init(void (*handler)(shake_hand_evt_t p_evt));

/*
���ܣ�˦�ֹ���ʹ��
��������
���أ���*/
void shake_hand_enable(void);

/*
���ܣ�˦�ֹ��ܹر�ʹ��
��������
���أ���*/
void shake_hand_disable(void);

/*
���ܣ���������𶯳�ʱʱ�䣬��������ڼ䣬˦�ֹ�����Ч����ʱ���Զ��ٴ��Զ���Ч
��������ʱʱ�䣬��λ0.5s,������ʱ��+1S
���أ���*/
void SetVibraTimeout(unsigned char Count);

/*
���ܣ����üƲ���ʱʱ�䣬���Ʋ��ڼ䣬˦�ֹ�����Ч����ʱ���Զ��ٴ��Զ���Ч
��������ʱʱ�䣬��λ0.5s, �̶�����1S
���أ���*/
void SetSetp2STpyTimeout(unsigned char Count);
#endif


