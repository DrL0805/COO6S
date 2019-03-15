
#ifndef PROTOCOL_DEV_INFO_H__
#define PROTOCOL_DEV_INFO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)


typedef struct
{
    uint16_t        name_flag;
    uint8_t         name_len;
    char            name[11];
}prot_prod_name_evt_t;

typedef struct
{
    uint8_t         fw_version[3];   // [0] 主版本号 1 ~ 99, [1] 次版本号 1 ~ 99, [2] 测试版本号 1 ~ 99
}prot_prod_ver_evt_t;

typedef struct
{
    uint8_t         date[3];        // [0] 年, [1] 月, [2] 日
}prot_prod_date_evt_t;

typedef struct
{
    uint8_t         percent;        // 电量百分比值
}prot_prod_soc_evt_t;

typedef struct
{ 
    uint8_t         type;           //电池类型
    uint8_t         percent;        // 电量百分比值
}prot_prod_soc_new_evt_t;

typedef struct
{ 
    uint16_t         batt_vol;           //电池类型
}prot_prod_soc_batt_vol_t;

typedef enum
{
    AUTH_REQ_ASK            = 0U,
    AUTH_REQ_FORCE          = 1U,
}auth_req_t;

typedef enum
{
    AUTH_STATE_PASS         = 1U,
    AUTH_STATE_FAIL         = 0U,
}auth_state_t;

typedef union
{
    uint8_t         call_auth_req;          // @ auth_req_t
    uint8_t         set_auth_state;         // @ auth_state_t
}prot_prod_auth_evt_t;

typedef struct
{
    uint8_t msb;
    uint8_t sn[10];
}prot_prod_sn_evt_t;

typedef struct
{
    uint8_t msb;
    uint8_t sn[9];
}prot_prod_manager_sn_evt_t;

typedef struct
{
    uint8_t mac[6];
}prot_prod_mac_evt_t;

typedef struct
{
    uint8_t pro_num[5];
    uint8_t pro_type[5];
}prot_prod_pro_num_evt_t;

typedef struct
{
    uint8_t         system_name[11];
}prot_prod_phone_sys_evt_t;

typedef struct
{
    uint8_t         system_model[11];
}prot_prod_phone_mod_evt_t;

typedef struct
{
    uint8_t         utc_bytes[6];           // 年月日时分秒
    char            timezone_int;           // 时区整数部分，-12~12
    uint8_t         timezone_decimal;       // 时区小数部分
    uint8_t         week;                   // 星期 0~6
}prot_date_evt_t;

typedef union
{
    uint8_t         utc_bytes[6];           // 年月日时分秒
    char            timezone_int;           // 时区整数部分，-12~12
    uint8_t         timezone_decimal;       // 时区小数部分
    uint8_t         week;                   // 星期 0~6
    uint16_t        city_code;              // 城市代码
}prot_sec_city_date_evt_t;

typedef struct
{
    uint8_t         week_list : 7;          // bit 0 ~ bit 6 ????чǚ??чǚ1
    uint8_t         repeat    : 1;          // ???ё????ì? 0 ??????
}prot_weeks_t;

typedef union
{
    prot_weeks_t    weeks;
    uint8_t         data;
}prot_weeks_u;

typedef struct
{
    uint8_t         num;                    // ???? 1 ~ 10
    uint8_t         hours;                  // ??????: ?, BCD ??
    uint8_t         minute;                 // ??????: ??, BCD ??
    prot_weeks_u    weeks;                  // чǚ??
    uint8_t         enable;                 // ???b?    
}prot_alarm_clk_param_t;

typedef union
{
    prot_alarm_clk_param_t      set_param;
    prot_alarm_clk_param_t      ret_param;
    uint8_t                     get_num;
    uint8_t                     echo_alram_clock_index;
}prot_alarm_clk_evt_t;


typedef struct
{
    uint8_t                      reset_nums;
    uint32_t                     reset_reason;
    uint8_t                      reset_line[2];
    union
    {
        uint32_t                 reset_wdt_addr;
        uint8_t                  reset_str[4];
    }reset_info;        
    
}prot_reset_info_evt_t;


typedef struct
{
    uint8_t  reset_type;
    uint16_t reset_cnt;
    uint32_t reset_utc;
}reset_info1_t;

typedef struct
{
    uint8_t  debug_info[6]; 
    uint32_t err_code;    
}reset_info2_t;

typedef struct
{
   uint8_t  index;
   union
   {
       reset_info1_t reset_info1;
       reset_info2_t reset_info2;
   }reset_info_pack;       
}prot_reset_tot_info_evt_t;

typedef struct
{
    uint8_t key_info[4];
}prot_key_info_evt_t;

typedef union
{
    prot_prod_name_evt_t        prod_name;
    prot_prod_ver_evt_t         prod_ver;
    prot_prod_date_evt_t        prod_date;
    prot_prod_soc_evt_t         prod_soc;
    prot_prod_soc_new_evt_t     prod_soc_new;
    prot_prod_soc_batt_vol_t    prod_batt_vol;
    prot_prod_auth_evt_t        prod_auth;
    prot_prod_sn_evt_t          prod_sn;
    prot_prod_manager_sn_evt_t  prod_manager_sn;
    prot_prod_mac_evt_t         prod_mac;
    prot_prod_pro_num_evt_t     prod_pro_num;
    prot_prod_phone_sys_evt_t   prod_phone_sys;
    prot_prod_phone_mod_evt_t   prod_phone_mod;

    prot_date_evt_t             date;
    prot_sec_city_date_evt_t    sec_city_date;
    prot_alarm_clk_evt_t        alarm_clk;
    prot_reset_info_evt_t       reset_info;
    prot_key_info_evt_t         key_reset_info;
    prot_reset_tot_info_evt_t   reset_total_info;
}prot_dev_info_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_DEV_INFO_H__ */


