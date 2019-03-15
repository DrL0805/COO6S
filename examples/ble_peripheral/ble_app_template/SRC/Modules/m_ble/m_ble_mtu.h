#ifndef __BLE_MTU_H__
#define __BLE_MTU_H__

#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf.h"
#include "ble.h"
#include "app_error.h"
//#include "nrf_log.h" 


//// <i> Requested BLE GAP data length to be negotiated.

//#ifndef NRF_SDH_BLE_GAP_DATA_LENGTH
//#define NRF_SDH_BLE_GAP_DATA_LENGTH 251
//#endif

//// <o> NRF_SDH_BLE_PERIPHERAL_LINK_COUNT - Maximum number of peripheral links. 
//#ifndef NRF_SDH_BLE_PERIPHERAL_LINK_COUNT
//#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 1
//#endif

//// <o> NRF_SDH_BLE_CENTRAL_LINK_COUNT - Maximum number of central links. 
//#ifndef NRF_SDH_BLE_CENTRAL_LINK_COUNT
//#define NRF_SDH_BLE_CENTRAL_LINK_COUNT 0
//#endif

//// <o> NRF_SDH_BLE_TOTAL_LINK_COUNT - Total link count. 
//// <i> Maximum number of total concurrent connections using the default configuration.

//#ifndef NRF_SDH_BLE_TOTAL_LINK_COUNT
//#define NRF_SDH_BLE_TOTAL_LINK_COUNT 1
//#endif

//// <o> NRF_SDH_BLE_GAP_EVENT_LENGTH - GAP event length. 
//// <i> The time set aside for this connection on every connection interval in 1.25 ms units.

//#ifndef NRF_SDH_BLE_GAP_EVENT_LENGTH
//#define NRF_SDH_BLE_GAP_EVENT_LENGTH 6
//#endif

//// <o> NRF_SDH_BLE_GATT_MAX_MTU_SIZE - Static maximum MTU size. 
//#ifndef NRF_SDH_BLE_GATT_MAX_MTU_SIZE
//#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 247
//#endif

void mtu_init(void);

#endif

