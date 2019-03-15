
#ifndef TEST_MANUF_H__
#define TEST_MANUF_H__

#include <stdint.h>


#define TEST_HARDWARE_PCBA 		0x01
#define TEST_HARDWARE_JIXIN 	0x02




#define TEST_PDU_MIN_BYTE	6	

#define TEST_PDU_HEAD	0x25
#define TEST_PDU_VER	0X01
#define TEST_PDU_XOR	0x61		// 临时，协议结尾固定

#define TEST_PDU_STOP_TEST			0x00	// 停止测试
#define TEST_PDU_TEST_METHOD_1 		0X01	// 测试方法1
#define TEST_PDU_ERR_CODE_NO_ERR	0X00	// 错误码无错误

/* 模块代号定义 */ 

#define TEST_PDU_MODULE_NULL 			0X00
#define TEST_PDU_MODULE_EXT_FLASH		0x01
#define TEST_PDU_MODULE_GSENSOR 	    0x02		 // C001S G-sensor
#define TEST_PDU_MODULE_GYRO		    0x03         //陀螺仪
#define TEST_PDU_MODULE_GEOMAGNETISM    0x04          //地磁
#define TEST_PDU_MODULE_VDD				0x05
#define TEST_PDU_MODULE_MOVT_CONNECTED	0x06
#define TEST_PDU_MODULE_LED				0x07
#define TEST_PDU_MODULE_MOTOR			0x08
#define TEST_PDU_MODULE_KEY				0x09
#define TEST_PDU_MODULE_BLE				0x0A
#define TEST_PDU_MODULE_MOVT_RUN		0x0B
#define TEST_PDU_MODULE_CURRENT			0x0C
#define TEST_PDU_MODULE_OLED			0x0D
#define TEST_PDU_MODULE_FONT_LIB		0x0E
#define TEST_PDU_MODULE_CHARGING		0x0F
#define TEST_PDU_MODULE_BATTERY_PATROL	0x10
#define TEST_PDU_MODULE_HRS		        0x11       //心率
#define TEST_PDU_MODULE_UV		        0x18       //紫外线


typedef enum
{
	SN_CMD_BURN = 0x01,			// 请求烧录
	SN_CMD_FEEDBACK = 0x02,		// 返回烧录结果
	SN_CMD_READ  = 0x03,			// 读取SN码
}SN_CMD_TYPE_M;

typedef enum
{
	TEST_PDU_CMD_ACK = 0x01,
	TEST_PDU_CMD_REQUEST = 0x02,
	TEST_PDU_CMD_CTRL  = 0x11,
	TEST_PDU_CMD_FEEDBACK = 0x12,
	TEST_PDU_CMD_BLE_NAME = 0x13,
	TEST_PDU_CMD_OTHER = 0X14,
    TEST_PDU_CMD_SN_BURN = 0X15
}TEST_PDU_CMD_TYPE_M;

typedef struct
{
	uint8_t 				Head;					
	uint8_t 				ProVer;					
	uint8_t					PackNum;			
	uint8_t					PackLen;				
	TEST_PDU_CMD_TYPE_M		CmdType;				
	uint8_t 				CmdData[255];			
	uint8_t 				Xor;					
	
	uint8_t 				Len;	

	uint8_t 				LastPackNum;
}TEST_PROTOCOL_T;


extern TEST_PROTOCOL_T		TEST_PDU;

void TPDU_StartTestSend(uint8_t ModuleCodeName, uint8_t Sub_ModuleCodeName,uint8_t TestMethod);
void TPDU_RequestTestSend(uint8_t HardWare);
uint32_t TPDU_ProtocolResolver(uint8_t* DataBuf, uint8_t DataLen);
void TPDU_SNBurnHandler(uint8_t len,uint8_t *data);




void test_manuf_init(void);

bool test_manuf_set_ble_addr(uint8_t * p_addr);
bool test_manuf_set_ble_name(void);

#endif /* TEST_MANUF_H__ */


