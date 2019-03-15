#ifndef __M_COMS_BLE_H__
#define __M_COMS_BLE_H__

#include <stdbool.h>
#include <sdk_errors.h>

extern uint16_t m_usr_conn_handle;

ret_code_t m_coms_ble_init(bool erase_bonds);

ret_code_t m_coms_ble_advertise(bool bondable);

ret_code_t m_coms_ble_shutdown(void);

bool m_coms_ble_bond_stored(void);

void m_coms_ble_disable(void);

ret_code_t m_coms_ble_enable(void);

void advertising_start(bool erase_bonds);

#endif

