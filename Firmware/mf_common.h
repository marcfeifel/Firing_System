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

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

void Init_Device(void);
void interrupts(void);
void noInterrupts(void);
uint32_t millis(void);
void Sleep(uint32_t millis);

#endif // MF_COMMON_H_INCLUDED