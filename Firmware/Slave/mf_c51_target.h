#ifndef MF_C51_TARGET_H_INCLUDED
#define MF_C51_TARGET_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "compiler_defs.h"
#include "c8051f930_defs.h"

// definition for the master device
#define NODEID 1
#define NETWORKID 1

/* GPIO assignments */
SBIT(PIN_ADDRESS_CUEb0_O,  P0, 0); // Cue select bit 0 ("Column" select bit 0)
SBIT(PIN_ADDRESS_CUEb1_O,  P0, 1); // Cue select bit 1 ("Column" select bit 1)
SBIT(PIN_ADDRESS_CUEb2_O,  P0, 2); // Cue select bit 2 ("Row" select bit 0)
SBIT(PIN_ADDRESS_CUEb3_O,  P0, 3); // Cue select bit 3 ("Row" select bit 1)
SBIT(PIN_ADDRESS_SOCKb0_O, P0, 4); // Socket select bit 0
SBIT(PIN_ADDRESS_SOCKb1_O, P0, 5); // Socket select bit 1
SBIT(PIN_ADDRESS_SOCKb2_O, P0, 6); // Socket select bit 2
SBIT(PIN_CUE_PRESENT_I,    P0, 7); // High if Cue is populated

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
SBIT(PIN_FIRE_CLK_A,     P2, 2); // Clocks U5A 4-bit counter
SBIT(PIN_FIRE_CLK_B,     P2, 3); // Clocks U5B 4-bit counter
// PIN 2.4 - Open
SBIT(PIN_RFM69HW_RST_O,  P2, 5); // RFM69HW RST
SBIT(PIN_FIRE_CLEAR,     P2, 6); // Clears U5A and U5B
// PIN 2.7 - C2D

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MF_C51_TARGET_H_INCLUDED
