#include <stdio.h>

#include "mf_common.h"
#include "rfm69.h"

typedef enum
{
    KEYSWITCH_STATE_OFF,
    KEYSWITCH_STATE_ARMED,
    KEYSWITCH_STATE_TEST,
    KEYSWITCH_STATE_FAULT,
    
} KEYSWITCH_STATE_t;

KEYSWITCH_STATE_t Get_Keyswitch_State(void);

void main(void)
{
    // will store the keyswitch state
    KEYSWITCH_STATE_t keyswitch_state_prev = KEYSWITCH_STATE_FAULT;
    
    // initialize the C8051F920
    Init_Device();
    // clear the CTS signal so the FTDI will transmit
    PIN_UART_RTS_O = 0;
    
    printf("Start.\r\n");

    // initialize the HopeRF radio
    RFM69_construct(true);
    RFM69_initialize();

    printf("Super loop started.\r\n");
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


// read the key-switch inputs and interpret
KEYSWITCH_STATE_t Get_Keyswitch_State(void)
{
    // get the state of the pins
    const bit pin_armed = PIN_SWITCH_ARMED_I;
    const bit pin_test = PIN_SWITCH_TEST_I;
    
    // assume a faulted state
    KEYSWITCH_STATE_t state = KEYSWITCH_STATE_FAULT;
    
    if (!pin_armed && !pin_test)
    {
        // both pins low
        state = KEYSWITCH_STATE_OFF;
        
    }
    else if (pin_armed && !pin_test)
    {
        // armed high, test low
        state = KEYSWITCH_STATE_ARMED;
        
    }
    else if (!pin_armed && pin_test)
    {
        // armed low, test high
        state = KEYSWITCH_STATE_TEST;
        
    }
    else
    {
        // both pins high
        // do nothing - already assumed faulted state
    } // if (switch pins)
    
    // return the state
    return state;
    
} // Get_Keyswitch_State()
