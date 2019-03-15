
#ifndef __M_PROTOCOL_XXX_H__
#define __M_PROTOCOL_XXX_H__

#include "event_bus.h"
#include "protocol.h"


bool m_protocol_xxx_event_handler(const event_t *p_event);

bool m_protocol_xxx_prot_evt_execution(protocol_evt_t * p_evt);

uint32_t m_protocol_xxx_init(void);


#endif  /** __M_PROTOCOL_XXX_H__ */


