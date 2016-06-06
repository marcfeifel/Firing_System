#include "fb_common.h"
#include "fb_crc.h"

#define CRC0SEL_SET() CRC0CN |= BIT4
#define CRC0SEL_CLR() CRC0CN &= ~BIT4
#define CRC0INIT_SET() CRC0CN |= BIT3
#define CRC0INIT_CLR() CRC0CN &= ~BIT3
#define CRC0VAL_SET() CRC0CN |= BIT2
#define CRC0VAL_CLR() CRC0CN &= ~BIT2
#define CRC0PNT_CLR() CRC0CN &= ~(BIT1 | BIT0)

CRC16_t fb_CRC_Compute(void const * p_in, uint16_t len, CRC16_t starting_crc)
{
    // will store the computed crc
    CRC16_t ending_crc;

    // pointer to the incoming data
    uint8_t const * p_data = p_in;

    // store the previous SFRPAGE
    const uint16_t SFRPAGE_prev = SFRPAGE;

    // set the CRC SFR page
    SFRPAGE = CRC0_PAGE;

    // set for 16-bit CRC
    CRC0SEL_SET();

    // clear the data-out pointer
    CRC0PNT_CLR();

    // write the initial CRC
    CRC0DAT = starting_crc & 0xFF;
    CRC0DAT = (starting_crc >> 8) & 0xFF;

    // loop through the data
    while (len--)
    {
        // incorporate the next byte
        CRC0IN = *p_data++;

    } // while (len--)

    // get the new CRC
    ending_crc = CRC0DAT;
    ending_crc |= (uint16_t)CRC0DAT << 8;

    // restore the SFR page
    SFRPAGE = SFRPAGE_prev;

    // return the value
    return ending_crc;

}
