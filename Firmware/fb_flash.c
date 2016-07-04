#include "fb_flash.h"

#include "fb_common.h"

// PSCTL flash-control bits
#define SFLE_SET() PSCTL |= BIT2
#define SFLE_CLR() PSCTL &= ~BIT2
#define SFLE_WRITE(_val) do { _val ? SFLE_SET() : SFLE_CLR(); } while (0)
#define PSEE_SET() PSCTL |= BIT1
#define PSEE_CLR() PSCTL &= ~BIT1
#define PSWE_SET() PSCTL |= BIT0
#define PSWE_CLR() PSCTL &= ~BIT0

// flash unlock macro
#define FLKEY_UNLOCK() do { FLKEY = 0xA5; FLKEY = 0xF1; } while (0)

void fb_Flash_Erase_Blocks(uint16_t first_block, uint16_t num_blocks)
{
    // pointer used to write/trigger the erase
    SEGMENT_VARIABLE_SEGMENT_POINTER(p_erase, uint8_t, SEG_XDATA, SEG_DATA) = first_block * DEVICE_BLOCK_SIZE;

    // in order to ensure that we are future-proof with devices which have 512B blocks, lets make sure
    // we actually erase twice as many half-sized blocks
    // each 1k block will get erased twice, but other than taking a bit longer, this has no real impact
    num_blocks <<= 1;

    // loop through the blocks
    while (num_blocks--)
    {
        // disable interrupts
        IE = 0;

        // enable the erase function
        PSEE_SET();
        PSWE_SET();

        // unlock the flash erase/write functions
        FLKEY_UNLOCK();

        // trigger the erase
        *p_erase = 0;

        // disable the flash erase/write functions
        PSWE_CLR();
        PSEE_CLR();

        // restore the previos interrupt state
        IE = 1;

        // because we are erasing in 512B chunks for future-proofness,
        // need to actually increment by half-blocks
        p_erase += (DEVICE_BLOCK_SIZE >> 1);

    } // while (num_blocks--)
} // fb_Flash_Erase_Blocks()


void fb_Flash_Write(void * p_dest, void * p_src, uint32_t len)
{
    // pointer used to write to the destination - a pointer located in data, into xdata
    SEGMENT_VARIABLE_SEGMENT_POINTER(p_write, uint8_t, SEG_XDATA, SEG_DATA) = p_dest;

    // pointer used to read from the source
    uint8_t * p_read = p_src;

    // loop through len times
    while (len)
    {
        // get the byte and store it in the data segment, and update the pointer
        SEGMENT_VARIABLE(byte_to_commit, uint8_t, SEG_XDATA) = *(p_read++);

        // disable interrupts
        IE = 0;

        // enable the write function
        PSWE_SET();

        // unlock the flash erase/write functions
        FLKEY_UNLOCK();

        // performt the write
        *(uint8_t*)p_write = byte_to_commit;

        // disable the write function
        PSWE_CLR();

        // restore the previos interrupt state
        IE = 1;

        // move to the next byte (don't update this variable during the flash write - bad things could happen)
        p_write++;

        // update the counter
        len--;

    } // while (len)
} // fb_Flash_Write()
