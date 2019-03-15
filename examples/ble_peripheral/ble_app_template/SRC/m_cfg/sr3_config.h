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

#ifndef _SR3_CONFIG_H
#define _SR3_CONFIG_H

#include "nordic_common.h"

// SoC configuration:
#define APP_TIMER_PRESCALER                     0
#define IS_IO_VALID(io)                         (((io) & ~0x1F) == 0)

// Audio codecs:
//#define CONFIG_AUDIO_CODEC_ADPCM                1
//#define CONFIG_AUDIO_CODEC_BV32FP               2
//#define CONFIG_AUDIO_CODEC_OPUS                 3
//#define CONFIG_AUDIO_CODEC_SBC                  4



// Connection bandwidth tag:
#define CONFIG_CONN_CFG_HIGH_BW_TAG             1

// Fetch board configuration:
#if defined(CONFIG_BOARD_NRF52832_PCA20023)
#elif defined(CONFIG_BOARD_NRF52832_D001)
# include "sr3_config_nrf52832_PH000.h"
#else
# define CONFIG_BOARD "UNKNOWN"
# error "CONFIG_BOARD_xxx is not defined!"
#endif





// Obtain stack guard configuration.
#if CONFIG_STACK_GUARD_ENABLED
# define CONFIG_STACK_SIZE_GUARD    (1 << CONFIG_STACK_GUARD_CONFIG_SIZE)
#else /* !NRF_STACK_GUARD_ENABLED */
# define CONFIG_STACK_SIZE_GUARD    0
#endif /* NRF_STACK_GUARD_ENABLED */

#define CONFIG_STACK_SIZE   (CONFIG_STACK_SIZE_BASE + CONFIG_STACK_SIZE_AUDIO + CONFIG_STACK_SIZE_GUARD)

// Check if dynamic BLE address management is needed.
#define CONFIG_BLE_DYNAMIC_ADDR_ENABLED (CONFIG_SEC_BOND && (CONFIG_MAX_BONDS > 1) && CONFIG_CHANGE_ADDRESS)

// Check if peer rank management is needed.
#define CONFIG_BLE_PEER_RANK_ENABLED    (CONFIG_SEC_BOND && (CONFIG_MAX_BONDS > 1))

// Check if RNG monitor is needed.
#define CONFIG_RNG_MONITOR_ENABLED      (CONFIG_SEC_LESC_PAIRING || CONFIG_NFC_ENABLED)

// Fetch HID configuration:
//#include "sr3_config_hid.h"

// Fetch IR configuration:
//#include "sr3_config_ir.h"

// Fetch buzzer configuration:
//#include "sr3_config_buzzer.h"

#endif /* _SR3_CONFIG_H */
