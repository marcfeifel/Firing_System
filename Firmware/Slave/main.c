#include "mf_common.h"
#include "rfm69.h"

void Cue_Select(uint8_t socket, uint8_t cue);
bool Cue_Is_Present(void);
void Cue_Fire(void);

#define NUM_SOCKETS 8
#define NUM_CUES    16

#define CLOCKS_TO_TEST 8
#define CLOCKS_TO_FIRE 8

static uint8_t cues[NUM_SOCKETS][NUM_CUES] = { 0 };

static bit system_is_armed = true;

void main(void)
{
    // initialize the C8051F920
    Init_Device();
    
    // initialize the HopeRF radio
    RFM69_construct(true);
    RFM69_initialize();

    // super loop
    while (true)
    {
        uint8_t socket = 0;
        for (socket = 0; socket < NUM_SOCKETS; socket++)
        {
            uint8_t cue;
            for (cue = 0; cue < NUM_CUES; cue++)
            {
                Cue_Select(socket, cue);
        
                Sleep(10);
                
                if (Cue_Is_Present())
                {
                    cues[socket][cue] = 1;
                }
                else
                {
                    // do nothing
                }
            }
        }
    }
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
            PIN_FIRE_CLK_O = 1;
            Sleep(5);
            PIN_FIRE_CLK_O = 0;
            Sleep(5);
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
        PIN_TEST_CLK_O = 1;
        Sleep(5);
        PIN_TEST_CLK_O = 0;
        Sleep(5);
    }
} // FireTest_Assert_Test()


void Cue_Select(uint8_t socket, uint8_t cue)
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
    return PIN_CUE_PRESENT_I;
    
} // Cur_Is_Present()
