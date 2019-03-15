#ifndef __M_FLASH_ADDR_H__
#define __M_FLASH_ADDR_H__

#define MAC_ADDR        0x00010000 // MAC��ȡ��ַ
#define CORE_ID_ADDR    0x00010006 // ��о��ȡ��ַ
#define CUS_ADDR        0x00010086 // �ɱ�sn��ȡ��ַ
#define PROJ_ID_ADDR    0x00010107 // ��Ŀ��Ŷ�ȡ��ַ

#define RESET_ADDR      0x00014000 // ���渴λ��Ϣ(Ԥ��4KB == 0x1000)

#define SPORT_ORIGIN_ADDR				(RESET_ADDR + 4096)

#define SPORT_TIME_START_ADD         	(0x40000)	// �˶�ʱ�俪ʼ��ַ
#define SPORT_DIRECTORY_START_ADD       (0x41000)	// �˶�Ŀ¼��ʼ��ַ
#define SPORT_DATA_START_ADD            (0x42000)	// �˶����ݿ�ʼ��ַ

#define SPORT_DIRECTORY_END_ADD         (0x41000 + 0x1000)	// �˶�Ŀ¼������ַ
#define SPORT_DATA_END_ADD              (0x42000 + 0x1000)	// �˶����ݽ�����ַ

#define SLEEP_TIME_START_ADD            (0x60000)		// ˯��ʱ�俪ʼ��ַ
#define SLEEP_DIRECTORY_START_ADD       (0x61000)		// ˯��Ŀ¼��ʼ��ַ
#define SLEEP_DATA_START_ADD            (0x62000)		// ˯�����ݿ�ʼ��ַ


#define SLEEP_DIRECTORY_END_ADD         (0x61000 + 0x1000)	// ˯��Ŀ¼������ַ
#define SLEEP_DATA_END_ADD              (0x62000 + 0x1000)	// ˯�����ݽ�����ַ


/******************************�Զ���******************************/
#define BODY_DATA_SIZE					(8)
#define BODY_DATA_MAX_SIZE				(1536)	//1536 * 8 = 12KB��Ԥ��12KB
#define BODY_TEMP_ORIGIN_ADDR			(0x0)										// �����������ݵ���ʼ�����������ַ�ڻ������ı��ݵ�ַ
#define BODY_TEMP_DATA_HEAD_ADDR		(BODY_TEMP_ORIGIN_ADDR + 0x1000)				// ��������Ԥ������������ʼ��ַ
#define BODY_TEMP_DATA_TAIL_ADDR		(BODY_TEMP_DATA_HEAD_ADDR + 12 * 1024)	// ��������Ԥ���������Ľ�����ַ��Ԥ��12KB


#define ENVIR_DATA_SIZE					(15)	
#define ENVIR_DATA_MAX_SIZE				(819)		//819 * 15 = 11.99KB 
#define ENVIR_ORIGIN_ADDR				((((BODY_TEMP_DATA_TAIL_ADDR - 1) >> 12) + 1) << 12)					// ���ڻ�����������ʼ��ַ�������ַ�ڻ������ı��ݵ�ַ
#define ENVIR_DATA_HEAD_ADDR			(ENVIR_ORIGIN_ADDR + 0x1000)					// ��������Ԥ������������ʼ��ַ
#define ENVIR_DATA_TAIL_ADDR			(ENVIR_DATA_HEAD_ADDR + 12 * 1024)		// ��������Ԥ���������Ľ�����ַ��Ԥ��12KB


#endif
