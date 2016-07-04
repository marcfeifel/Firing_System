#include "fb_common.h"
#include "fb_flash.h"
#include "fb_crc.h"

#include "fb_messages.h"

// first block used
#define FB_PROGRAM_FLASH_BLOCK_FIRST    DEVICE_FLASH_BLOCK_NUM(FB_PROGRAM_FLASH_ADDRESS)

// number of blocks used
#define FB_PROGRAM_FLASH_BLOCKS_NUM_OF  DEVICE_FLASH_MIN_BLOCKS(512)

// size of the area allocated, in bytes
#define FB_PROGRAM_FLASH_SIZE           (FB_PROGRAM_FLASH_BLOCKS_NUM_OF * DEVICE_BLOCK_SIZE)

// base address of the area allocated
#define FB_PROGRAM_FLASH_ADDRESS        (DEVICE_FLASH_SIZE_USABLE - 1024)

// allocate the space in flash
LOCATED_VARIABLE_NO_INIT(the_program_raw[1024], uint8_t, SEG_CODE, FB_PROGRAM_FLASH_ADDRESS);

// pointer used to read the program - a pointer into flash
VARIABLE_SEGMENT_POINTER(p_firing_times_ms_flash, uint32_t, SEG_CODE) = &the_program_raw;

static uint32_t m_firing_times_ms[128] = {0};


void fb_Remote_Program_Load(uint32_t * p_firing_times_ms)
{
    uint8_t pin;
    
    for (pin = 0; pin < 128; pin++)
    {
        p_firing_times_ms[pin] = p_firing_times_ms_flash[pin];
        
    }
} // fb_Remote_Program_Load()


void Msg_Remote_Program_Handle(void)
{
    FB_MSG_REMOTE_PROGRAM_t * program = (FB_MSG_REMOTE_PROGRAM_t*)Msg_Get_Payload_Ptr();
    
    m_firing_times_ms[program->pin] = program->firing_time_ms;

    Msg_ACK();

} // Msg_Remote_Program_Handle()


void Msg_Remote_Program_Commit_Handle(void)
{
    bool success = true;
    
    uint8_t pin;
    
    fb_Flash_Erase_Blocks(FB_PROGRAM_FLASH_BLOCK_FIRST, FB_PROGRAM_FLASH_BLOCKS_NUM_OF);

    fb_Flash_Write(the_program_raw, m_firing_times_ms, 512);

    for (pin = 0; pin < 128; pin++)
    {
        if (m_firing_times_ms[pin] != p_firing_times_ms_flash[pin])
            success = false;
        
    }
    
    if (success)
        Msg_ACK();
    
    Reset_MCU();
    
} // Msg_Remote_Program_Commit_Handle()