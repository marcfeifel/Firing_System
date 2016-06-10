#include "fb_common.h"
#include "fb_flash.h"
#include "fb_crc.h"

#define FB_PROGRAM_NAME_LEN      64

typedef struct _FB_PROGRAM_t
{
    uint8_t name[FB_PROGRAM_NAME_LEN];

    struct
    {
        // time the element is to be fired
        uint32_t      firing_time_ms;

        // cue to be fired
        encoded_cue_t encoded_cue;

    } elements[CUES_NUM_OF * SOCKETS_NUM_OF];

    CRC16_t crc;

} FB_PROGRAM_t;

// first block used
#define FB_PROGRAM_FLASH_BLOCK_FIRST    DEVICE_FLASH_BLOCK_NUM(FB_PROGRAM_FLASH_ADDRESS)

// number of blocks used
#define FB_PROGRAM_FLASH_BLOCKS_NUM_OF  DEVICE_FLASH_MIN_BLOCKS(sizeof(FB_PROGRAM_t))

// size of the area allocated, in bytes
#define FB_PROGRAM_FLASH_SIZE           (FB_PROGRAM_FLASH_BLOCKS_NUM_OF * DEVICE_BLOCK_SIZE)

// base address of the area allocated
#define FB_PROGRAM_FLASH_ADDRESS        (DEVICE_FLASH_SIZE_USABLE - FB_PROGRAM_FLASH_SIZE)

// allocate the space in flash
LOCATED_VARIABLE_NO_INIT(the_program_raw[FB_PROGRAM_FLASH_SIZE], uint8_t, SEG_CODE, FB_PROGRAM_FLASH_ADDRESS);

// pointer used to read the program - a pointer into flash
VARIABLE_SEGMENT_POINTER(p_the_program_read, FB_PROGRAM_t, SEG_CODE) = &the_program_raw;

void blah(void)
{
    uint32_t dummy_first = *(uint32_t*)p_the_program_read;
    dummy_first++;

    fb_Flash_Erase_Blocks(FB_PROGRAM_FLASH_BLOCK_FIRST, FB_PROGRAM_FLASH_BLOCKS_NUM_OF);

    fb_Flash_Write(the_program_raw, &dummy_first, sizeof(dummy_first));

    while (dummy_first)
    {
        dummy_first++;
    }
}
