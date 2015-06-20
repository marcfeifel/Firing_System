#ifndef MF_COMMON_H_INCLUDED
#define MF_COMMON_H_INCLUDED

#include "mf_c51_target.h"

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef char  int8_t;
typedef short int16_t;
typedef long  int32_t;
typedef bit bool;

#define HIGH 1
#define LOW 0
#define false 0
#define true (!false)

typedef enum
{
    KEYSWITCH_STATE_OFF,
    KEYSWITCH_STATE_ARMED,
    KEYSWITCH_STATE_TEST,
    KEYSWITCH_STATE_FAULT,
    
} KEYSWITCH_STATE_t;

void Init_Device(void);
KEYSWITCH_STATE_t Get_Keyswitch_State(void);
void interrupts(void);
void noInterrupts(void);
uint32_t millis(void);

#endif // MF_COMMON_H_INCLUDED