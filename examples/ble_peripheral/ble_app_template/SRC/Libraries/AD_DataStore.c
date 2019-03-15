#include "AD_DataStore.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "m_drv_w25x80.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "m_database.h"
#include "AppStepDrive.h"
#include "AD_SleepAlgorithm.h"

#include "includes.h"

#if CONFIG_RTT_AD_DATASTORE_ENABLED
#define DataStore_Print(M, ...)	SEGGER_Print("DataStore", M, ##__VA_ARGS__)
#else
#define DataStore_Print(M, ...)
#endif


#define     DirectoryDataLenMax            288


static unsigned char NewSportDirectoryState = 0;
static unsigned char NewSleepDirectoryState = 0;


void updateStartTime(unsigned int * tBeforeCorrect, unsigned int * tAfterCorrect, unsigned char select)//
{
	
}

unsigned char FlashWrite(unsigned int addr, unsigned char * sendData, unsigned short size)
{
//    DBG1_EVT2("FlashWrite :addr :%x, size :%d",addr,size);
    unsigned char i = 0;
    unsigned int AddrTemp = addr;
    unsigned char *sendDataTemp = sendData;
    unsigned short sizeTemp = size;
    for(i=0;;i++)
    {
        if((AddrTemp%256 + sizeTemp)>256) //
        {
            DataStore_Print("over page :addr : %x",AddrTemp);
            if(AddrTemp%4096 == 0)
            {
                   DataStore_Print("w25x80_erase_sector 111 :addr :%x",AddrTemp);
                    w25x80_erase_sector(AddrTemp);
            }
            w25x80_write(AddrTemp,sendDataTemp,256 -(AddrTemp%256));//
            sizeTemp -= 256 -(AddrTemp%256);
            sendDataTemp += 256 -(AddrTemp%256);
            AddrTemp += 256 -(AddrTemp%256);
        }
        else
        {
            if(sizeTemp > 0)
            {
                if(AddrTemp%4096 == 0)
                {
                        DataStore_Print("w25x80_erase_sector 222 :addr :%x",AddrTemp);
                        w25x80_erase_sector(AddrTemp);
                }
                w25x80_write(AddrTemp,sendDataTemp,sizeTemp);
   
            }
            break;
        }
    }
		return 0;
}

unsigned char readData(unsigned long addr, unsigned char *recData, unsigned int size)
{
	unsigned char i = 0;
    unsigned int AddrTemp = addr;
    unsigned char *sendDataTemp = recData;
    unsigned short sizeTemp = size;
    for(i=0;;i++)
    {
        if((AddrTemp%256 + sizeTemp)>256) //
        {
            w25x80_read(AddrTemp,sendDataTemp,256 -(AddrTemp%256));//
            sizeTemp -= 256 -(AddrTemp%256);
            sendDataTemp += 256 -(AddrTemp%256);
            AddrTemp += 256 -(AddrTemp%256);
        }
        else
        {
            if(sizeTemp > 0)
            {
                w25x80_read(AddrTemp,sendDataTemp,sizeTemp);
            }
            break;
        }
    }
		return 0;
}

unsigned char WriteDataFlash(unsigned char *data, unsigned char len, unsigned char Tpy)
{
   	//uint32_t *DirectoryAdd = Tpy ? (&SleepDataWriteAdd) : (&SportDataWriteAdd);
	uint32_t *DirectoryAdd = Tpy ? (&system_database.SleepDataWriteAdd) : (&system_database.SportDataWriteAdd);
//    DBG1_EVT1("WriteDataFlash DirectoryAdd :%08x",*DirectoryAdd);   
	FlashWrite(*DirectoryAdd, data, len);//需解决夸扇区写的问题
	
	*DirectoryAdd += len;
	return 0;
}

unsigned char GetDirectoryState(unsigned char Tpy)// 获取当前目录的状态
{
	uint32_t DirectoryAdd = Tpy ? (NewSleepDirectoryState) : (NewSportDirectoryState);
	return DirectoryAdd;
}
/*
static void write_temp_to_flash(uint8_t tpy)
{
    if(tpy)
    {
        if(TempSleepPakgCount)
        {
            
            WriteDataFlash((unsigned char *)TempSleepPakgBuff, TempSleepPakgCount * 2, 1);
            TempSleepPakgCount=0;
        }
        
    }
    else
    {
        if(TempSportPakgCount)
        {
            
            WriteDataFlash((unsigned char *)TempSportPakgBuff, TempSportPakgCount * 2, 1);
            TempSportPakgCount=0;
        }            
    }
    
}*/

unsigned char SetDataEndTpy(unsigned char Tpy)// 设置目录结束标志
{
    /*结束目录前，保存缓冲区的数据*/
    //write_temp_to_flash(Tpy);
//    DBG1_EVT0("SetDataEndTpy");
	unsigned char Data[2] = { 0x00,0xc0 };
	if (Tpy == 0)// 
	{
		if (NewSportDirectoryState == 0)
		{
			return 1;
		}
		NewSportDirectoryState = 0;
	}
	else
	{
		if (NewSleepDirectoryState == 0)
		{
			return 1;
		}
		NewSleepDirectoryState = 0;
	}
	WriteDataFlash(Data, 2, Tpy);
	return 0;
}

unsigned char SetNewDirectory(unsigned char Tpy,unsigned int TimeKey)//      设置一个新目录
{
	uint32_t TempDataAdd = 0;
	uint32_t *TempDirectoryAdd = 0;
	unsigned char *TempDirectoryState = 0;
	/*
	uint32_t SportDirectoryNumber = (SportDirectoryWriteAdd - SPORT_DIRECTORY_START_ADD)/4;
	uint32_t SleepDirectoryNumber = (SleepDirectoryWriteAdd - SLEEP_DIRECTORY_START_ADD)/4;

	uint32_t SportDataLen = SportDataWriteAdd - SPORT_DATA_START_ADD;
	uint32_t SleepDataLen = SleepDataWriteAdd - SLEEP_DATA_START_ADD;
	*/

	uint32_t SportDirectoryNumber = (system_database.SportDirectoryWriteAdd - SPORT_DIRECTORY_START_ADD)/4;
	uint32_t SleepDirectoryNumber = (system_database.SleepDirectoryWriteAdd - SLEEP_DIRECTORY_START_ADD)/4;

	uint32_t SportDataLen = system_database.SportDataWriteAdd - SPORT_DATA_START_ADD;
	uint32_t SleepDataLen = system_database.SleepDataWriteAdd - SLEEP_DATA_START_ADD;

	uint32_t TimeTemp = 0;
	uint16_t TimeCount = ((((SportDataLen - (SportDirectoryNumber * 4)) + (SleepDataLen - (SleepDirectoryNumber * 4))))/2)|0x8000;
    //DBG1_EVT1("SetNewDirectory:TimeCount: %04x",TimeCount);

//	uint16_t temp = TimeCount & 0xff;	  数据高低位互换这里不用
//	TimeCount = (TimeCount >> 8) + (temp << 8);

	unsigned char TempData[4] = { 0x55 };
	if (Tpy == 0)
	{
		//TempDataAdd = SportDataWriteAdd;
		TempDataAdd = system_database.SportDataWriteAdd;
		//if(SportDirectoryWriteAdd >= (SPORT_DIRECTORY_START_ADD+4096))
		if( system_database.SportDirectoryWriteAdd >= (SPORT_DIRECTORY_START_ADD+4096) )
		{
				SleepAndSportDataStoreInit(Tpy);
		}
		//TempDirectoryAdd = &SportDirectoryWriteAdd;
		TempDirectoryAdd = &system_database.SportDirectoryWriteAdd;
		TempDirectoryState = &NewSportDirectoryState;
	}
	else
	{
		TempDataAdd = system_database.SleepDataWriteAdd;
//        DBG1_EVT1("SleepDirectoryAddr:  %04x",system_database.SleepDirectoryWriteAdd - SLEEP_DIRECTORY_START_ADD);
		if( system_database.SleepDirectoryWriteAdd >= (SLEEP_DIRECTORY_START_ADD+4096) )
		{
				SleepAndSportDataStoreInit(Tpy);
		}
		TempDirectoryAdd = &system_database.SleepDirectoryWriteAdd;
		TempDirectoryState = &NewSleepDirectoryState;
	}
    DataStore_Print("TempDirectoryAdd :%08x  ,TempDataAdd :%08x",*TempDirectoryAdd,TempDataAdd);   
	TempData[1] = TempDataAdd & 0xff;
	TempData[2] = (TempDataAdd >> 8) & 0xff;
	TempData[3] = (TempDataAdd >> 16) & 0xff;
	FlashWrite(*TempDirectoryAdd, TempData, 4);
	*TempDirectoryState = 1;
	*TempDirectoryAdd += 4;
	
	WriteDataFlash((unsigned char *)(&TimeCount),2, Tpy);
	TimeTemp = system_database.utc_seconds;//获取去时间
	TimeTemp = TimeTemp - (TimeKey *60);
    //DBG1_EVT1("SetNewDirectory:TimeTemp:  %08x",TimeTemp);
	WriteDataFlash((unsigned char *)(&TimeTemp),4, Tpy);
	return 0;
}

static unsigned char NullSportPakgCount = 0;
static uint16_t NullSportPakgBuff[NULL_SPORT_PAKG_THRESHOLD] = { 0 };



void SleepDataProcess(uint16_t data)
{   
	uint16_t Temp = 0;
	Temp = data & 0xff;		 	//高低位调换
	data = (data >> 8) + (Temp << 8);

	if (GetDirectoryState(0))//有计步目录
	{
		NullSportPakgBuff[NullSportPakgCount] = data;
		NullSportPakgCount++;
		if (NullSportPakgCount > (NULL_SPORT_PAKG_THRESHOLD - 1))//空包计数大于阈值
		{
			NullSportPakgCount = 0;
			SetDataEndTpy(0);
			if (NewSleepDirectoryState)//睡眠目录存在
			{
				while (1);//出错了
			}
			else
			{
				SetDataEndTpy(0);
				SetNewDirectory(1,(NULL_SPORT_PAKG_THRESHOLD-1) *5);
				WriteDataFlash((unsigned char *)NullSportPakgBuff, NULL_SPORT_PAKG_THRESHOLD * 2, 1);
			}
		}
	}
	else //没有计步目录
	{
		if (!GetDirectoryState(1))//睡眠目录不存在
		{
			SetNewDirectory(1,NullSportPakgCount*5);
		}
		if(NullSportPakgCount !=0)
		{
			WriteDataFlash((unsigned char *)NullSportPakgBuff, NullSportPakgCount * 2, 1);
			NullSportPakgCount = 0;
		}
        /*缓存一个小时数据*/
        #if 0
        TempSleepPakgBuff[TempSleepPakgCount] = data;
        TempSleepPakgCount++;
        if(TempSleepPakgCount > NULL_SPORT_PAKG_THRESHOLD-1)
        {
            TempSleepPakgCount=0;
            WriteDataFlash((unsigned char *)TempSleepPakgBuff, NULL_SPORT_PAKG_THRESHOLD * 2, 1);
        }
        #else       
		WriteDataFlash((unsigned char *)(&data), 2, 1);//写入睡眠数据
        #endif
        
        uint16_t count_temp = GetDirectoryCount(1);
        uint16_t len_temp = CheckDirectoryDataLen(count_temp-1,1);
//        DBG1_EVT2("TimeTemp: %d, CheckDirectoryDataLen :%d",count_temp,len_temp);
        if(len_temp >= DirectoryDataLenMax) //单个目录长度288字节（12个小时），结束目录   //  CheckDirectoryDataLen
        {
//            DBG1_EVT0("SleepDataProcess : SetDataEndTpy ");
            SetDataEndTpy(1);
        }
	}
}

void SportDataProcess(uint16_t data)
{
	uint16_t Temp = 0;
	Temp = data & 0xff;		 	//高地位调换
	data = (data >> 8) + (Temp << 8);
	if (GetDirectoryState(1))//睡眠目录存在
	{
		SetDataEndTpy(1);
	}
	if(!GetDirectoryState(0))//没有计步目录
	{
		SetNewDirectory(0,NullSportPakgCount *5);
	}
	if(NullSportPakgCount != 0)
	{
		memset(NullSportPakgBuff, 0, NullSportPakgCount*2);
		WriteDataFlash((unsigned char *)NullSportPakgBuff, NullSportPakgCount * 2, 0);
		NullSportPakgCount = 0;
	}
     /*缓存一个小时数据*/
    #if 0
    TempSportPakgBuff[TempSportPakgCount] = data;
    TempSportPakgCount++;
    if(TempSportPakgCount > NULL_SPORT_PAKG_THRESHOLD-1)
    {
        TempSportPakgCount=0;
        WriteDataFlash((unsigned char *)TempSportPakgBuff, NULL_SPORT_PAKG_THRESHOLD * 2, 1);
    }
    #else
	WriteDataFlash((unsigned char *)(&data), 2, 0);//写入睡眠数据
    #endif
}




#define FAKE_DATA  0

#if FAKE_DATA
#include "app_timer.h"
//#include "fake_data.h"
#include "resources.h"
APP_TIMER_DEF(fake_data_timer_id);
APP_TIMER_DEF(print_data_timer_id);
void print_data_log(void * p_context);
static void fake_data_timeout_timer_handler(void * p_context);
#endif





void setSleepAndSportData(void)// 
{
    unsigned int Sport_5min = 0;
    unsigned int Sleep_5min = 0;
						
    Sport_5min = gSteps_get();
    gSteps_set(0);

    if (Sport_5min == 0)// 
    {
        getGsensorDiffData(&Sleep_5min);
        setGsensorDiffData(0);
        gensor_set_1hz();  //没有计步，降低采样率
        SleepDataProcess(Sleep_5min);
    }
    else
    {
    	  SportDataProcess(Sport_5min);

    } 
}

void event_gStepData(void)//计算步数累加和事件
{
    static unsigned short TimerTick1_2s = 0;//1s节拍器
    TimerTick1_2s++;//1s计数 加1

    if (TimerTick1_2s >= DATA_SAMPLE_INTV)//5min判断
    {
        setSleepAndSportData();//写数据到Flash
        TimerTick1_2s = 0;//1s计数
    }
    #if FAKE_DATA 
    static uint8_t first_in =1;
    if(first_in)
    {
        app_timer_create(&fake_data_timer_id, APP_TIMER_MODE_REPEATED, fake_data_timeout_timer_handler);
        app_timer_start(fake_data_timer_id,APP_TIMER_TICKS(100),NULL);
        first_in =0;
    }
    #endif
}

void m_data_store_event_handler(const event_t *p_event)
{
    if(p_event->type == EVT_SYSTEM_STATE)
    {
        
    }
}


#if 0
static uint32_t sleep_dir_addr_add =SLEEP_DIRECTORY_START_ADD;
static uint32_t sport_dir_addr_add =SLEEP_DIRECTORY_START_ADD;

 
uint16_t GetDirectoryCount(uint16_t tpy)// 获取目录数，不算已经删除的
{
	unsigned char DataAddTemp[4] = { 0,0,0,0 };
	uint32_t *addr_temp_add = tpy ?  (&sleep_dir_addr_add) : (&sport_dir_addr_add);
	uint16_t  i = 0;
    
    while (1)//判断当前连续目录是否已经被删除，删除的
	{
		readData(*addr_temp_add, DataAddTemp, 4);
		if (DataAddTemp[0] == 0xff)
		{
			break;
		}
		else if (DataAddTemp[0] != 0)
		{
			break;
		}
        else //当前目录已经被删除
        {
            *addr_temp_add += 4;  //更新起始读目录
        }
		
	}
    
    
    uint32_t addr_temp = *addr_temp_add;
    DBG1_EVT1("start addr_temp  %d ",addr_temp);    
	while (1)
	{
		readData(addr_temp, DataAddTemp, 4);
		if (DataAddTemp[0] == 0xff)
		{
			break;
		}
		else if (DataAddTemp[0] != 0)
		{
			i++;
		}
        if(i > 9)  //获取目录数的操作一次最多只告知app有9条目录
        {
            break;
        }
		addr_temp += 4;
	}
	return i;
}
#else
uint16_t GetDirectoryCount(uint16_t tpy)// 
{
	
	uint32_t add = tpy ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);

    
    #define READ_SIZE 128
    uint8_t data_temp[READ_SIZE]={0xff};
    uint16_t count = 0;
    while(1)
    {
        readData(add, data_temp, READ_SIZE);
        add += READ_SIZE;
        for(int j=0; j< (READ_SIZE/4);j++)
        {
            if(data_temp[4*j] == 0xff)
            {
                return count;
            }
            else if(data_temp[4*j] != 0)
            {
                count++;
            }
        }
    }
}
#endif



uint16_t GetDirectoryNume(uint16_t data_type, uint16_t Count)// 根据有效目录的序号，获取目录在flash的序号
{
	
	uint32_t add = data_type ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);


    #define READ_SIZE 128
    uint8_t data_temp[READ_SIZE]={0xff};
    uint16_t count = 0;
    uint16_t cnt = 0;
    while(1)
    {
        readData(add, data_temp, READ_SIZE);
        add += READ_SIZE;
        for(int j=0; j< (READ_SIZE/4);j++)
        {
            if(data_temp[4*j] == 0xff)
            {
                return 0x8000;
            }
            else if(data_temp[4*j] != 0)
            {
                cnt++;
                if(cnt == (Count +1))
                {
                    return count;
                }
            }
            count++;
        }
    }
}

uint32_t GetDirectoryUTC_Time(uint16_t count, uint16_t DataTpy)	// 
{
	uint32_t DataStartAddr = DataTpy ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);
	unsigned char TempData[4] = { 0,0,0,0 };
	//unsigned long TemAdd = 0;
	uint32_t time = 0;
	//unsigned long TemTime = 0;
	count = GetDirectoryNume(DataTpy, count);
	if (count & 0x8000)
	{
		return 0;
	}
	readData(DataStartAddr + count * 4, TempData, 4);//TempData 55 地址高 次高 低
	
	readData(((*((uint32_t*)(TempData)))>>8)+2, TempData, 4);
	
	memcpy((unsigned char *)(&time),TempData,4);
	//TemTime = invLong(TemTime);
	//*time = (TemTime);

	return time;
}

unsigned char DeleteDataCount(unsigned char type, uint32_t utcTime)	// 根据utc时间删除指定目录
{
	uint32_t i = 0;
	uint32_t TempTime = 0;
	uint32_t TempAddr = type ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);

//	DBG1_EVT0("delete data Test");
	for (i = 0;; i++)
	{
		TempTime = GetDirectoryUTC_Time(i, type);
		if (TempTime == 0)
		{
			return 1;
		}
		if (utcTime == TempTime)
		{
			break;
		}
	}
	
	unsigned int TempD = 0;
	TempD = GetDirectoryCount(type);
#if 0    
    uint8_t buf[100];
    w25x80_read(SPORT_DIRECTORY_START_ADD, buf,100);
    for(int i=0;i<100;i++)
    {
        NRF_LOG_RAW_INFO("%02x ",buf[i]);
        if(((i+1)%4==0)&&(i!=0))
        {
            NRF_LOG_RAW_INFO(" ");
            if((i+1)%16==0)
            {
               NRF_LOG_RAW_INFO("\r\n"); 
            }            
        }
    }
    NRF_LOG_RAW_INFO("\r\n");
#endif
//    DBG1_EVT1("GetDirectoryCount 111 TempD %d ",TempD);
	if (TempD == (i + 1))
	{
		SetDataEndTpy(type);
	}
	
	i = GetDirectoryNume(type, i);
	unsigned char TempData = 0x00;
    w25x80_write(TempAddr + (i * 4), &TempData, 1);
	TempD = GetDirectoryCount(type);
//    DBG1_EVT1("GetDirectoryCount 222 TempD %d ",TempD);

#if 0    
    w25x80_read(SPORT_DIRECTORY_START_ADD, buf,100);
    for(int i=0;i<100;i++)
    {
        NRF_LOG_RAW_INFO("%02x ",buf[i]);
        if(((i+1)%4==0)&&(i!=0))
        {
            NRF_LOG_RAW_INFO(" ");
            if((i+1)%15==0)
            {
               NRF_LOG_RAW_INFO("\r\n"); 
            }            
        }
    }
    NRF_LOG_RAW_INFO("\r\n");
#endif   

//	if(TempD == 0)
//	{
//			DataStore_Print("delete data Test %d ",type);
//			SleepAndSportDataStoreInit(type);
//	}   
	return 0;
}

uint32_t CheckDirectoryDataLen(uint16_t count, uint16_t DataTpy)// 检查当前目录长度，（睡眠包处理保存专用，判断单个目录数据是否很长）
{
	unsigned char StartTempDirectoryV[4] = { 0 };
	unsigned char StopTempDirectoryV[4] = { 0 };
	uint32_t DirectoryAddr = DataTpy ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);
	//uint32_t DtatAddr = DataTpy ? (SleepDataWriteAdd) : (SportDataWriteAdd);
	uint32_t DtatAddr = DataTpy ? (system_database.SleepDataWriteAdd) : (system_database.SportDataWriteAdd);
	uint16_t Len =0;
	
	count = GetDirectoryNume(DataTpy, count);
	if (count & 0x8000)
	{
		return 0;
	}
	readData(DirectoryAddr + (count * 4), StartTempDirectoryV, 4);
	readData(DirectoryAddr + (count * 4) + 4, StopTempDirectoryV, 4);
	
	if (StopTempDirectoryV[0] != 0xff)
	{
		Len = (*((uint32_t *)(StopTempDirectoryV)) >> 8) - (*((uint32_t *)(StartTempDirectoryV)) >> 8) - 8;
	}
	else
	{
		Len = DtatAddr - (*((uint32_t *)(StartTempDirectoryV)) >> 8) - 6;
		unsigned char TempDatax[2] = { 0 };
		readData(DtatAddr - 2, TempDatax, 2);
		if (TempDatax[1] & 0xc0)// 
		{
			Len -= 2;
		}
	}
	return Len;
}


uint32_t GetDirectoryDataLen(uint16_t count, uint16_t DataTpy)// 
{
	unsigned char StartTempDirectoryV[4] = { 0 };
	unsigned char StopTempDirectoryV[4] = { 0 };
	uint32_t DirectoryAddr = DataTpy ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);
	//uint32_t DtatAddr = DataTpy ? (SleepDataWriteAdd) : (SportDataWriteAdd);
	uint32_t DtatAddr = DataTpy ? (system_database.SleepDataWriteAdd) : (system_database.SportDataWriteAdd);
	uint16_t Len =0;
	
	count = GetDirectoryNume(DataTpy, count);
	if (count & 0x8000)
	{
		return 0;
	}
	readData(DirectoryAddr + (count * 4), StartTempDirectoryV, 4);
	readData(DirectoryAddr + (count * 4) + 4, StopTempDirectoryV, 4);
    DataStore_Print("StartTempDirectoryV : %02x %02x %02x %02x ",StartTempDirectoryV[0],StartTempDirectoryV[1],StartTempDirectoryV[2],StartTempDirectoryV[3]);
    DataStore_Print("StopTempDirectoryV : %02x %02x %02x %02x ",StopTempDirectoryV[0],StopTempDirectoryV[1],StopTempDirectoryV[2],StopTempDirectoryV[3]);
    

    DataStore_Print("!!!!!!data_addr_temp : %x stop_data_addr_temp : %x !!!!!!",(*((uint32_t *)(StartTempDirectoryV)) >> 8),(*((uint32_t *)(StopTempDirectoryV)) >> 8));
    
    
 
	if (StopTempDirectoryV[0] != 0xff)
	{
		Len = (*((uint32_t *)(StopTempDirectoryV)) >> 8) - (*((uint32_t *)(StartTempDirectoryV)) >> 8) - 8;
        DataStore_Print("GetDirectoryDataLen 111: %d",Len);
	}
	else
	{
//        DBG1_EVT0("GetDirectoryDataLen : SetDataEndTpy");
        SetDataEndTpy(DataTpy); 
		Len = DtatAddr - (*((uint32_t *)(StartTempDirectoryV)) >> 8) - 6;
        DataStore_Print("DtatAddr:%08x",DtatAddr);
        DataStore_Print("GetDirectoryDataLen 222: %d",Len);
		unsigned char TempDatax[2] = { 0 };
		readData(DtatAddr - 2, TempDatax, 2);
        DataStore_Print("TempDatax:%02x %02x",TempDatax[0],TempDatax[1]);
		if (TempDatax[1] & 0xc0)// 
		{
			Len -= 2;
            DataStore_Print("GetDirectoryDataLen 333: %d",Len);
		}
	}
    /*单个目录长度已经大于限制最大长度2倍了，说明发生了异常情况，重新初始化*/
    if( Len > DirectoryDataLenMax * 2)
    {
        SleepAndSportDataStoreInit(DataTpy);
        Len = 0;
    }
	return Len;
}

unsigned char GetSleepAndStepData(unsigned char PakgCount, uint16_t  DirectoryCount, uint16_t DataTpy, unsigned char *data)	// 
{
	uint16_t TempLen = 0;
	uint32_t DirectoryAddr = DataTpy ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);
	uint32_t TempAdd = 0;
	uint32_t PakgLenTemp = 0;
	unsigned char StartTempDirectoryV[4] = { 0 };
	TempLen = GetDirectoryDataLen(DirectoryCount, DataTpy);
	
	if (((PakgCount) * 16) >= TempLen)
	{
		return 1;
	}
	DirectoryCount = GetDirectoryNume(DataTpy, DirectoryCount);
	if (DirectoryCount & 0x8000)
	{
		return 1;
	}
    DataStore_Print("[GetSleepAndStepData] TempLen: %d, DirectoryCount: %d, readData: %08x.", TempLen, DirectoryCount, DirectoryAddr + (DirectoryCount * 4));
	readData(DirectoryAddr + (DirectoryCount * 4), StartTempDirectoryV, 4);
	memcpy((unsigned char *)(&TempAdd),StartTempDirectoryV,4);
	TempAdd = TempAdd >>8;
//	TempAdd = (*((uint32 *)(StartTempDirectoryV)) >> 8) + 2;
	PakgLenTemp = TempLen - PakgCount * 16;

	if (PakgLenTemp > 16)
	{
		PakgLenTemp = 16;
	}
	readData(TempAdd + PakgCount * 16 + 6, data, PakgLenTemp);
	return 0;
}

unsigned char SleepAndSportDataStoreInit(unsigned char select)//??,????
{
    DataStore_Print("SleepAndSportDataStoreInit");
	if(select)
	{
        DataStore_Print("w25x80_erase_sector 333 :addr :%x",SLEEP_DIRECTORY_START_ADD);
		w25x80_erase_sector(SLEEP_DIRECTORY_START_ADD);
        DataStore_Print("w25x80_erase_sector 333 :addr :%x",SLEEP_DATA_START_ADD);
		w25x80_erase_sector(SLEEP_DATA_START_ADD);
    	/*SleepDataWriteAdd = SLEEP_DATA_START_ADD;	
		SleepDirectoryWriteAdd = SLEEP_DIRECTORY_START_ADD;	*/
		system_database.SleepDataWriteAdd = SLEEP_DATA_START_ADD;	// 
		system_database.SleepDirectoryWriteAdd = SLEEP_DIRECTORY_START_ADD;	// 
		NewSleepDirectoryState = 0;
	}
	else
	{
        DataStore_Print("w25x80_erase_sector 444 :addr :%x",SPORT_DIRECTORY_START_ADD);
		w25x80_erase_sector(SPORT_DIRECTORY_START_ADD);
        DataStore_Print("w25x80_erase_sector 444 :addr :%x",SPORT_DATA_START_ADD);
		w25x80_erase_sector(SPORT_DATA_START_ADD);
		/*
		SportDataWriteAdd = SPORT_DATA_START_ADD;	// 
		SportDirectoryWriteAdd = SPORT_DIRECTORY_START_ADD;	//
		*/
		system_database.SportDataWriteAdd = SPORT_DATA_START_ADD;	// 
		system_database.SportDirectoryWriteAdd = SPORT_DIRECTORY_START_ADD;	// 
		NewSportDirectoryState = 0;
	}
	return 0;
}

unsigned char DeleteDataTpye(unsigned char type)																					
{
    DataStore_Print("DeleteDataTpye");
	SleepAndSportDataStoreInit(type);
	return 0;
}


#if FAKE_DATA

/*******************************************
***************造假专用*********************
********************************************/



static unsigned char FakeNullSportPakgCount = 0;
static uint16_t FakeNullSportPakgBuff[NULL_SPORT_PAKG_THRESHOLD] = { 0 };

uint32_t fake_utc_seconds = 1535760000;//09 01
unsigned char FakeSetNewDirectory(unsigned char Tpy,unsigned int TimeKey)  //设置一个新目录
{
	uint32_t TempDataAdd = 0;
	uint32_t *TempDirectoryAdd = 0;
	unsigned char *TempDirectoryState = 0;

	uint32_t SportDirectoryNumber = (system_database.SportDirectoryWriteAdd - SPORT_DIRECTORY_START_ADD)/4;
	uint32_t SleepDirectoryNumber = (system_database.SleepDirectoryWriteAdd - SLEEP_DIRECTORY_START_ADD)/4;

	uint32_t SportDataLen = system_database.SportDataWriteAdd - SPORT_DATA_START_ADD;
	uint32_t SleepDataLen = system_database.SleepDataWriteAdd - SLEEP_DATA_START_ADD;

	uint32_t TimeTemp = 0;
	uint16_t TimeCount = ((((SportDataLen - (SportDirectoryNumber * 4)) + (SleepDataLen - (SleepDirectoryNumber * 4))))/2)|0x8000;
    //DBG1_EVT1("SetNewDirectory:TimeCount: %04x",TimeCount);

	unsigned char TempData[4] = { 0x55 };
	if (Tpy == 0)
	{
		TempDataAdd = system_database.SportDataWriteAdd;
		if(system_database.SportDirectoryWriteAdd >= (SPORT_DIRECTORY_START_ADD+4096))
		{
				SleepAndSportDataStoreInit(Tpy);
		}
		TempDirectoryAdd = &system_database.SportDirectoryWriteAdd;
		TempDirectoryState = &NewSportDirectoryState;
	}
	else
	{
		TempDataAdd = system_database.SleepDataWriteAdd;
		if(system_database.SleepDirectoryWriteAdd >= (SLEEP_DIRECTORY_START_ADD+4096))
		{
				SleepAndSportDataStoreInit(Tpy);
		}
		TempDirectoryAdd = &system_database.SleepDirectoryWriteAdd;
		TempDirectoryState = &NewSleepDirectoryState;
	}
	TempData[1] = TempDataAdd & 0xff;
	TempData[2] = (TempDataAdd >> 8) & 0xff;
	TempData[3] = (TempDataAdd >> 16) & 0xff;
	FlashWrite(*TempDirectoryAdd, TempData, 4);
	*TempDirectoryState = 1;
	*TempDirectoryAdd += 4;
	
	WriteDataFlash((unsigned char *)(&TimeCount),2, Tpy);
	TimeTemp = fake_utc_seconds;//获取去时间
	TimeTemp = TimeTemp - (TimeKey *60);
    //DBG1_EVT1("SetNewDirectory:TimeTemp:  %08x",TimeTemp);
	WriteDataFlash((unsigned char *)(&TimeTemp),4, Tpy);
	return 0;
}

 




void FakeSleepDataProcess(uint16_t data)
{
	uint16_t Temp = 0;
	Temp = data & 0xff;		 	//高低位调换
	data = (data >> 8) + (Temp << 8);

	if (GetDirectoryState(0))//有计步目录
	{
		FakeNullSportPakgBuff[FakeNullSportPakgCount] = data;
		FakeNullSportPakgCount++;
		if (FakeNullSportPakgCount > (NULL_SPORT_PAKG_THRESHOLD - 1))//空包计数大于阈值
		{
			FakeNullSportPakgCount = 0;
			SetDataEndTpy(0);
			if (NewSleepDirectoryState)//睡眠目录存在
			{
				while (1);//出错了
			}
			else
			{
				SetDataEndTpy(0);
				FakeSetNewDirectory(1,(NULL_SPORT_PAKG_THRESHOLD-1) *5);
				WriteDataFlash((unsigned char *)FakeNullSportPakgBuff, NULL_SPORT_PAKG_THRESHOLD * 2, 1);
			}
		}
	}
	else //没有计步目录
	{
		if (!GetDirectoryState(1))//睡眠目录不存在
		{
			FakeSetNewDirectory(1,FakeNullSportPakgCount*5);
		}
		if(FakeNullSportPakgCount !=0)
		{
			WriteDataFlash((unsigned char *)NullSportPakgBuff, FakeNullSportPakgCount * 2, 1);
			NullSportPakgCount = 0;
		}        
		WriteDataFlash((unsigned char *)(&data), 2, 1);//写入睡眠数据
        
        uint16_t count_temp = GetDirectoryCount(1);
        uint16_t len_temp =  CheckDirectoryDataLen(count_temp-1,1);
        uint16_t len_temp2 =0;
        if(count_temp>=2) len_temp2 = CheckDirectoryDataLen(count_temp-2,1);
//        DBG1_EVT3("count_temp %d, len_temp : %d len_temp2:%d",count_temp,len_temp,len_temp2);
        if(len_temp >= 288) //单个目录长度288字节（12个小时），结束目录   //  CheckDirectoryDataLen
        {
            SetDataEndTpy(1);
        }
	}
}


void FakeSportDataProcess(uint16_t data)
{
	uint16_t Temp = 0;
	Temp = data & 0xff;		 	//高地位调换
	data = (data >> 8) + (Temp << 8);
	if (GetDirectoryState(1))//睡眠目录存在
	{
		SetDataEndTpy(1);
	}
	if(!GetDirectoryState(0))//没有计步目录
	{
		FakeSetNewDirectory(0,FakeNullSportPakgCount *5);
	}
	if(FakeNullSportPakgCount != 0)
	{
		memset(FakeNullSportPakgBuff, 0, FakeNullSportPakgCount*2);
		WriteDataFlash((unsigned char *)FakeNullSportPakgBuff, FakeNullSportPakgCount * 2, 0);
		NullSportPakgCount = 0;
	}
	WriteDataFlash((unsigned char *)(&data), 2, 0);//写入睡眠数据
}



void FakeSetSleepAndSportData(void * p_context)// 
{   
    static uint16_t cnt=0;
    static uint8_t first_in =1;
    static uint16_t i =0;
    static uint16_t sleep_data=0;
    static uint16_t sport_data=0;

//    uint16_t len =sizeof(fake_dat) / sizeof(fake_dat[0]);
//    if(i < len) //小时 *24 =天
//    {
//        switch(fake_dat[i].type)
//        {
//            case 0:
//                FakeSportDataProcess(fake_dat[i].data);
//                DBG1_EVT2("SportData, i : %d ,fake_dat[i].data : %d",i,fake_dat[i].data);
//                break;
//            case 1:
//                FakeSleepDataProcess(fake_dat[i].data);
//                DBG1_EVT2("SleepData, i : %d ,fake_dat[i].data : %d",i,fake_dat[i].data);
//                break;
//        }
//        fake_utc_seconds+=300;
//        i++;
//        DBG1_EVT1("FakeSetSleepAndSportData : i : %d ",i);
//    }

   if(cnt< (24 * 5) )
   {
       cnt ++;
       DataStore_Print("FakeSetSleepAndSportData : cnt : %d  days:%d",cnt,cnt/24);
       if(cnt%2)
       {
           for(int j = 0;j<12;j++) //1小时睡眠数据
           {
               FakeSleepDataProcess(sleep_data);
               fake_utc_seconds+=300;
               sleep_data+=2;
           }
       }
       else
       {
           for(int j = 0;j<12;j++) //1小时计步数据
           {
               FakeSportDataProcess(sport_data);
               fake_utc_seconds+=300;
               sport_data++;
           }
       }
   }

        
    else
    {
        if(first_in)
        {
            app_timer_stop(fake_data_timer_id);
//            app_timer_create(&print_data_timer_id, APP_TIMER_MODE_REPEATED, print_data_log);
//            app_timer_start(print_data_timer_id, APP_TIMER_TICKS(500, 0), NULL);
            first_in =0;
        }
        sleep_data=0;
        sport_data=0;
    }        
}

static void fake_data_timeout_timer_handler(void * p_context)
{
    APP_ERROR_CHECK(app_isched_event_put(&g_bg_scheduler, FakeSetSleepAndSportData, NULL));    
}

unsigned char TestGetSleepAndStepData(unsigned char PakgCount, uint16_t  DirectoryCount, uint16_t DataTpy, unsigned char *data)	// 
{
	uint16_t TempLen = 0;
	uint32_t DirectoryAddr = DataTpy ? (SLEEP_DIRECTORY_START_ADD) : (SPORT_DIRECTORY_START_ADD);
	uint32_t TempAdd = 0;
	uint32_t PakgLenTemp = 0;
	unsigned char StartTempDirectoryV[4] = { 0 };
	TempLen = CheckDirectoryDataLen(DirectoryCount, DataTpy);
	
	if (((PakgCount) * 16) >= TempLen)
	{
		return 1;
	}
	DirectoryCount = GetDirectoryNume(DataTpy, DirectoryCount);
	if (DirectoryCount & 0x8000)
	{
		return 1;
	}

	readData(DirectoryAddr + (DirectoryCount * 4), StartTempDirectoryV, 4);
	memcpy((unsigned char *)(&TempAdd),StartTempDirectoryV,4);
	TempAdd = TempAdd >>8;
	PakgLenTemp = TempLen - PakgCount * 16;

	if (PakgLenTemp > 16)
	{
		PakgLenTemp = 16;
	}
	readData(TempAdd + PakgCount * 16 + 6, data, PakgLenTemp);
	return 0;
}

void print_data_log(void * p_context)
{
    static uint8_t cnt_0,cnt_1 =0;
    //DBG1_EVT0("print_data_log ");
    uint8_t databuf[20]={0xff};
    uint16_t len_temp,count_temp,pack_count;
    uint32_t get_seconds;
    //获得计步目录
    count_temp = GetDirectoryCount(0);
//    DBG1_EVT3("count_temp : %d  cnt_0 :%d   cnt_1 : %d",count_temp,cnt_0,cnt_1);
    if(cnt_0 < count_temp)
    {
        len_temp = CheckDirectoryDataLen(cnt_0,0);
        get_seconds = GetDirectoryUTC_Time(cnt_0, 0);  
        
        pack_count = CEIL_DIV(len_temp, 16);
        NRF_LOG_RAW_INFO("GetDirectoryNum :%d , DirectoryLen :%04x  ,UTC :%08x  ,Pack_Count :%04x \r\n",cnt_0,len_temp,get_seconds,pack_count);
        for(int j=0;j<pack_count;j++)
        {
            databuf[0]=0x00;
            databuf[1]=0x00;
            databuf[2]=(j>>8) & 0xff;
            databuf[3]=j & 0xff;
            memset(&databuf[4],0xff,16);
            TestGetSleepAndStepData(j, cnt_0, 0, &databuf[4]);
            for(int k=4;k<20;k++)
            {
                if(k%2 ==0)
                {
                    NRF_LOG_RAW_INFO(" ");                    
                }             
                NRF_LOG_RAW_INFO("%02x",databuf[k]);
            }
            NRF_LOG_RAW_INFO("\r\n");
        } 
        cnt_0++;
        if(cnt_0 ==count_temp)
        {
            uint8_t sport_buf[400]={0};          
            w25x80_read(SPORT_DATA_START_ADD, sport_buf,400);
            for(int i=0;i<400;i++)
            {
                NRF_LOG_RAW_INFO("%02x ",sport_buf[i]);
                if((sport_buf[i-1]==0x00)&&(sport_buf[i]==0xc0)) NRF_LOG_RAW_INFO("\r\n"); 
    //                if(((i+1)%4==0)&&(i!=0))
    //                {
    //                    NRF_LOG_RAW_INFO(" ");
    //                    if((i+1)%15==0)
    //                    {
    //                       NRF_LOG_RAW_INFO("\r\n"); 
    //                    }            
    //                }
            }
            NRF_LOG_RAW_INFO("\r\n");
        }        
    }
    else
    {
        count_temp = GetDirectoryCount(1);
//        DBG1_EVT1("count_temp : %d ",count_temp);
        if(cnt_1 < count_temp)
        {
            len_temp = CheckDirectoryDataLen(cnt_1,1);
            get_seconds = GetDirectoryUTC_Time(cnt_1, 1);  
            
            pack_count = CEIL_DIV(len_temp, 16);
            NRF_LOG_RAW_INFO("GetDirectoryNum :%d , DirectoryLen :%04x  ,UTC :%08x  ,Pack_Count :%04x \r\n",cnt_1,len_temp,get_seconds,pack_count);
            for(int j=0;j<pack_count;j++)
            {
                databuf[0]=0x00;
                databuf[1]=0x00;
                databuf[2]=(j>>8) & 0xff;
                databuf[3]=j & 0xff;
                memset(&databuf[4],0xff,16);
                TestGetSleepAndStepData(j, cnt_1, 1, &databuf[4]);
                for(int k=4;k<20;k++)
                {
                    if(k%2 ==0)
                    {
                        NRF_LOG_RAW_INFO(" ");                    
                    }             
                    NRF_LOG_RAW_INFO("%02x",databuf[k]);
                }
                NRF_LOG_RAW_INFO("\r\n");
            } 
            cnt_1++;
            if(cnt_1 ==count_temp)
            {
                uint8_t sleep_buf[1000]={0};          
                w25x80_read(SLEEP_DATA_START_ADD, sleep_buf,1000);
                for(int i=0;i<1000;i++)
                {
                    NRF_LOG_RAW_INFO("%02x ",sleep_buf[i]);
                    if((sleep_buf[i-1]==0x00)&&(sleep_buf[i]==0xc0)) NRF_LOG_RAW_INFO("\r\n"); 
        //                if(((i+1)%4==0)&&(i!=0))
        //                {
        //                    NRF_LOG_RAW_INFO(" ");
        //                    if((i+1)%15==0)
        //                    {
        //                       NRF_LOG_RAW_INFO("\r\n"); 
        //                    }            
        //                }
                }
                NRF_LOG_RAW_INFO("\r\n");
            }
            
        }
        else
        {
            app_timer_stop(print_data_timer_id);
        }
    }  
}

#endif


#if CONFIG_TEST_ENABLED 
#include "app_timer.h"
APP_TIMER_DEF(print_normal_flash_data_timer_id);
void print_normal_flash_data_timer_handler(void * p_context);


bool m_flash_test_event_handler(const event_t * p_evt)
{
    static uint8_t first_in =1;    
    if (p_evt->type == EVT_TEST_CMD)
    {
        if (p_evt->test.test_event == TEST_EVT_FLASH_READ_FAKE_DATA)
        {
            DBG0_EVT0("m_flash_test_event_handler:TEST_FLASH_EVT_READ_FAKE_DATA");
            if(first_in)
            {
                app_timer_create(&print_normal_flash_data_timer_id, APP_TIMER_MODE_REPEATED, print_normal_flash_data_timer_handler);
                first_in =0;
            }
            app_timer_start(print_normal_flash_data_timer_id, APP_TIMER_TICKS(1000, 0), NULL);
        }
        else if (p_evt->test.test_event == TEST_EVT_FLASH_WRITE_FAKE_DATA)
        {

        }
    }
    return false;     
}

void print_normal_flash_data_timer_handler(void * p_context)
{
    uint8_t tempbuf[200]={0xff}; 
    static uint8_t state =0;
    switch(state)
    { 
        case 0:
            NRF_LOG_RAW_INFO("\r\n SPORT_DIRECTORY : \r\n");
            do
            {
                memset(tempbuf,0xff,100);
                w25x80_read(SPORT_DIRECTORY_START_ADD, tempbuf,100);
                for(int i=0;i<100;i++)
                {
                    NRF_LOG_RAW_INFO("%02x ",tempbuf[i]);
                    if((tempbuf[i-1]==0x00)&&(tempbuf[i]==0xc0)) 
                    {                    
                        NRF_LOG_RAW_INFO("\r\n");
                    }
                }
            }
            while((tempbuf[97] != 0xff)&&(tempbuf[98] != 0xff)&&(tempbuf[98] != 0xff));
            state =1;
            break;
            
        case 1:
            NRF_LOG_RAW_INFO("\r\n SLEEP_DIRECTORY : \r\n");
            do
            {
                memset(tempbuf,0xff,100);
                w25x80_read(SLEEP_DIRECTORY_START_ADD, tempbuf,100);
                for(int i=0;i<100;i++)
                {
                    NRF_LOG_RAW_INFO("%02x ",tempbuf[i]);
                    if((tempbuf[i-1]==0x00)&&(tempbuf[i]==0xc0)) 
                    {                    
                        NRF_LOG_RAW_INFO("\r\n");
                    }
                }
            }
            while((tempbuf[97] != 0xff)&&(tempbuf[98] != 0xff)&&(tempbuf[98] != 0xff));
            state =2;
            break;
            
        case 2:
            NRF_LOG_RAW_INFO("\r\n SPORT_DATA : \r\n");
            uint32_t static addr= SPORT_DATA_START_ADD;       
            {
                memset(tempbuf,0xff,100);
                w25x80_read(addr, tempbuf,100);
                addr+=100;
                for(int i=0;i<100;i++)
                {
                    NRF_LOG_RAW_INFO("%02x",tempbuf[i]);
                    if(i%2 == 0)
                    {
                        NRF_LOG_RAW_INFO("  ");
                    }
                    if((tempbuf[i-1]==0x00)&&(tempbuf[i]==0xc0)) 
                    {                    
                        NRF_LOG_RAW_INFO("\r\n");
                    }
                }
            }
            if((tempbuf[97] == 0xff)&&(tempbuf[98] == 0xff)&&(tempbuf[98] == 0xff))
            {
                state =3;
            }
            break;
            
        case 3:
            NRF_LOG_RAW_INFO("\r\n SLEEP_DATA : \r\n");
            uint32_t static sleep_addr= SLEEP_DATA_START_ADD;   
            {
                memset(tempbuf,0xff,100);
                w25x80_read(sleep_addr, tempbuf,100);
                sleep_addr+=100;
                for(int i=0;i<100;i++)
                {
                    NRF_LOG_RAW_INFO("%02x",tempbuf[i]);
                    if(i%2 == 0)
                    {
                        NRF_LOG_RAW_INFO("  ");
                    }
                    if((tempbuf[i-1]==0x00)&&(tempbuf[i]==0xc0)) 
                    {                    
                        NRF_LOG_RAW_INFO("\r\n");
                    }
                }
            }
            if((tempbuf[97] == 0xff)&&(tempbuf[98] == 0xff)&&(tempbuf[98] == 0xff))
            {
                state = 0;
                app_timer_stop(print_normal_flash_data_timer_id);
            }
            break;
     }
       
}
#endif



