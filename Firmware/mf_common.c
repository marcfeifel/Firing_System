#include "mf_common.h"
#include "RFM69.h"

// used by the millisecond timer
static volatile uint32_t _millis = 0;

// used by the RFM69 ISR handler
static volatile bit RFM69_DIO0_last = LOW;
static volatile bit RFM69_ISR_Enabled = false;
static volatile bit RFM69_ISR_Pending = false;

// get the number of milliseconds since power-on
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
    
} // millis()


// used by the RFM69 to enable its interrupt handler
void Interrupts(void)
{
    // signal interrupts are enabled
    RFM69_ISR_Enabled = 1;
    
} // Interrupts()


// used by the RFM69 to disable its interrupt handler
void noInterrupts(void)
{
    // signal interrupts are disabled
    RFM69_ISR_Enabled = 0;
    
} // noInterrupts()


void Sleep(uint32_t ms)
{
    uint32_t sleep_until = millis() + ms;
    
    while (millis() < sleep_until)
    {
        // do nothing
    }
} // Sleep()


// handle timer interrupts
INTERRUPT(timer2_ISR, INTERRUPT_TIMER2)
{
    // get this bit
    bit RFM69_DIO0_this = PIN_RFM69HW_DIO0_I;
    
    // clear the flag
    TF2H = 0;
    
    // increment the counter
    _millis++;

    // if was low and now is high, then ISR is pending
    if (!RFM69_DIO0_last && RFM69_DIO0_this)
    {
        // indicate an interrupt is pending
        RFM69_ISR_Pending = true;
    }
    else
    {
        // do nothing
    }
    
    // store the present state of DIO0
    RFM69_DIO0_last = RFM69_DIO0_this;

    // if ISR is enabled and ISR is pending, call handler
    if (RFM69_ISR_Enabled && RFM69_ISR_Pending)
    {
        // call the handler
        RFM69_interruptHandler();
        
        // clear the pending flag
        RFM69_ISR_Pending = false;
        
    }
    else
    {
        // do nothing
    }
        
} // timer2_ISR()
