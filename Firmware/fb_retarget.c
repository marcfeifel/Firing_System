#include "reg51.h"

static bit first_pass_complete = 0;

void retarget_init(void)
{
    // clear TI
    TI = 0;

    // clear RI so that we don't indicate we've received anything
    RI = 0;
    
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
    while (!RI)
    {
        // loop until RI set
    }
    
    // get the byte
    c = SBUF;
    
    // clear RI
    RI = 0;
    
    // return the byte
    return c;
    
} // _getkey()


char _kbhit(void)
{
    // make sure we're ready for this
    if (!first_pass_complete)
        retarget_init();
    
    return RI;
    
} // _kbhit()


char putchar(char c)
{
    // make sure we're ready for this
    if (!first_pass_complete)
        retarget_init();

    // move the byte into the transmit butter
    SBUF = c;
    
    // for now, this blocks until a byte gets sent
    while (!TI)
    {
        // loop until it gets set
    }
    
    // clear it
    TI = 0;
    
    // a byte was successfully moved into the transmit buffer
    return c;
    
} // putchar()
