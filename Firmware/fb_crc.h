#ifndef FB_CRC_H_INCLUDED
#define FB_CRC_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "fb_common.h"
    
#define NEW_CRC_INIT 0xFFFF
    
typedef uint16_t CRC16_t;

CRC16_t fb_CRC_Compute(void const * p_in, uint16_t len, CRC16_t starting_crc);
    
    
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FB_CRC_H_INCLUDED
