
#ifndef DRV_LTR_390__H__
#define DRV_LTR_390__H__


ret_code_t uv_test(void);
ret_code_t drv_ltr_390uv_init(void);
ret_code_t drv_ltr_390uv_uninit(void);
ret_code_t ltr_390uv_enable(void);
ret_code_t ltr_390uv_disable(void);
ret_code_t ltr_390uv_read_uv_data(uint8_t *uv_data_buf);

#endif // DRV_BDTEMP__H__

