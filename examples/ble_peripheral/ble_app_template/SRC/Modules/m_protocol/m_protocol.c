

#include <stdint.h>
#include <stdbool.h>

#include "app_debug.h"
#include "app_error.h"

#include "m_system_state.h"
#include "protocol.h"
#include "resources.h"
#include "sr3_config.h"
#include "m_protocol.h"
#include "m_protocol_link.h"
#include "m_protocol_interact.h"
#include "m_protocol_devinfo.h"
#include "m_protocol_daily_sport.h"
#include "m_protocol_pack_trans.h"
#include "m_protocol_pack_trans.h"
#include "m_protocol_sensor_data.h"
//#include "m_protocol_sleep_record.h"
#include "m_protocol_test.h"

typedef bool (*prot_evt_execution_t)(protocol_evt_t *);


/**@brief Event Bus handlers. */
static const event_handler_t m_protocol_event_handlers[] =
{
    m_protocol_link_event_handler,
    m_protocol_ineract_event_handler,
    m_protocol_devinfo_event_handler,
    m_protocol_daily_sport_event_handler,
    m_protocol_pack_trans_event_handler,
    m_protocol_sensor_data_event_handler,
//    m_protocol_sleep_record_event_handler,
    m_protocol_test_event_handler,
};

/**@brief Protocol Event handlers. */
static const prot_evt_execution_t prot_evt_executions[] =
{
    m_protocol_link_prot_evt_execution,
    m_protocol_interact_prot_evt_execution,
    m_protocol_devinfo_prot_evt_execution,
    m_protocol_daily_sport_prot_evt_execution,
    m_protocol_pack_trans_prot_evt_execution,
    m_protocol_sensor_data_prot_evt_execution,
//    m_protocol_sleep_record_prot_evt_execution,
    m_protocol_test_prot_evt_execution,
};

bool m_protocol_event_handler(const event_t * p_event)
{
    for (uint8_t i = 0; i < ARRAY_SIZE(m_protocol_event_handlers); i++)
    {
        m_protocol_event_handlers[i](p_event);
    }

    return false;
}

static void prot_evt_execution(protocol_evt_t * p_event)
{
    for (uint8_t i = 0; i < ARRAY_SIZE(prot_evt_executions); i++)
    {
        if (prot_evt_executions[i](p_event))
        {
            break;
        } 
    }
}

void m_protocol_init(m_protocol_recv_handler_t recv, m_protocol_send_handler_t send)
{
    protocol_recv_handler_t recv_handler;

    recv_handler.input_func = recv;
    recv_handler.output_func = prot_evt_execution;

    protocol_init(recv_handler, send);

    APP_ERROR_CHECK(m_protocol_link_init());
    APP_ERROR_CHECK(m_protocol_ineract_init());
    APP_ERROR_CHECK(m_protocol_devinfo_init());
    APP_ERROR_CHECK(m_protocol_daily_sport_init());
    APP_ERROR_CHECK(m_protocol_pack_trans_init());
    APP_ERROR_CHECK(m_protocol_sensor_data_init());
//    APP_ERROR_CHECK(m_protocol_sleep_record_init());
    APP_ERROR_CHECK(m_protocol_test_init());
    
}

void m_protocol_recv_process(void)
{
    protocol_recv_process();
}





















