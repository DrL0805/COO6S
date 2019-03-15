
#ifndef __M_PROTOCOL_TEST_H__
#define __M_PROTOCOL_TEST_H__

#include "event_bus.h"
#include "protocol.h"


bool m_protocol_test_event_handler(const event_t *p_event);

bool m_protocol_test_prot_evt_execution(protocol_evt_t * p_evt);

uint32_t m_protocol_test_init(void);


#endif  /** __M_PROTOCOL_LINK_H__ */


