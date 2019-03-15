
#ifndef TEST_MANUF_COMS_H__
#define TEST_MANUF_COMS_H__


typedef enum
{
    TMC_EVT_SEND_OK,
}tmc_evt_t;

typedef void (*tmc_evt_handler_t)(tmc_evt_t evt);

void tmc_send_command(uint8_t * p_data, uint16_t length);

uint32_t tmc_init(tmc_evt_handler_t);

void tmc_disable(void);


#endif /* TEST_MANUF_COMS_H__ */


