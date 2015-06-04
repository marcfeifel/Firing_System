#include "mf_c51_target.h"

#include <stdio.h>

typedef enum
{
    KEYSWITCH_STATE_OFF,
    KEYSWITCH_STATE_ARMED,
    KEYSWITCH_STATE_TEST,
    KEYSWITCH_STATE_FAULT,
    
} KEYSWITCH_STATE_t;

code U8 init[] =
{
    0xFF, // dummy
    0x04, // 0x01
    0x00,
    0x1A,
    0x0B,
    0x00,
    0x52,
    0xE4,
    0xC0,
    0x00,
    0x41,
    0x00,
    0x02,
    0x92,
    0xF5,
    0x20, // 0x0F
    0x24,
    0x9F,
    0x09,
    0x1A,
    0x40,
    0xB0,
    0x7B,
    0x9B,
    0x08,
    0x86,
    0x8A,
    0x40,
    0x80,
    0x06,
    0x10,
    0x00, // 0x1F
    0x00,
    0x00,
    0x00,
    0x02,
    0xFF,
    0x00,
    0x05,
    0x80,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
    0x03,
    0x98,
    0x00, // 0x2F
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x10,
    0x40,
    0x00,
    0x00,
    0x00,
    0x0F,
    0x02,
    0x00,
    0x00, // 0x3F
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x01,
    0x00 // 0x4F
};

U8 resp[sizeof(init)];


void Init_Device(void);
KEYSWITCH_STATE_t Get_Keyswitch_State(void);

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
    
    do
    {
        U8 i;
        
        // write it all
        for (i = 1; i < 0x50; i++)
        {
            // set chip-select low
            NSS0MD0 = 0;

            // clear the interrupt flat
            SPIF0 = 0;
            
            // write the address
            SPI0DAT = i | 0x80;
            
            while (!SPIF0)
            {
                // wait for the xfer to complete
            }
            
            // clear the interrupt flag
            SPIF0 = 0;
            
            // start the transfer
            SPI0DAT = init[i];
            
            while (!SPIF0)
            {
                // wait for the xfer to complete
            }
            
            // set chip-select high
            NSS0MD0 = 1;

        } // for (registers);


        // read it all back
        for (i = 1; i < 0x50; i++)
        {
            // set chip-select low
            NSS0MD0 = 0;

            // clear the interrupt flat
            SPIF0 = 0;
            
            // write the address
            SPI0DAT = i;
            
            while (!SPIF0)
            {
                // wait for the xfer to complete
            }
            
            // clear the interrupt flag
            SPIF0 = 0;
            
            // start the transfer
            SPI0DAT = 0;
            
            while (!SPIF0)
            {
                // wait for the xfer to complete
            }
            
            resp[i] = SPI0DAT;
            
            // set chip-select high
            NSS0MD0 = 1;

        } // for (registers);

        for (i = 1; i < 0x50; i++)
        {
            // print it
            printf("Reg: 0x%02X = 0x%02X, Expected 0x%02X\r\n", (U16)i, (U16)resp[i], (U16)init[i]);
        }
    } while (0);
    
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
