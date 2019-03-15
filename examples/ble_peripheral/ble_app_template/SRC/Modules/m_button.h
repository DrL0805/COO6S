
#ifndef M_BUTTON_H__
#define M_BUTTON_H__

#include "m_drv_btn_action.h"
#include "event_bus.h"

#ifdef __cplusplus
extern "C" {
#endif



uint32_t m_button_init(void);

uint32_t m_button_enable(void);

uint32_t m_button_disable(void);

bool m_button_event_handler(const event_t * p_event);



bool m_button_check_enter_normal_state_button(void);


#ifdef __cplusplus
}
#endif

#endif /* M_BUTTON_H__ */

/** @} */

