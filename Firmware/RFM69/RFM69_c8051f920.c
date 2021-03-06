#include "fb_common.h"
#include "fb_c51_target.h"

uint8_t RFM69_SPI_transfer(uint8_t out)
{
    // clear the interrupt flag
    SPIF0 = 0;
    
    // write the address
    SPI0DAT = out;

    while (!SPIF0)
    {
        // wait for the xfer to complete
    }

    return SPI0DAT;
}


// select the RFM69 transceiver (save SPI settings, set CS low)
void RFM69_SPI_select() {
    NSS0MD0 = LOW;
}


// unselect the RFM69 transceiver (set CS high, restore SPI settings)
void RFM69_SPI_unselect() {
    NSS0MD0 = HIGH;
}

void RFM69_assert_reset() {
    PIN_RFM69HW_RST_O = 1;
}

void RFM69_deassert_reset() {
    PIN_RFM69HW_RST_O = 0;
}
