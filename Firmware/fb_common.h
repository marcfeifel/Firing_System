#ifndef FB_COMMON_H_INCLUDED
#define FB_COMMON_H_INCLUDED

#include "compiler_defs.h"

// definition for the master device
//#define NODEID_LOCAL    NODEID_MASTER
#define NETWORKID 1

#define NODEID_BROADCAST     RF69_BROADCAST_ADDR
#define NODEID_MASTER        2

#define NODEID_REMOTE_BASE   3
#define NODEID_REMOTE0       NODEID_REMOTE_BASE
#define NODEID_REMOTE1       (NODEID_REMOTE0 + 1)
#define NODEID_REMOTE2       (NODEID_REMOTE1 + 1)
#define NODEID_REMOTE3       (NODEID_REMOTE2 + 1)
#define NODEID_REMOTE4       (NODEID_REMOTE3 + 1)
#define NODEID_REMOTE5       (NODEID_REMOTE4 + 1)
#define NODEID_REMOTE_LAST   NODEID_REMOTE5

// the number of firing 'pins' per remote
#define PINS_NUM_OF    (SOCKETS_NUM_OF * CUES_NUM_OF)

#define FB_REMOTE_ARMED_MAGIC_NUMBER 0x12345678

#define HIGH 1
#define LOW 0

#define BIT(_bit) (1 << (_bit))
#define BIT_SET(_val, _bit) ((_val) |= BIT(_bit))
#define BIT_CLR(_val, _bit) ((_val) &= ~BIT(_bit))
#define BIT_IS_SET(_val, _bit) ((_val) & BIT(_bit))
#define BIT_IS_CLR(_val, _bit) (!BIT_IS_SET(_val, _bit))

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

typedef enum
{
    CUE0,
    CUE1,
    CUE2,
    CUE3,
    CUE4,
    CUE5,
    CUE6,
    CUE7,
    CUE8,
    CUE9,
    CUE10,
    CUE11,
    CUE12,
    CUE13,
    CUE14,
    CUE15,
    CUES_NUM_OF

} CUE_ENUM_t;

typedef enum
{
    SOCKET0,
    SOCKET1,
    SOCKET2,
    SOCKET3,
    SOCKET4,
    SOCKET5,
    SOCKET6,
    SOCKET7,
    SOCKETS_NUM_OF

} SOCKET_ENUM_t;


#define STATUS_ARMED_KEY        0x01 // the "key" is armed
#define STATUS_ARMED_KEEP_ALIVE 0x02 // the keep-alive is armed
#define STATUS_SHOW_IS_RUNNING  0x04 // a show is being fired/running
typedef uint8_t status_word_t;

typedef uint8_t encoded_cue_t;

#define MAKE_PIN(_socket, _cue)   ((((_socket) & 0x07) << 4) | ((_cue) & 0x0F))
#define GET_CUE(_pin)     ((_pin) & 0x0F)
#define GET_SOCKET(_pin)  (((_pin) >> 4) & 0x07)

#define CUE_EMPTY 0xFFFFFFFF

// structure which holds a single bit about a socket's cues
typedef uint16_t SOCKET_t;

// structure which holds a single bit about a remote module's sockets' cues
typedef struct
{
    SOCKET_t sockets[SOCKETS_NUM_OF];

} REMOTE_CUES_t;


void Init_Device(void);

char _kbhit(void);

bool millis_expired(uint32_t timer_ms);
uint32_t millis(void);

void Sleep(uint32_t millis);

void Reset_MCU(void);

uint16_t Remote_NodeID_to_Integer(uint8_t nodeid);

void Task_1ms_Low_Priority_Handler(void);

void Task_1ms_High_Priority_Handler(void);
void Task_1ms_High_Priority_Disable(void);
void Task_1ms_High_Priority_Enable(void);

#endif // FB_COMMON_H_INCLUDED