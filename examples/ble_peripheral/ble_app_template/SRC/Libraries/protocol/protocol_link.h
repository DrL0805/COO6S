
#ifndef PROTOCOL_LINK_H__
#define PROTOCOL_LINK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef enum
{
    CONN_INTV_CONFIG_FAST,                  // 0，直接加快连接间隔，不断开
    CONN_INTV_CONFIG_RENEW,                 // 1，恢复连接间隔，不断开
    CONN_INTV_CONFIG_DISCONN_FAST,          // 2，先断开，回连后加快连接间隔
    CONN_INTV_CONFIG_DISCONN_RENEW,         // 3，先断开，回连后恢复连接间隔
}prot_link_conn_intv_config_t;

typedef union
{
    uint8_t config;                         // @ prot_link_conn_intv_config_t
}prot_link_conn_intv_evt_t;

typedef union
{
    uint8_t enabled;                        // 1，断开时提醒 0，断开时不提醒
}prot_link_linkloss_remind_enable_evt_t;

typedef union
{
    uint8_t adv_name_str[11];                        // adv name
}prot_link_adv_name_evt_t;


typedef union
{
    prot_link_adv_name_evt_t                 adv_name;
    prot_link_conn_intv_evt_t                conn_intv;
    prot_link_linkloss_remind_enable_evt_t   linkloss_remind_enable;
}prot_link_evt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_LINK_H__ */


