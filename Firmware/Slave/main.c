#include "mf_common.h"
#include "rfm69.h"

void Cue_Select(uint8_t socket, uint8_t cue);
bool Cue_Is_Present(void);
void Cue_Fire(void);

void main(void)
{
    Init_Device();
    
    // put the HopeRF into reset
    PIN_RFM69HW_RST_O = 1;
    PIN_RFM69HW_RST_O = 0;
    
    RFM69_construct(true);
    RFM69_initialize();

    while (1)
    {
/*        uint8_t i = 0;
        for (i = 1; i < 2; i++)
        {
            uint8_t j;
            for (j = 0; j < 16; j++)
            {
                Cue_Select(i, j);
        
                Sleep(10);
                
                if (Cue_Is_Present())
                {
                    Sleep(10);
                }
                else
                {
                    // do nothing
                }
            }
        }
        Cue_Select(0, 0);*/
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
