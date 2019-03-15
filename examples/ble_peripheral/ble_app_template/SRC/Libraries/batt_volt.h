
#ifndef BATT_VOLT_H__
#define BATT_VOLT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


/**@brief Function for initializing the battery voltage module.
 */
uint32_t batt_volt_init(void);
uint32_t batt_volt_convert(void);

/**@brief Function for reading the battery voltage.
 *
 * @param[out]   p_vbatt       Pointer to the battery voltage value.
 */
uint32_t batt_volt_get(uint16_t * p_vbatt);


uint32_t batt_volt_disabled(void);



#ifdef __cplusplus
}
#endif

#endif /* BATT_VOLT_H__ */


