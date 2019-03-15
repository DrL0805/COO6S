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

#define MAC_ADDR       0x0002000 // MAC读取地址
#define SN_ADDR        0x0001000 // 成表sn读取地址


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
功能：获取指定类型数据有多少条目录
参数：宏 SLEEP_DATA，SLEEP_DATA
返回：当前数据类型的目录条数*/
uint16_t GetDirectoryCount(uint16_t data_type);

/*
功能：获取指定目录UTC时间
参数：
            count：目录序号，从0 开始算
            DataTpy：数据类型，参考宏 SLEEP_DATA，SLEEP_DATA
            *UTC：通过该指针返回UTC时间
返回：返回非0 不成功*/
uint32_t GetDirectoryUTC_Time(uint16_t count, uint16_t DataTpy); 

/*
功能：获取指定目录长度 ，可能返回0
参数：
            count：目录序号，从0 开始算
            DataTpy：数据类型，参考宏 SLEEP_DATA，SLEEP_DATA
返回:       Datalen：通过该指针返回数据总长度*/
uint32_t GetDirectoryDataLen(uint16_t count, uint16_t DataTpy); //获取制定数据的长度

/*
功能：检查指定目录长度 ，可能返回0  睡眠数据处理专用
参数：
            count：目录序号，从0 开始算
            DataTpy：数据类型，参考宏 SLEEP_DATA，SLEEP_DATA
返回:       Datalen：通过该指针返回数据总长度*/
uint32_t CheckDirectoryDataLen(uint16_t count, uint16_t DataTpy); //获取制定数据的长度



/*
功能：获取制定目录，制定包号数据，data初始化成全ff，
参数：
            PakgCount：包序号
            DirectoryCount：目录序号
            DataTpy：数据类型
            data：通过该参数传出数据，初始化成全FF传入          
返回：返回非0 不成功*/
unsigned char GetSleepAndStepData(unsigned char PakgCount, uint16_t DirectoryCount,unsigned short DataTpy,unsigned char *data); 

/*
功能：删除某类型所有数据
参数：宏 SLEEP_DATA，SLEEP_DATA
返回：返回非0 不成功*/
unsigned char DeleteDataTpye(unsigned char type) ;

/*
功能：删除制定类型，指定时间数据
参数：
            type:数据类型，参考宏 SLEEP_DATA，SLEEP_DATA
            utcTime:制定要删除的数据UTC时间
返回：返回非0 不成功*/
unsigned char DeleteDataCount(unsigned char type,unsigned int utcTime);

/*
功能：数据存储相关接口，每1/2s调用一次
参数：无
返回：无*/
void event_gStepData(void);


/*功能：初始化存储数据存储区域
select：数据类型，参考宏 SLEEP_DATA，SLEEP_DATA
返回：返回非0 不成功*/
unsigned char SleepAndSportDataStoreInit(unsigned char select);//初始化存储

/*功能：返回机芯MAC
参数：*data，用来接收返回的MAC地址，6字节
返回：空*/
uint32_t getMAC(unsigned char * data);

/*功能：返回机芯sn
参数：*data，用来接收返回的SN,最多128字节，len长度（len事先定义暂定20）单位字节
返回：空*/
void get_sn(unsigned char * data,unsigned char len);



/*功能：写成表sn
参数：*data:待写入成表的sn，长度，暂定20
返回：返回非0 不成功*/
unsigned char setCusID(unsigned char * data,unsigned char len);

/*功能：读取成表SN
参数：*data:用来接收返回的成表SN,len，长度，暂定20
返回：空*/
void getCusID(unsigned char * data,unsigned char len);  
unsigned char GetDirectoryState(unsigned char Tpy);// 获取当前目录的状态
unsigned char SetDataEndTpy(unsigned char Tpy);// 设置目录结束标志

void FakeSetSleepAndSportData(void * p_context);
bool m_flash_test_event_handler(const event_t * p_evt);

#endif

