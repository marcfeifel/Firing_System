#ifndef FB_COMMON_H_INCLUDED
#define FB_COMMON_H_INCLUDED

#include "fb_c51_target.h"

#define NODEID_MASTER  2
#define NODEID_REMOTE0 3
//#define NODEID_REMOTE1 4
//#define NODEID_REMOTE2 5
//#define NODEID_REMOTE3 6

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

typedef uint8_t encoded_cue_t;

#define ENCODE_CUE(_socket, _cue) (((_socket) << 4) & 0xF)) | ((_cue) & 0x0F))
#define GET_CUE(_encoded_cue)     ((_encoded_cue) & 0x0F)
#define GET_SOCKET(_encoded_cue)  (((_encoded_cue) >> 4) & 0x0F)

// structure which holds a single bit about a socket's cues
typedef uint16_t SOCKET_t;

// structure which holds a single bit about a remote module's sockets' cues
typedef struct
{
    SOCKET_t sockets[SOCKETS_NUM_OF];

} REMOTE_CUES_t;


void Init_Device(void);

char _kbhit(void);

void interrupts(void);
void noInterrupts(void);

bool millis_expired(uint32_t timer_ms);
uint32_t millis(void);

void Sleep(uint32_t millis);

bool Ping(uint32_t target, uint32_t timeout_ms, uint16_t * p_response_time_ms, uint16_t * p_rssi);
void Ping_Handler(void);
void Pong_Handler(void);

void Disbatcher(void);
void Pong(void);

#endif // FB_COMMON_H_INCLUDED