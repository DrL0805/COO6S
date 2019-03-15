#include "m_ble_dfu.h"
#include "nrf_log.h"
#include "app_error.h"
#include "nrf_soc.h"
#include "nrf_bootloader_info.h"
#include "nrf_pwr_mgmt.h"

#include "m_debug.h"

#if CONFIG_RTT_DFU_ENABLED
#define DFU_Print(M, ...)	SEGGER_Print("DFU", M, ##__VA_ARGS__)
#else
#define DFU_Print(M, ...)
#endif

#if (BLE_DFU_ENABLED == 1)

#if (BLE_DFU_TYPE == DFU_SDK)
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
            DFU_Print("Device is preparing to enter bootloader mode.");
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            APP_ERROR_CHECK(event_send(EVT_SYSTEM_STATE, SYSTEM_STATE_DFU));
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
            DFU_Print("Device will enter bootloader mode.");
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
            DFU_Print("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
            DFU_Print("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
            APP_ERROR_CHECK(false);
            break;

        default:
            DFU_Print("Unknown event from ble_dfu_buttonless.");
            break;
    }
}


void ble_dfu_init(void)
{
    uint32_t        err_code;
    ble_dfu_buttonless_init_t   dfus_init = {0};

    // Initialize the async SVCI interface to bootloader.
//    err_code = ble_dfu_buttonless_async_svci_init();
//    APP_ERROR_CHECK(err_code);
//    
    dfus_init.evt_handler = ble_dfu_evt_handler;

    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code);
}

#elif (BLE_DFU_TYPE == DFU_MYSELF)
uint32_t ble_dfu_bootloader_start_finalize(void)
{
    uint32_t err_code;

    NRF_LOG_DEBUG("In ble_dfu_buttonless_bootloader_start_finalize\r\n");

    err_code = sd_power_gpregret_clr(0, 0xffffffff);
    VERIFY_SUCCESS(err_code);

    err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
    VERIFY_SUCCESS(err_code);

    // Signal that DFU mode is to be enter to the power management module
    nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);

    return NRF_SUCCESS;
}
#endif

#endif





