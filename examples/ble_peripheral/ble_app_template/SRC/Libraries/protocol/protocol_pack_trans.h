
#ifndef PROTOCOL_PACK_TRANS_H__
#define PROTOCOL_PACK_TRANS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

#define DATA_TYPE_STEPS                             0x0000
#define DATA_TYPE_SLEEP                             0x0001
#define DATA_TYPE_END                               0x0002

#define DATA_INTV_1US                               1
#define DATA_INTV_1MS                               2
#define DATA_INTV_1S                                3
#define DATA_INTV_10US                              4
#define DATA_INTV_10MS                              5
#define DATA_INTV_10S                               6
#define DATA_INTV_1MINUTE                           7
#define DATA_INTV(VALUE, UNIT)                      (((UNIT) << 12) + ((VALUE) & 0x0FFF))

#define DATAREQ_RESULT_SUCCESS                      0
#define DATAREQ_RESULT_INVALID_DATATYPE             1
#define DATAREQ_RESULT_INVALID_CONTENT_INDEX        2
#define DATAREQ_RESULT_INVALID_PACK_INDEX           3
#define DATAREQ_RESULT_INTERNAL_ERROR               4


#define DELETE_SUCCESS                              0                   // ɾ���ɹ�
#define DELETE_ERROR_INTERNAL                       1                   // ɾ��ʧ��(�ڲ�����)
#define DELETE_NO_PERMISSION                        2                   // ɾ��ʧ��(û��Ȩ��)

typedef union
{
    uint16_t get_data_type;                             // �������ͣ���: DATA_TYPE_STEPS
    struct 
    {
        uint16_t data_type;                             // �������ͣ���: DATA_TYPE_STEPS
        uint16_t content_count;
        uint32_t all_data_len;
    }ret_param;
}prot_content_count_evt_t;

typedef union
{
    struct
    {
        uint16_t data_type;                             // �������ͣ���: DATA_TYPE_STEPS
        uint16_t content_index;
    }get_param;
    struct
    {
        uint32_t utc_second;
        uint16_t data_len;
        uint16_t data_intv;                             // �� 4 bit Ϊ��������ĵ�λ����: DATA_INTV_1US
        uint8_t  data_unit_len;
    }ret_param;
}prot_content_info_evt_t;

typedef union
{
    uint8_t ret_result;                                 // ��������������: DATAREQ_RESULT_SUCCESS
    struct
    {
        uint16_t data_type;                             // �������ͣ���: DATA_TYPE_STEPS
        uint32_t utc_second;    
        uint16_t pack_index;
    }get_param;
}prot_package_req_evt_t;

typedef union
{
    uint8_t ret_datatype_count;
    struct
    {
        uint16_t datatype_index;
        uint16_t data_type;
        uint16_t saved_content_count_of_datatype;
        uint16_t saved_all_data_len;
    }echo_param;
}prot_auto_update_content_count_evt_t;

typedef union
{
    uint16_t call_datatype;                             // �������ͣ���: DATA_TYPE_STEPS
    uint8_t  ret_content_count;
    struct
    {
        uint16_t content_index;
        uint32_t utc_second;
        uint16_t data_len;
        uint16_t data_intv;                             // �� 4 bit Ϊ��������ĵ�λ����: DATA_INTV_1US
        uint8_t  data_unit_len;
    }echo_param;
}prot_auto_update_content_info_evt_t;

typedef struct
{
    uint8_t echo_result;                                // ɾ�����: DELETE_SUCCESS
}prot_del_all_evt_t;

typedef union
{
    uint8_t call_datatype;
    struct
    {
        uint16_t data_type;
        uint8_t  result;                                // ɾ�����: DELETE_SUCCESS
    }echo_param;
}prot_del_datatype_evt_t;

typedef union
{
    struct
    {
        uint16_t data_type;
        uint32_t utc_second;
    }call_param;

    struct
    {
        uint16_t data_type;
        uint32_t utc_second;
        uint8_t  result;                                // ɾ�����: DELETE_SUCCESS
    }echo_param;
}prot_del_content_utc_evt_t;

typedef union
{
    prot_content_count_evt_t                content_count;
    prot_content_info_evt_t                 content_info;
    prot_package_req_evt_t                  package_req;
    prot_auto_update_content_count_evt_t    auto_update_content_count;
    prot_auto_update_content_info_evt_t     auto_update_content_info;
    prot_del_all_evt_t                      del_all;
    prot_del_datatype_evt_t                 del_datatype;
    prot_del_content_utc_evt_t              del_content_utc;
}prot_pack_trans_evt_t;



#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_PACK_TRANS_H__ */


