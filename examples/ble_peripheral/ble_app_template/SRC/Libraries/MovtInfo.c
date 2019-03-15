#include "AD_DataStore.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "nrf_delay.h"
#include "nrf_error.h"
#include "m_drv_w25x80.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

uint32_t getMAC(unsigned char * data)//读取mca地址
{
    uint32_t err_code;
    err_code = w25x80_read(MAC_ADDR, data, 6);
    return err_code;
}



//typedef struct manager_sn
//{
//   uint8_t  manager_name[3];
//   uint8_t  sec_manager_name[3];
//   uint8_t  brand_name[3];
//   uint8_t  movt_type[3];
//   uint8_t  pes[4];
//}MANAGER_SN;

//void get_manager_sn(uint8_t data,uint8_t len)
//{
//       uint8_t lenovo_sn[18]={0,0,18,0,0,1,0,0,0,'C','0','0','1','S',0,0,0,0};
//}


//unsigned char setCusID(unsigned char * data,unsigned char len)//写入成表SN
//{
//    unsigned int last_bit_addr = 0x00010106;//上锁地址
//    unsigned char lock_state = 0;
//    unsigned char lock = 0x55;
//    unsigned int this_CUS_ADDR = CUS_ADDR;

//    w25x80_read(last_bit_addr, &lock_state, 1);//读锁状态
//    if (lock_state != 0x55)
//    {
//        w25x80_write(this_CUS_ADDR, data, len);//写SN
//        w25x80_write(last_bit_addr, &lock, 1);//上锁
//        return 0x00;//
//    }
//    else
//    {
//        return 0xff;//
//    }
//}

void get_sn(unsigned char * data,unsigned char len)//读机芯SN
{
    w25x80_read(SN_ADDR, data, len);
}




