#ifndef FB_FLASH_H_INCLUDED
#define FB_FLASH_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    
#include "fb_common.h"

#define DEVICE_BLOCK_SIZE          1024
#define DEVICE_FLASH_SIZE_TOTAL    32768
#define DEVICE_FLASH_SIZE_USABLE   (DEVICE_FLASH_SIZE_TOTAL - DEVICE_BLOCK_SIZE) // don't use the last block, as it is protected
#define DEVICE_NUM_BLOCKS          (DEVICE_FLASH_SIZE_USABLE / DEVICE_BLOCK_SIZE)
#define DEVICE_FLASH_ADDRESS_START 0x0000
#define DEVICE_FLASH_ADDRESS_END   ((DEVICE_FLASH_ADDRESS_START - DEVICE_FLASH_SIZE) - 1)
    
#define DEVICE_FLASH_BLOCK_NUM(_address) ((_address) / DEVICE_BLOCK_SIZE)      // get the block an address is located in
#define DEVICE_FLASH_ADDRESS(_block_num) ((_block_num) * DEVICE_BLOCK_SIZE)    // get the base-address of a block
#define DEVICE_FLASH_MIN_BLOCKS(_bytes)  (DEVICE_FLASH_BLOCK_NUM(_bytes) + 1) // minimum number of blocks needed to store _bytes bytes
    
void fb_Flash_Erase_Blocks(uint16_t first_block, uint16_t num_blocks);
void fb_Flash_Write(void * p_dest, void * p_src, uint32_t len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FB_FLASH_H_INCLUDED
    