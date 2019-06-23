#include "fb_c51_target.h"

void HAL_assert_uart_rts() {
    PIN_UART_RTS_O = 1;
}

void HAL_deassert_uart_rts() {
    PIN_UART_RTS_O = 0;
}

bool HAL_get_pin_switch_armed() {
    return PIN_SWITCH_ARMED_I;
}

bool HAL_get_pin_switch_test() {
	  return PIN_SWITCH_TEST_I;
}