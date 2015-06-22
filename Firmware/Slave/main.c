#include <string.h>

#include "mf_common.h"
#include "rfm69.h"

#define CLOCKS_TO_TEST 8
#define CLOCKS_TO_FIRE 8

void FireTest_Clear(void);
void FireTest_Assert_Fire(void);
void FireTest_Assert_Test(void);

void Cue_Select(SOCKET_ENUM_t socket, CUE_ENUM_t cue);
bool Cue_Is_Present(void);
void Cue_Fire(void);
void Cue_Scan_All(REMOTE_t * p_remote);

static REMOTE_t cues_present = { 0 };

static bit system_is_armed = true;

void main(void)
{
    // initialize the C8051F920
    Init_Device();
    
    // clear the fire and test signals
    FireTest_Clear();
    
    // initialize the HopeRF radio
    RFM69_construct(true);
    RFM69_initialize();

    // super loop
    while (true)
    {
/*        if (RFM69_receiveDone())
        {
            REMOTE_t remote = {0};
            
            if (RFM69_ACKRequested());
            {
                Sleep(100);
                RFM69_sendACK("", 0);
            }
            
            Cue_Scan_All(&remote);
            
            RFM69_send(NODEID_MASTER, &remote, sizeof(remote), false);
            
//            while (!RFM69_ACKReceived(NODEID_MASTER))
            {
                // loop while waiting
            }
        }
        else
        {
            // do nothing
        }*/
        
        Disbatcher();
    } // while (true)
} // main()


void FireTest_Clear(void)
{
    // assert the reset/clear signal
    PIN_COUNTER_CLEAR_O = 1;
    Sleep(2);

    // set both clock pins high to put them into their idle state
    PIN_TEST_CLK_O = 1;
    PIN_FIRE_CLK_O = 1;
    Sleep(2);

    // de-assert the reset/clear signal
    PIN_COUNTER_CLEAR_O = 0;
    Sleep(2);
        
} // FireTest_Clear()


void FireTest_Assert_Fire(void)
{
    if (system_is_armed)
    {
        uint8_t i = 0;
        
        for (i = 0; i < CLOCKS_TO_FIRE; i++)
        {
            PIN_FIRE_CLK_O = 0;
            Sleep(2);
            PIN_FIRE_CLK_O = 1;
        }
    }
    else
    {
        while(1)
        {
            // this should never happen - need to decide what this means
        }
    }
} // FireTest_Assert_Fire()


void FireTest_Assert_Test(void)
{
    uint8_t i = 0;
    
    for (i = 0; i < CLOCKS_TO_TEST; i++)
    {
        PIN_TEST_CLK_O = 0;
        Sleep(2);
        PIN_TEST_CLK_O = 1;
    }
} // FireTest_Assert_Test()


void Cue_Select(SOCKET_ENUM_t socket, CUE_ENUM_t cue)
{
    // make sure we don't immediately fire
    FireTest_Clear();
    
    PIN_ADDRESS_CUEb0_O = cue & BIT0 ? 1 : 0;
    PIN_ADDRESS_CUEb1_O = cue & BIT1 ? 1 : 0;
    PIN_ADDRESS_CUEb2_O = cue & BIT2 ? 1 : 0;
    PIN_ADDRESS_CUEb3_O = cue & BIT3 ? 1 : 0;
    
    PIN_ADDRESS_SOCKb0_O = socket & BIT0 ? 1 : 0;
    PIN_ADDRESS_SOCKb1_O = socket & BIT1 ? 1 : 0;
    PIN_ADDRESS_SOCKb2_O = socket & BIT2 ? 1 : 0;
    
} // Cue_Select()


bool Cue_Is_Present(void)
{
    // will store whether the cue is present or not
    bit cue_is_present = false;
    
    // assert the test signal
    FireTest_Assert_Test();
    Sleep(2);
    
    // see if the cue is not empty
    cue_is_present = !PIN_CUE_EMPTY_I;
    
    // de-assert the test signal
    FireTest_Clear();
    
    // return the result
    return cue_is_present;
    
} // Cur_Is_Present()


void Cue_Scan_All(REMOTE_t * p_remote)
{
    // will iterate through the sockets
    uint8_t socket = 0;
    
    // assume all cues empty by clearing all memory
    memset(p_remote, 0, sizeof(REMOTE_t));
    
    // iterate through the sockets
    for (socket = 0; socket < SOCKETS_NUM_OF; socket++)
    {
        // will iterate through the cues
        uint8_t cue;
        
        // iterate through the cues
        for (cue = 0; cue < CUES_NUM_OF; cue++)
        {
            // select the cue
            Cue_Select(socket, cue);
    
            // check if it is present or not
            if (Cue_Is_Present())
            {
                // it is present - set the appropriate bit
                BIT_SET(p_remote->sockets[socket], cue);
            }
            else
            {
                // it is not present - already cleared memory
            }
        } // for (cue)
    } // for (socket)
} // Cue_Scan_All()
