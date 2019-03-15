#ifndef AD_DATASTORE_H_
#define AD_DATASTORE_H_

#define AD_DATASTORE_VERSION "v0.0.2"
#define AD_DATASTORE_ID 0x0028

#include <stdint.h>
#include "string.h"
#include <stdbool.h>
#include "event_bus.h"

#define SLEEP_DATA 1
#define SPORT_DATA 0

#define MAC_ADDR       0x0002000 // MAC��ȡ��ַ
#define SN_ADDR        0x0001000 // �ɱ�sn��ȡ��ַ


#define     SLEEP_RECODE_ADDR                  (0x20000)


#define     DATA_SAMPLE_INTV 600

#define     SPORT_TIME_START_ADD                (0x40000)
#define     SPORT_DIRECTORY_START_ADD           (0x41000)
#define     SPORT_DATA_START_ADD                (0x42000)

#define     SPORT_DIRECTORY_END_ADD             (0x41000 + 0x1000)
#define     SPORT_DATA_END_ADD                  (0x42000 + 0x1000)

#define     SLEEP_TIME_START_ADD                (0x60000)
#define     SLEEP_DIRECTORY_START_ADD           (0x61000)
#define     SLEEP_DATA_START_ADD                (0x62000)


#define     SLEEP_DIRECTORY_END_ADD           (0x61000 + 0x1000)
#define     SLEEP_DATA_END_ADD                (0x62000 + 0x1000)

#define     NULL_SPORT_PAKG_THRESHOLD           12

/*
���ܣ���ȡָ�����������ж�����Ŀ¼
�������� SLEEP_DATA��SLEEP_DATA
���أ���ǰ�������͵�Ŀ¼����*/
uint16_t GetDirectoryCount(uint16_t data_type);

/*
���ܣ���ȡָ��Ŀ¼UTCʱ��
������
            count��Ŀ¼��ţ���0 ��ʼ��
            DataTpy���������ͣ��ο��� SLEEP_DATA��SLEEP_DATA
            *UTC��ͨ����ָ�뷵��UTCʱ��
���أ����ط�0 ���ɹ�*/
uint32_t GetDirectoryUTC_Time(uint16_t count, uint16_t DataTpy); 

/*
���ܣ���ȡָ��Ŀ¼���� �����ܷ���0
������
            count��Ŀ¼��ţ���0 ��ʼ��
            DataTpy���������ͣ��ο��� SLEEP_DATA��SLEEP_DATA
����:       Datalen��ͨ����ָ�뷵�������ܳ���*/
uint32_t GetDirectoryDataLen(uint16_t count, uint16_t DataTpy); //��ȡ�ƶ����ݵĳ���

/*
���ܣ����ָ��Ŀ¼���� �����ܷ���0  ˯�����ݴ���ר��
������
            count��Ŀ¼��ţ���0 ��ʼ��
            DataTpy���������ͣ��ο��� SLEEP_DATA��SLEEP_DATA
����:       Datalen��ͨ����ָ�뷵�������ܳ���*/
uint32_t CheckDirectoryDataLen(uint16_t count, uint16_t DataTpy); //��ȡ�ƶ����ݵĳ���



/*
���ܣ���ȡ�ƶ�Ŀ¼���ƶ��������ݣ�data��ʼ����ȫff��
������
            PakgCount�������
            DirectoryCount��Ŀ¼���
            DataTpy����������
            data��ͨ���ò����������ݣ���ʼ����ȫFF����          
���أ����ط�0 ���ɹ�*/
unsigned char GetSleepAndStepData(unsigned char PakgCount, uint16_t DirectoryCount,unsigned short DataTpy,unsigned char *data); 

/*
���ܣ�ɾ��ĳ������������
�������� SLEEP_DATA��SLEEP_DATA
���أ����ط�0 ���ɹ�*/
unsigned char DeleteDataTpye(unsigned char type) ;

/*
���ܣ�ɾ���ƶ����ͣ�ָ��ʱ������
������
            type:�������ͣ��ο��� SLEEP_DATA��SLEEP_DATA
            utcTime:�ƶ�Ҫɾ��������UTCʱ��
���أ����ط�0 ���ɹ�*/
unsigned char DeleteDataCount(unsigned char type,unsigned int utcTime);

/*
���ܣ����ݴ洢��ؽӿڣ�ÿ1/2s����һ��
��������
���أ���*/
void event_gStepData(void);


/*���ܣ���ʼ���洢���ݴ洢����
select���������ͣ��ο��� SLEEP_DATA��SLEEP_DATA
���أ����ط�0 ���ɹ�*/
unsigned char SleepAndSportDataStoreInit(unsigned char select);//��ʼ���洢

/*���ܣ����ػ�оMAC
������*data���������շ��ص�MAC��ַ��6�ֽ�
���أ���*/
uint32_t getMAC(unsigned char * data);

/*���ܣ����ػ�оsn
������*data���������շ��ص�SN,���128�ֽڣ�len���ȣ�len���ȶ����ݶ�20����λ�ֽ�
���أ���*/
void get_sn(unsigned char * data,unsigned char len);



/*���ܣ�д�ɱ�sn
������*data:��д��ɱ��sn�����ȣ��ݶ�20
���أ����ط�0 ���ɹ�*/
unsigned char setCusID(unsigned char * data,unsigned char len);

/*���ܣ���ȡ�ɱ�SN
������*data:�������շ��صĳɱ�SN,len�����ȣ��ݶ�20
���أ���*/
void getCusID(unsigned char * data,unsigned char len);  
unsigned char GetDirectoryState(unsigned char Tpy);// ��ȡ��ǰĿ¼��״̬
unsigned char SetDataEndTpy(unsigned char Tpy);// ����Ŀ¼������־

void FakeSetSleepAndSportData(void * p_context);
bool m_flash_test_event_handler(const event_t * p_evt);

#endif

