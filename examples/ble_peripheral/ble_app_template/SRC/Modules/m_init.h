#ifndef __M_INIT_H__
#define __M_INIT_H__

/**@brief Calls initialization functions for the enabled modules as defined by sr3_config.
 *
 * @param[in]   is_resume              True if system is resuming from the System OFF state.
 */
void m_init(bool is_resume, bool is_pressed, bool is_pressed_reset,uint32_t error_resume);

#endif
