

#ifndef __M_PROTOCOL_H__
#define __M_PROTOCOL_H__

#include "protocol.h"
#include "event_bus.h"
typedef protocol_input_handler_t m_protocol_recv_handler_t ;

typedef protocol_send_handler_t m_protocol_send_handler_t ;



bool m_protocol_event_handler(const event_t *p_event);


void m_protocol_init(m_protocol_recv_handler_t recv, m_protocol_send_handler_t send);

/* 每次成功收到数据后，调用该函数进行数据处理 */
void m_protocol_recv_process(void);

#endif  /** __M_PROTOCOL_H__ */

/** @} */
