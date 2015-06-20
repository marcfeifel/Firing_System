#include <stdio.h>

#include "mf_common.h"
#include "rfm69.h"

void main(void)
{
    KEYSWITCH_STATE_t keyswitch_state_prev = KEYSWITCH_STATE_FAULT;
    
    Init_Device();
    
    // put the HopeRF into reset
    PIN_RFM69HW_RST_O = 1;
    PIN_RFM69HW_RST_O = 0;
    
    // clear the CTS signal so the FTDI will transmit
    PIN_UART_RTS_O = 0;
    
    printf("Start.\r\n");
    
    printf("Pre-init.\r\n");
    RFM69__readAllRegs();
    RFM69_construct(true);
    RFM69_initialize();
    printf("Post-init.\r\n");
    RFM69__readAllRegs();

    while (1)
    {
        // get the current 
        const KEYSWITCH_STATE_t keyswitch_state_this = Get_Keyswitch_State();

        if (keyswitch_state_this != keyswitch_state_prev)
        {
            switch (keyswitch_state_this)
            {
                case KEYSWITCH_STATE_OFF:
                    printf("Keyswitch: Off\r\n");
                    break;
                
                case KEYSWITCH_STATE_TEST:
                    printf("Keyswitch: Test\r\n");
                    break;
                
                case KEYSWITCH_STATE_ARMED:
                    printf("Keyswitch: Armed\r\n");
                    break;
                
                case KEYSWITCH_STATE_FAULT:
                default:
                    printf("Keyswitch: Fault\r\n");
                    break;
                
            } // switch (mode change)
            
            // store it for next time
            keyswitch_state_prev = keyswitch_state_this;
            
        }
        else
        {
            // do nothing
        }
    }
} // main()
