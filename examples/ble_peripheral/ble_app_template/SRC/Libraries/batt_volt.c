

#include "nrf_drv_saadc.h"
#include "batt_volt.h"
#include "app_debug.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#define ADC_ENABLE_PIN                                  CONFIG_IO_ADC_ENABLE_PIN
#define ADC_ENABLE_ACTIVE                               1

static const nrf_drv_saadc_config_t saadc_config =
{
    .resolution         = (nrf_saadc_resolution_t)1,
    .oversample         = (nrf_saadc_oversample_t)0,
    .interrupt_priority = 6,
    .low_power_mode     = 0,
};

#define SAADC_CHANNEL_CONFIG_VOL(PIN_P)                \
    {                                                  \
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,     \
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,     \
        .gain       = NRF_SAADC_GAIN1_6,               \
        .reference  = NRF_SAADC_REFERENCE_INTERNAL,    \
        .acq_time   = NRF_SAADC_ACQTIME_10US,          \
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,     \
        .burst      = NRF_SAADC_BURST_DISABLED,        \
        .pin_p      = (nrf_saadc_input_t)(PIN_P),      \
        .pin_n      = NRF_SAADC_INPUT_DISABLED         \
    }

#define ADC_REF_VOLTAGE_IN_MILLIVOLTS  600  //!< Reference voltage (in milli volts) used by ADC while doing conversion.
#define DIODE_FWD_VOLT_DROP_MILLIVOLTS 270  //!< Typical forward voltage drop of the diode (Part no: SD103ATW-7-F) that is connected in series with the voltage supply. This is the voltage drop when the forward current is 1mA. Source: Data sheet of 'SURFACE MOUNT SCHOTTKY BARRIER DIODE ARRAY' available at www.diodes.com.
#define ADC_RES_10BIT                  1024 //!< Maximum digital value for 10-bit ADC conversion.
#define ADC_PRE_SCALING_COMPENSATION   6    //!< The ADC is configured to use VDD with 1/6 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
    ((((ADC_VALUE) *ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_10BIT) * ADC_PRE_SCALING_COMPENSATION)

static nrf_saadc_value_t adc_buf[1];        //!< Buffer used for storing ADC values.
static uint16_t m_batt_lvl_in_milli_volts = 0;  //!< Current battery level.

#if 1
__STATIC_INLINE void adc_pin_ctrl_on(void)
{
    if (ADC_ENABLE_ACTIVE)
    {
        nrf_gpio_pin_set(ADC_ENABLE_PIN);
    }
    else
    {
        nrf_gpio_pin_clear(ADC_ENABLE_PIN);
    }
}
#endif

__STATIC_INLINE void adc_pin_ctrl_off(void)
{
    if (ADC_ENABLE_ACTIVE)
    {
        nrf_gpio_pin_clear(ADC_ENABLE_PIN);
    }
    else
    {
        nrf_gpio_pin_set(ADC_ENABLE_PIN);
    }
}


__STATIC_INLINE void adc_pin_ctrl_init(void)
{
    nrf_gpio_cfg_output(ADC_ENABLE_PIN);
}


/**@brief Function handling events from 'nrf_drv_saadc.c'.
 *
 * @param[in] p_evt SAADC event.
 */
static void saadc_event_handler(nrf_drv_saadc_evt_t const * p_evt)
{
    if (p_evt->type == NRF_DRV_SAADC_EVT_DONE)
    {
        nrf_saadc_value_t adc_result;

        adc_result = p_evt->data.done.p_buffer[0];
#if 1
        m_batt_lvl_in_milli_volts = ADC_RESULT_IN_MILLI_VOLTS(adc_result) *3 >> 1; 
                                    
#else
            nrf_saadc_value_t measurement = *(nrf_saadc_value_t *)(p_context);
    uint32_t voltage;
    uint8_t level = 0;
    // Calculate battery voltage
    voltage = (((uint32_t)(measurement) * ADC_DIVIDER * ADC_REFERENCE) / ADC_MAX_CONV)  * 3;// 1M / (1+2)M ·ÖÑ¹
        m_batt_lvl_in_milli_volts = adc_result;
#endif
    }
}

uint32_t batt_volt_init(void)
{
    ret_code_t                  err_code;
    nrf_saadc_channel_config_t  config = SAADC_CHANNEL_CONFIG_VOL(NRF_SAADC_INPUT_AIN1);

    adc_pin_ctrl_init();
    adc_pin_ctrl_off();

    err_code = nrf_drv_saadc_init(&saadc_config, saadc_event_handler);

    if (err_code == NRF_SUCCESS)
    {
        err_code = nrf_drv_saadc_channel_init(0, &config);
    }

    if (err_code == NRF_SUCCESS)
    {
        err_code = nrf_drv_saadc_buffer_convert(adc_buf, 1);
    }

    if (err_code == NRF_SUCCESS)
    {
        err_code = nrf_drv_saadc_sample();
    }

    return err_code;
}
uint32_t batt_volt_convert(void)
{
    DBG1_EVT0("Auto Test: batt_volt_convert");
    ret_code_t                  err_code;
//    nrf_saadc_channel_config_t  config = SAADC_CHANNEL_CONFIG_VOL(NRF_SAADC_INPUT_VDD);

    adc_pin_ctrl_on();
    nrf_delay_ms(1);

//    err_code = nrf_drv_saadc_init(&saadc_config, saadc_event_handler);

 
    err_code = nrf_drv_saadc_buffer_convert(adc_buf, 1);


    if (err_code == NRF_SUCCESS)
    {
        err_code = nrf_drv_saadc_sample();
    }

    return err_code;
}

uint32_t batt_volt_get(uint16_t * p_vbatt)
{

    uint32_t err_code = NRF_SUCCESS;

    *p_vbatt = m_batt_lvl_in_milli_volts;

    adc_pin_ctrl_off();

//    err_code = nrf_drv_saadc_buffer_convert(adc_buf, 1);

//    if (err_code == NRF_SUCCESS)
//    {
//        err_code = nrf_drv_saadc_sample();
//    }

    return err_code;
}


uint32_t batt_volt_disabled(void)
{
    uint32_t err_code = NRF_SUCCESS;
    nrf_drv_saadc_uninit();
    adc_pin_ctrl_off();

    return err_code;
}



