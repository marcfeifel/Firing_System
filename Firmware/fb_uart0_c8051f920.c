#include "fb_common.h"
#include "fb_messages.h"
#include "fb_c51_target.h"

static bit first_pass_complete = 0;

#define BUFFER_LENGTH 32
static volatile char buffer_rx[BUFFER_LENGTH];
static volatile char buffer_rx_head = 0;
static volatile char buffer_rx_tail = 0;
static volatile char buffer_rx_count = 0;
static volatile char buffer_tx[BUFFER_LENGTH];
static volatile char buffer_tx_head = 0;
static volatile char buffer_tx_tail = 0;
static volatile char buffer_tx_count = 0;
static volatile char buffer_tx_active = 0;

INTERRUPT(uart0_ISR, INTERRUPT_UART0)
{
    if( RI0 ) {
        if ( buffer_rx_count < BUFFER_LENGTH ) {
            buffer_rx_count++;
            buffer_rx[buffer_rx_head++] = SBUF0;
            if(buffer_rx_head >= BUFFER_LENGTH) {
                buffer_rx_head = 0;
            }
        }
        RI0 = 0;
    } else if ( TI0 ) {
        if ( buffer_tx_count > 0 ) {
            buffer_tx_count--;
            SBUF0 = buffer_tx[buffer_tx_tail++];
            if(buffer_tx_tail >= BUFFER_LENGTH) {
                buffer_tx_tail = 0;
            }
        } else {
            buffer_tx_active = 0;
        }
        TI0 = 0;
    } else {
        // hmm?
    }
}

void retarget_init(void)
{
    // we're initialized
    first_pass_complete = 1;
    
} // retarget_init()


char _getkey(void)
{
    char c;
    
    // make sure we're ready for this
    if (!first_pass_complete)
        retarget_init();
    
    // wait for RI
    while (0 == buffer_rx_count)
    {
        // loop until RI set
    }
    
    // disable the interrupt
    ES0 = 0;
    
    // get the byte
    buffer_rx_count--;
    c = buffer_rx[buffer_rx_tail++];
    if( buffer_rx_tail >= BUFFER_LENGTH ) {
        buffer_rx_tail = 0;
    }    
    
    // enable the interrupt
    ES0 = 1;
    
    // return the byte
    return c;
    
} // _getkey()


char _kbhit(void)
{
    // make sure we're ready for this
    if (!first_pass_complete)
        retarget_init();
    
    return ( buffer_rx_count > 0 );
    
} // _kbhit()


char putchar(char c)
{
    // make sure we're ready for this
    if (!first_pass_complete)
        retarget_init();
    
    while( buffer_tx_count >= BUFFER_LENGTH ) {
        // loop until there is room
    }
    
    // disable interrupts
    ES0 = 0;

    if( buffer_tx_active ) {
        // move the byte into the transmit buffer
        buffer_tx_count++;
        buffer_tx[buffer_tx_head++] = c;
        if( buffer_tx_head >= BUFFER_LENGTH ) {
            buffer_tx_head = 0;
        }
    } else {
        // start the transmission
        buffer_tx_active = 1;
        SBUF0 = c;
    }
    
    // enable interrupts
    ES0 = 1;
    
    // a byte was successfully moved into the transmit buffer
    return c;
    
} // putchar()
