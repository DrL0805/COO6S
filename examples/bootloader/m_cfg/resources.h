/**
 * Copyright (c) 2016 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/**
 *
 * @defgroup MOD_HW Hardware configuration
 * @{
 * @ingroup other
 * @brief Shared hardware resource configuration.
 */
#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "nrf_pwr_mgmt.h"
#include "nrf_twi_mngr.h"
#include "app_isched.h"
#include "sr3_config.h"
#define LIS3DH_TWI_ADDRESS          (0x19)  /**< TWI address of the LIS3DH Accelerometer */
#define SX1509_TWI_ADDRESS          (0x3E)  /**< TWI address of the SX1509 I/O expander */
#define ICM20608_TWI_ADDRESS        (0x68)  /**< TWI address of the ICM20608 IMU */

#if NRF_PWR_MGMT_ENABLED
#define SHUTDOWN_PRIORITY_EARLY         0
#define SHUTDOWN_PRIORITY_DEFAULT       1
#define SHUTDOWN_PRIORITY_LATE          2
#define SHUTDOWN_PRIORITY_STATISTICS    3
#define SHUTDOWN_PRIORITY_FINAL         4

#if (NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT != 5)
#error "Unsupported NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT value!"
#endif /* (NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT != 5) */
#endif /* NRF_PWR_MGMT_ENABLED */

#if NRF_SDH_BLE_ENABLED
#define SOC_OBSERVER_PRIORITY_DEFAULT   0
#define SOC_OBSERVER_PRIORITY_LOW       1

#if (NRF_SDH_SOC_OBSERVER_PRIO_LEVELS != 2)
#error "Unsupported NRF_SDH_SOC_OBSERVER_PRIO_LEVELS value!"
#endif /* (NRF_SDH_SOC_OBSERVER_PRIO_LEVELS != 2) */
#endif /* NRF_SDH_BLE_ENABLED */

#if NRF_SDH_BLE_ENABLED
#define BLE_OBSERVER_PRIORITY_HIGH      0
#define BLE_OBSERVER_PRIORITY_DEFAULT   1
#define BLE_OBSERVER_PRIORITY_LOW       2

#if (NRF_SDH_BLE_OBSERVER_PRIO_LEVELS != 4)
#error "Unsupported NRF_SDH_BLE_OBSERVER_PRIO_LEVELS value!"
#endif /* (NRF_SDH_BLE_OBSERVER_PRIO_LEVELS != 3) */
#endif /* NRF_SDH_BLE_ENABLED */

#if CONFIG_BUTTON_ENABLED

#include "m_drv_btn_action.h"

typedef struct
{
    uint16_t                button_id;
    drv_btn_act_evt_type_t  button_action;
    uint16_t                button_event;
}button_action_event_map_t;

#define BUTTON_3H 1 // ??? CONFIG_IO_BUTTON_0
#define BUTTON_6H 0 // ??? CONFIG_IO_BUTTON_1
#define BUTTON_9H 2 // ??? CONFIG_IO_BUTTON_2

extern button_action_event_map_t common_button_action_event_maps[];
extern button_action_event_map_t normal_button_action_event_maps[];
extern button_action_event_map_t low_power_button_action_event_maps[];
extern button_action_event_map_t green_mode_button_action_event_maps[];
extern button_action_event_map_t photo_req_button_action_event_maps[];
extern button_action_event_map_t auth_ok_button_action_event_maps[];
extern button_action_event_map_t find_phone_button_action_event_maps[];
extern button_action_event_map_t reject_call_button_action_event_maps[];
extern button_action_event_map_t test_mode_button_action_event_maps[];


#endif // #if CONFIG_BUTTON_ENABLED

typedef enum
{
	BT_STATE_DISABLED = 0,
	BT_STATE_ADVERTISING,
	BT_STATE_CONNECTED,
}bt_state_t;


typedef struct
{
	uint8_t      system_state;      // @system_state_e
    uint8_t      ble_conn_state;    // @system_ble_conn_state_event_e
    uint8_t      batt_level;        // 电池电量，0 ~ 100 %
    uint32_t     data;
}system_params_t;

typedef struct
{
	uint16_t     button_event;	
}botton_params_t;

typedef struct
{
	uint32_t	 app_recv_event;	// @app_recv_event_e
	uint32_t	 prot_execution;	// @prot_execution_event_e

}coms_params_t;

typedef struct
{
	uint8_t 	type;
	uint8_t 	rate_khz;
	uint16_t	conn_id;
	uint16_t	peer_id;

}atvv_params_t;

typedef struct
{
	uint32_t	 test_event;		// @test_event_e
	uint32_t	 test_data;

}test_params_t;


extern system_params_t system_params;
extern botton_params_t botton_params;
extern coms_params_t coms_params;
extern atvv_params_t atvv_params;
extern test_params_t test_params;


extern app_isched_t g_fg_scheduler; /**< Scheduler for foreground tasks. */
extern app_isched_t g_bg_scheduler; /**< Scheduler for background tasks. */

extern nrf_twi_mngr_t const * const g_twi_mngr;        /**< TWI manager. */
extern nrf_drv_twi_config_t const g_twi_bus_config[2]; /**< TWI buses configuration. */

#endif /* __RESOURCES_H__ */
/** @} */
