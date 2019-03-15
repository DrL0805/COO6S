#ifndef __M_BLE_DFU_H__
#define __M_BLE_DFU_H__

#include "ble_dfu.h"
#include "sr3_config.h"
#include "sdk_config.h"


#if (BLE_DFU_ENABLED == 1)

#if (BLE_DFU_TYPE == DFU_SDK)
void ble_dfu_init(void);
#elif (BLE_DFU_TYPE == DFU_MYSELF)
uint32_t ble_dfu_bootloader_start_finalize(void);
#endif

#endif

#endif


