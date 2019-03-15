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
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_strerror.h"
#include "sr3_config.h"
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
#include "nrf_sdm.h"
#endif

#ifndef BOOTLOADER 
#include "m_database.h"
#include "includes.h"
#endif


#if CONFIG_DEBUG_ENABLED
#define Error_Print(M, ...)    SEGGER_Print("Error", M, ##__VA_ARGS__)
#else
#define Error_Print(M, ...)
#endif

extern uint32_t write_flash_reset_info(uint32_t error_code);


/*lint -save -e14 */
/**
 * Function is implemented as weak so that it can be overwritten by custom application error handler
 * when needed.
 */
__WEAK void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    
#ifndef BOOTLOADER 
    
    error_info_t * m_info = (error_info_t *)info;
    
    char *str = strrchr ((char *) m_info->p_file_name, '.') - 4;

    
    reset_total_info_t reset_total_info;
    memset(&reset_total_info,0x00,sizeof(reset_total_info));
    system_database.err_reset_cnt = system_database.err_reset_cnt + 1;
    reset_total_info.reset_cnt  = system_database.err_reset_cnt;
    reset_total_info.reset_type = 0x01;
    reset_total_info.reset_utc = system_database.utc_seconds;
    reset_total_info.err_code = m_info->err_code;
    memcpy(&reset_total_info.debug_info,str,4);
    reset_total_info.debug_info[4] = (m_info->line_num >> 8) & 0xff;
    reset_total_info.debug_info[5] = m_info->line_num & 0xff;
    
    
    ret_code_t    err = w25x80_erase_sector(0x0006000);
   
    if (NRF_SUCCESS == err)
    {
        err = w25x80_write(0x0006000, (uint8_t *)&reset_total_info, sizeof(reset_total_info));
    }
#endif
    
    __disable_irq();
    NRF_LOG_FINAL_FLUSH();

#ifdef BOOTLOADER 
    NVIC_SystemReset();
#else 
    
#ifdef CONFIG_DEBUG_ENABLED
//    NRF_LOG_ERROR("Fatal error");

    switch (id)
    {
#if defined(SOFTDEVICE_PRESENT) && SOFTDEVICE_PRESENT
        case NRF_FAULT_ID_SD_ASSERT:
            Error_Print("SOFTDEVICE: ASSERTION FAILED");
            break;
        case NRF_FAULT_ID_APP_MEMACC:
            Error_Print("SOFTDEVICE: INVALID MEMORY ACCESS");
            break;
#endif
        case NRF_FAULT_ID_SDK_ASSERT:
        {
            assert_info_t * p_info = (assert_info_t *)info;
            Error_Print("ASSERTION FAILED at %s:%u",
                          p_info->p_file_name,
                          p_info->line_num);
            break;
        }
        case NRF_FAULT_ID_SDK_ERROR:
        {
            error_info_t * p_info = (error_info_t *)info;
            Error_Print("ERROR %u [%s] at %s:%u\r\nPC at: 0x%08x",
                          p_info->err_code,
                          nrf_strerror_get(p_info->err_code),
                          p_info->p_file_name,
                          p_info->line_num,
                          pc);
             Error_Print("End of error report");
            break;
        }
        default:
            Error_Print("UNKNOWN FAULT at 0x%08X", pc);
            break;
    }
    app_error_save_and_stop(id, pc, info);
#else
    write_flash_reset_info(0xDE);

    NVIC_SystemReset();
#endif // CONFIG_DEBUG_ENABLED

#endif //BOOTLOADER

#ifndef DEBUG
//    NRF_LOG_WARNING("System reset");
#ifndef BOOTLOADER 
    
    char *str = strrchr ((char *)p_file_name, '.') - 4;

    
    reset_total_info_t reset_total_info;
    memset(&reset_total_info,0x00,sizeof(reset_total_info));
    system_database.err_reset_cnt = system_database.err_reset_cnt + 1;
    reset_total_info.reset_cnt  = system_database.err_reset_cnt;
    reset_total_info.reset_type = 0x01;
    reset_total_info.reset_utc = system_database.utc_seconds;
    reset_total_info.err_code = error_code;
    memcpy(&reset_total_info.debug_info,str,4);
    reset_total_info.debug_info[4] = (line_num >> 8) & 0xff;
    reset_total_info.debug_info[5] = line_num & 0xff;
    
    
    ret_code_t    err = w25x80_erase_sector(0x0006000);
   
    if (NRF_SUCCESS == err)
    {
        err = w25x80_write(0x0006000, (uint8_t *)&reset_total_info, sizeof(reset_total_info));
    }
#endif
    NVIC_SystemReset();
#else
//    app_error_save_and_stop(id, pc, info);
#endif // DEBUG
}
/*lint -restore */
