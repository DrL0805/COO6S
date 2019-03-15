/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "sr3_config.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "drv_board.h"

#define ASSERT_SIGNAL(pin, act_low, configure)          \
do {                                                    \
    if (IS_IO_VALID((pin)))                             \
    {                                                   \
        if ((act_low))                                  \
        {                                               \
            nrf_gpio_pin_clear((pin));                  \
        }                                               \
        else                                            \
        {                                               \
            nrf_gpio_pin_set((pin));                    \
        }                                               \
                                                        \
        if ((configure))                                \
        {                                               \
             nrf_gpio_cfg_output((pin));                \
        }                                               \
    }                                                   \
} while (0)

#define DEASSERT_SIGNAL(pin, act_low, configure)        \
do {                                                    \
    if (IS_IO_VALID((pin)))                             \
    {                                                   \
        if ((act_low))                                  \
        {                                               \
            nrf_gpio_pin_set((pin));                    \
        }                                               \
        else                                            \
        {                                               \
            nrf_gpio_pin_clear((pin));                  \
        }                                               \
                                                        \
        if ((configure))                                \
        {                                               \
             nrf_gpio_cfg_output((pin));                \
        }                                               \
    }                                                   \
} while (0)

#define PIN_MASK(pin)   (IS_IO_VALID(pin) ? (1u << (pin)) : 0)

#if CONFIG_PWR_MGMT_ENABLED
/* Pins which should retain configuration during System OFF state. */
static const uint32_t m_protected_pins = 0
    | PIN_MASK(CONFIG_IO_BUTTON_WAKEUP)
    | PIN_MASK(CONFIG_IO_LED_0)
    | PIN_MASK(CONFIG_IO_LED_1)
    | PIN_MASK(CONFIG_IO_ADC_ENABLE_PIN)
    | PIN_MASK(CONFIG_IO_FLASH_SPI_CS_PIN)
    | PIN_MASK(CONFIG_IO_FLASH_SPI_SCK_PIN)
    | PIN_MASK(CONFIG_IO_FLASH_SPI_MOSI_PIN)
    | PIN_MASK(CONFIG_IO_FLASH_SPI_MISO_PIN)
    | PIN_MASK(CONFIG_IO_GSENSOR_SPI_CS_PIN)
    | PIN_MASK(CONFIG_IO_GSENSOR_SPI_MOSI_PIN)
    | PIN_MASK(CONFIG_IO_GSENSOR_SPI_MISO_PIN)
    | PIN_MASK(CONFIG_IO_GSENSOR_SPI_SCK_PIN)
    | 0;

bool drv_board_wakeup_prepare(bool wakeup)
{
    //DBG0_EVT1("Func: %s", __func__);

    unsigned int i;


    /* Make sure that all peripherals with dynamic power management are off. */
    DEASSERT_SIGNAL(CONFIG_IO_LED_0,            CONFIG_LED_ACT_LOW_MASK & (1 << 0), false);
    DEASSERT_SIGNAL(CONFIG_IO_LED_1,            CONFIG_LED_ACT_LOW_MASK & (1 << 1), false);




    /* Disable all unused pins. */
    for (i = 0; i < 32; i++)
    {
        if (m_protected_pins & (1ul << i))
        {
            continue;
        }

        NRF_GPIO->PIN_CNF[i] = ((GPIO_PIN_CNF_SENSE_Disabled    << GPIO_PIN_CNF_SENSE_Pos) \
                             | (GPIO_PIN_CNF_DRIVE_S0S1         << GPIO_PIN_CNF_DRIVE_Pos) \
                             | (GPIO_PIN_CNF_PULL_Disabled      << GPIO_PIN_CNF_PULL_Pos)  \
                             | (GPIO_PIN_CNF_INPUT_Disconnect   << GPIO_PIN_CNF_INPUT_Pos) \
                             | (GPIO_PIN_CNF_DIR_Input          << GPIO_PIN_CNF_DIR_Pos));
	}

    return true;
}
#endif /* CONFIG_PWR_MGMT_ENABLED */

ret_code_t drv_board_init(void)
{
    /* Make sure that all peripherals with dynamic power management are off. */
    DEASSERT_SIGNAL(CONFIG_IO_LED_0,            CONFIG_LED_ACT_LOW_MASK & (1 << 0), true);
    DEASSERT_SIGNAL(CONFIG_IO_LED_1,            CONFIG_LED_ACT_LOW_MASK & (1 << 1), true);


    return NRF_SUCCESS;
}

