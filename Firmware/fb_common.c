#include "fb_common.h"
#include "fb_messages.h"
#include "RFM69.h"

//#define ENABLE_CLOCK_SKEWING

// used by the millisecond timer
static volatile uint32_t m_millis = 0;

// used by the RFM69 ISR handler
static volatile bool RFM69_DIO0_last = LOW;
static volatile bool RFM69_ISR_Enabled = false;
static volatile bool RFM69_ISR_Pending = false;

// get the number of milliseconds since power-on
uint32_t millis(void)
{
    // this is not atomic, so need to disable interrupts

    // will store the count
    uint32_t millis = 0;

    // store the previous interrupt-enable state
    bool old_ET2 = ET2;

    // disable interrupt
    ET2 = 0;

    // get the count
    millis = m_millis;

    // restore the interrupt enable
    ET2 = old_ET2;

    // return the count
    return millis;

} // millis()


void millis_correct(uint32_t reference_millis)
{
    bool old_ET2 = ET2;
    
    ET2 = 0;
    
    m_millis = (m_millis + reference_millis) >> 1;
    
    ET2 = old_ET2;
    
} // millis_correct()


bool millis_expired(uint32_t timer_ms)
{
    return millis() > timer_ms ? true : false;

} // millis_expired()


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


void Reset_MCU(void)
{
    RSTSRC |= BIT4;
    while (1)
    {
    }
} // Reset()


// handle timer interrupts
INTERRUPT(timer2_ISR, INTERRUPT_TIMER2)
{
    
    // get this bit
    bool RFM69_DIO0_this = PIN_RFM69HW_DIO0_I;

    // clear the flag
    TF2H = 0;

    // increment the counter
    m_millis++;
    
#ifdef ENABLE_CLOCK_SKEWING
#ifdef NODEID_LOCAL
#if NODEID_LOCAL==NODEID_REMOTE3
    // intentionally skew the clock
    do
    {
        static uint8_t skew = 0;

        if (!skew++)
        {
            m_millis++;
        }
        else
        {
            // do nothing
        }
        
    } while (0);
#endif
#endif
#endif
    
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


bool Ping(uint32_t target, uint32_t timeout_ms, uint16_t * p_response_time_ms, uint16_t * p_rssi)
{
    // will hold the timestamp for the end of the timeout period
    uint32_t ping_started_ms = 0;
    uint32_t ping_ended_ms = 0;

    // assume we didn't receive the pong
    bool pong_received = false;

    // setup the end of the timeout window
    ping_started_ms = millis();

    // send a ping
    RFM69_send(target, "Ping", 4, false);

    // loop until pong is received or we time out
    do
    {
        // find out when we started
        ping_ended_ms = millis();

        // TODO - need to actually check for a "pong" and such
        pong_received = RFM69_receiveDone();

    } while (!pong_received && ((ping_started_ms + timeout_ms) > ping_ended_ms));

    *p_rssi = RFM69_getRSSI();
    *p_response_time_ms = ping_ended_ms - ping_started_ms;

    // return whether we received a pong or not
    return pong_received;

} // Ping()
