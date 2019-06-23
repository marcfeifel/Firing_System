#include "fb_common.h"
#include "fb_messages.h"

//#define ENABLE_CLOCK_SKEWING

// used by the millisecond timer
static volatile uint32_t m_millis = 0;

void Task_1ms_Handler(void);

void Task_1ms_High_Priority_Disable(void)
{
    // disable interrupt
    ET2 = 0;

} // Task_1ms_High_Priority_Disable()


void Task_1ms_High_Priority_Enable(void)
{
    // restore the interrupt enable
    ET2 = 1;

} // Task_1ms_High_Priority_Enable()


// get the number of milliseconds since power-on
uint32_t millis(void)
{
    // this is not atomic, so need to disable interrupts

    // will store the count
    uint32_t millis = 0;

    // disable interrupt
    Task_1ms_High_Priority_Disable();
    
    // get the count
    millis = m_millis;

    // restore the interrupt enable
    Task_1ms_High_Priority_Enable();

    // return the count
    return millis;

} // millis()


bool millis_expired(uint32_t timer_ms)
{
    return millis() > timer_ms ? true : false;

} // millis_expired()


void Sleep(uint32_t ms)
{
    uint32_t sleep_until = millis() + ms;

    while (millis() < sleep_until)
    {
        // do nothing
    }
} // Sleep()


void Reset_MCU(void)
{
    RSTSRC |= BIT4;
    while (1)
    {
    }
} // Reset()


// handle timer interrupts
// 1ms high-priority interrupts
INTERRUPT(timer2_ISR, INTERRUPT_TIMER2)
{
    // clear the flag
    TF2H = 0;

    // increment the counter
    m_millis++;
    
    Task_1ms_High_Priority_Handler();
    
} // timer2_ISR()


// handle timer interrupts
// 1ms low-priority interrupts
INTERRUPT(timer3_ISR, INTERRUPT_TIMER3)
{
    // clear the flag
    TMR3CN &= ~0x80;

    Task_1ms_Low_Priority_Handler();
    
} // timer2_ISR()

uint16_t Remote_NodeID_to_Integer(uint8_t nodeid)
{
    return nodeid - NODEID_REMOTE_BASE;
    
} //  Remote_NodeID_to_Integer()
