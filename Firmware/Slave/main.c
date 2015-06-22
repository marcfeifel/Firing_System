#include "mf_common.h"
#include "rfm69.h"

void Cue_Select(uint8_t socket, uint8_t cue);
bool Cue_Is_Present(void);
void Cue_Fire(void);

#define NUM_SOCKETS 8
#define NUM_CUES    16

static xdata uint8_t cues[NUM_SOCKETS][NUM_CUES] = { 0 };

void main(void)
{
    Init_Device();
    
    // put the HopeRF into reset
    PIN_RFM69HW_RST_O = 1;
    PIN_RFM69HW_RST_O = 0;

    {
        uint8_t i = 0;
        
        // take the ripple-counters out of reset
        PIN_COUNTER_CLEAR_O = 1;
        Sleep(5);
        PIN_COUNTER_CLEAR_O = 0;
        Sleep(5);
        
        for (i = 0; i < 8; i++)
        {
            PIN_TEST_CLK_O = 1;
            Sleep(5);
            PIN_TEST_CLK_O = 0;
            Sleep(5);
        }
    }        
    
    RFM69_construct(true);
    RFM69_initialize();

    while (1)
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


void Cue_Select(uint8_t socket, uint8_t cue)
{
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


void Cue_Fire(void)
{
} // Cue_Fire()
