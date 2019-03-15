#ifndef __M_BLE_ANCS_C_H__
#define __M_BLE_ANCS_C_H__

#include "nrf_ble_gatts_c.h"
#include "nrf_ble_ancs_c.h"
#include "nrf_pwr_mgmt.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "peer_manager.h"
#include "ble_db_discovery.h"
#include "nrf_ble_ancs_c.h"

//#include "nrf_log.h"
//#include "nrf_log_ctrl.h"
//#include "nrf_log_default_backends.h"



extern uint16_t                 m_cur_ancs_conn_handle;
extern ble_ancs_c_t             m_ancs_c;

void ble_ancs_c_sec_req_timer_init(void);

void ble_ancs_c_sec_req_timer_start(uint16_t conn_handle);

void ble_ancs_c_sec_req_timer_stop(uint16_t conn_handle);

void ble_ancs_c_services_init(void);

void db_disc_handler(ble_db_discovery_evt_t * p_evt);

void db_disc_start(uint16_t conn_handle);

void ble_ancs_c_timer_init(void);

void ble_ancs_c_attr_req_timer_start(void);

void ble_ancs_c_attr_req_timer_stop(void);

bool m_coms_ble_ancs_c_reject_callin(void);
#endif

