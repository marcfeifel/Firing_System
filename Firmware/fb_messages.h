#ifndef FB_MESSAGES_H_INCLUDED
#define FB_MESSAGES_H_INCLUDED

#include "fb_common.h"

typedef enum
{
    FB_MSG_PING,
    FB_MSG_PONG,

    FB_MSG_ACK,
    FB_MSG_NACK,

    FB_MSG_CMD_SCAN_ALL_CUES,
    FB_MSG_RESP_SCAN_ALL_CUES,

    FB_MSG_CMD_FIRE_CUE,
    FB_MSG_RESP_FIRE_CUE,

    FB_MSG_CMD_FIRE_PROGRAM,
    FB_MSG_CUE_FIRED,

    FB_MSG_NUM_OF

} FB_MSG_ID_ENUM_t;


typedef struct
{
    FB_MSG_ID_ENUM_t id;

    int16_t         rssi;

} FB_MSG_BASE_t;


typedef struct
{
    FB_MSG_BASE_t   base;

    CUE_ENUM_t      cue;

    SOCKET_ENUM_t   socket;

} FB_MSG_CUE_t;


// ping and pong
typedef FB_MSG_BASE_t FB_MSG_PING_t;
typedef FB_MSG_BASE_t FB_MSG_PONG_t;

// ack and nack
typedef FB_MSG_BASE_t FB_MSG_ACK_t;
typedef FB_MSG_BASE_t FB_MSG_NACK_t;

// arm the system
typedef FB_MSG_BASE_t FB_CMD_SYS_ARM_t;  // command the system to arm
typedef FB_MSG_BASE_t FB_RESP_SYS_ARM_t; // response request confirmation
typedef FB_MSG_BASE_t FB_CONF_SYS_ARM_t; // confirmation of command to arm
typedef FB_MSG_BASE_t FB_SYS_ARMED_t;    // system is armed

// disarm the system
typedef FB_MSG_BASE_t FB_CMD_SYS_DISARM_t;
typedef FB_MSG_BASE_t FB_RESP_SYS_DISARM_t;

// scan all cues and response
typedef FB_MSG_BASE_t FB_MSG_CMD_SCAN_ALL_CUES_t;
typedef struct
{
    FB_MSG_BASE_t   base;

    REMOTE_CUES_t   cues_present;

} FB_MSG_RESP_SCAN_ALL_CUES_t;


// single-cue firing command and response
typedef FB_MSG_CUE_t FB_MSG_CMD_FIRE_CUE_t;  // issue the command to fire
typedef FB_MSG_CUE_t FB_MSG_RESP_FIRE_CUE_t; // response to the command to fire
typedef FB_MSG_CUE_t FB_MSG_CONF_FIRE_CUE_t; // confirm command to fire

typedef FB_MSG_BASE_t FB_MSG_CMD_FIRE_PROGRAM_t;

// a cue was fired
typedef FB_MSG_CUE_t FB_MSG_CUE_FIRED_t;

// keep-alive and set-time commands
// - keep-alive commands must be within some tolerance of the system time and are only
// relevant during shows (they are intended to correct drift)
// - set-time commands are arbitrary but can only occur outside the scope of a running show
// - response to set-time should respond with the new local time such that the master can verify
// the time was set correctly
typedef struct
{
    FB_MSG_BASE_t   base;

    uint32_t        time_ms;

} FB_MSG_KEEP_ALIVE_t, FB_MSG_CMD_SET_TIME_t, FB_MSG_RESP_SET_TIME_t;


typedef struct _FB_MSG_XMIT_DESCRIPTOR
{
    uint8_t         dest;

    uint8_t         payload_size;

    void const    * payload;

    uint8_t         transmit_complete;

    struct _FB_MSG_XMIT_DESCRIPTOR * p_next_message;

} FB_MSG_XMIT_DESCRIPTOR;


void Msg_Init(void);
void Msg_Run(void);
bool Msg_Received(void);
bool Msg_Is_For_Me(void);
uint8_t Msg_Get_Sender(void);
uint8_t Msg_Get_Payload_Size(void);
void const * Msg_Get_Payload_Ptr(void);
int16_t Msg_Get_RSSI(void);
void Msg_Enqueue_for_Xmit(uint8_t dest, void const * payload, uint8_t payload_size, FB_MSG_XMIT_DESCRIPTOR * p_message);
bool Msg_Xmit_Is_Complete(FB_MSG_XMIT_DESCRIPTOR const * p_message);

#endif // FB_MESSAGE_H_INCLUDED