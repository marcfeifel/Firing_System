#ifndef FB_C51_TARGET_H_INCLUDED
#define FB_C51_TARGET_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "compiler_defs.h"
#include "stdint.h"
#include "stdbool.h"
#include "c8051f930_defs.h"

// definition for the master device
#define NODEID    NODEID_MASTER
#define NETWORKID 1

/* GPIO assignments */
SBIT(PIN_SWITCH_TEST_I, P0, 0); // Switch Input - "Test"
SBIT(PIN_SWITCH_ARMED_I, P0, 1); // Switch Input - "Armed" 
//PIN 0.2 - FTDI CBUS3
//PIN 0.3 - FTDI CBUS0
//PIN 0.4 - FTDI Rx (previously FTDI CTS)
//PIN 0.5 - FTDI Tx (previously FTDI RTS)
SBIT(PIN_UART_RTS_O, P0, 6); // FTDI CTS (previously FTDI Tx)
//PIN 0.7 - Open (previously FTDI Rx)

SBIT(PIN_RFM69HW_DIO0_I, P1, 0); // RFM69HW DIO0
SBIT(PIN_RFM69HW_DIO1_I, P1, 1); // RFM69HW DIO1
SBIT(PIN_RFM69HW_DIO2_I, P1, 2); // RFM69HW DIO2
SBIT(PIN_RFM69HW_DIO3_I, P1, 3); // RFM69HW DIO3
SBIT(PIN_RFM69HW_DIO4_I, P1, 4); // RFM69HW DIO4
SBIT(PIN_RFM69HW_DIO5_I, P1, 5); // RFM69HW DIO5
SBIT(PIN_RFM69HW_SCK_O,  P1, 6); // RFM69HW SCK (previously RFM69HW NSS)
SBIT(PIN_RFM69HW_MISO_I, P1, 7); // RFM69HW MISO (previously RFM69HW MOSI)

SBIT(PIN_RFM69HW_MOSI_O, P2, 0); // RFM69HW MOSI (previously RFM69HW MISO)
SBIT(PIN_RFM69HW_NSS_O,  P2, 1); // RFM69HW NSS (previously RFM69HW_NSS)
// PIN 2.2 - Open
// PIN 2.3 - Open
// PIN 2.4 - Open
SBIT(PIN_RFM69HW_RST_O, P2, 5); // RFM69HW RST
// PIN 2.6 - open
// PIN 2.7 - C2D

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FB_C51_TARGET_H_INCLUDED
