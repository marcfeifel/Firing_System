#include <stdio.h>

#include "mf_c51_target.h"
#include "rfm69.h"

typedef enum
{
    KEYSWITCH_STATE_OFF,
    KEYSWITCH_STATE_ARMED,
    KEYSWITCH_STATE_TEST,
    KEYSWITCH_STATE_FAULT,
    
} KEYSWITCH_STATE_t;

uint32_t millis(void);
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

static volatile uint32_t _millis = 0;

uint32_t millis(void)
{
    // this is not atomic, so need to disable interrupts
    
    // will store the count
    uint32_t millis = 0;

    // store the previous interrupt-enable state
    bit old_ET2 = ET2;
    
    // disable interrupt
    ET2 = 0;
    
    // get the count
    millis = _millis;
    
    // restore the interrupt enable
    ET2 = old_ET2;
    
    // return the count
    return millis;
}

static volatile bit RFM69_DIO0_last = LOW;
static volatile bit RFM69_ISR_Enabled = false;
static volatile bit RFM69_ISR_Pending = false;
INTERRUPT(timer0_ISR, INTERRUPT_TIMER2)
{
    // get this bit
    bit RFM69_DIO0_this = PIN_RFM69HW_DIO0_I;
    
    // clear the flag
    TF2H = 0;
    
    // increment
    _millis++;

    // if was low and now is high, then ISR is pending
    if (!RFM69_DIO0_last && RFM69_DIO0_this)
    {
        RFM69_ISR_Pending = true;
    }
    RFM69_DIO0_last = RFM69_DIO0_this;

    // if ISR is enabled and ISR is pending, call handler
    if (RFM69_ISR_Enabled && RFM69_ISR_Pending)
    {
        RFM69_interruptHandler();
        
        RFM69_ISR_Pending = false;
    }
        
}

void Interrupts(void)
{
    RFM69_ISR_Enabled = 1;
}

void noInterrupts(void)
{
    RFM69_ISR_Enabled = 0;
}

