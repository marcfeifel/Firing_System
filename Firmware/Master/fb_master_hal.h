#ifndef FB_MASTER_HAL_INCLUDED
#define FB_MASTER_HAL_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "compiler_defs.h"

void HAL_assert_uart_rts();
void HAL_deassert_uart_rts();
bool HAL_get_pin_switch_armed();
bool HAL_get_pin_switch_test();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FB_MASTER_HAL_INCLUDED