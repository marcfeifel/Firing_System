#ifndef FB_MESSAGES_H_INCLUDED
#define FB_MESSAGES_H_INCLUDED

#include "fb_common.h"

typedef enum
{
    FB_MSG_REMOTE_RESET,
    FB_MSG_REMOTE_SYNC,
    FB_MSG_REMOTE_ARM,
    FB_MSG_REMOTE_DISARM,
    
    FB_MSG_PING,
    FB_MSG_PONG,

    FB_MSG_REMOTE_STATUS_REQUEST,
    FB_MSG_REMOTE_STATUS_RESPONSE,
    
    FB_MSG_KEEP_ALIVE,
    
//    FB_MSG_CMD_SCAN_ALL_CUES,
//    FB_MSG_RESP_SCAN_ALL_CUES,

//    FB_MSG_CMD_FIRE_PROGRAM,

    FB_MSG_NUM_OF

} FB_MSG_ID_ENUM_t;


typedef struct
{
    FB_MSG_ID_ENUM_t id;
    
    uint16_t rssi;
    
} FB_MSG_BASE_t;


// reset remotes
typedef FB_MSG_BASE_t FB_MSG_REMOTE_RESET_t;

// sync
//typedef FB_MSG_BASE_t FB_MSG_REMOTE_SYNC_t;

// ping and pong
typedef struct
{
    FB_MSG_BASE_t base;
    
    uint32_t ping_time_ms;
    
} FB_MSG_PING_t;
typedef FB_MSG_PING_t FB_MSG_PONG_t;

// arm the system
typedef struct
{
    FB_MSG_BASE_t base;
    
    uint32_t arm_key;
} FB_MSG_REMOTE_ARM_t;  // command the system to arm

// disarm the system
typedef FB_MSG_BASE_t FB_MSG_REMOTE_DISARM_t;

typedef struct
{
    FB_MSG_BASE_t base;
    
    status_word_t status_word;
} FB_MSG_KEEP_ALIVE_t;

// scan all cues and response
/*typedef FB_MSG_BASE_t FB_MSG_CMD_SCAN_ALL_CUES_t;
typedef struct
{
    FB_MSG_BASE_t   base;

    REMOTE_CUES_t   cues_present;

} FB_MSG_RESP_SCAN_ALL_CUES_t;*/


// single-cue firing command and response
/*typedef FB_MSG_CUE_t FB_MSG_CMD_FIRE_CUE_t;  // issue the command to fire
typedef FB_MSG_CUE_t FB_MSG_RESP_FIRE_CUE_t; // response to the command to fire
typedef FB_MSG_CUE_t FB_MSG_CONF_FIRE_CUE_t; // confirm command to fire

typedef FB_MSG_BASE_t FB_MSG_CMD_FIRE_PROGRAM_t;*/

// a cue was fired
//typedef FB_MSG_CUE_t FB_MSG_CUE_FIRED_t;

// keep-alive and set-time commands
// - keep-alive commands must be within some tolerance of the system time and are only
// relevant during shows (they are intended to correct drift)
// - set-time commands are arbitrary but can only occur outside the scope of a running show
// - response to set-time should respond with the new local time such that the master can verify
// the time was set correctly
/*typedef struct
{
    FB_MSG_BASE_t   base;

    uint32_t        time_ms;

} FB_MSG_KEEP_ALIVE_t, FB_MSG_CMD_SET_TIME_t, FB_MSG_RESP_SET_TIME_t;*/

typedef struct
{
    FB_MSG_BASE_t  base;
    
    uint32_t remote_time_ms;
    
    uint32_t show_time_ms;
    
    uint32_t time_to_next_cue_ms;
    
    status_word_t status_word;
    
} FB_MSG_REMOTE_STATUS_RESPONSE_t;
typedef FB_MSG_BASE_t FB_MSG_REMOTE_STATUS_REQUEST_t;

void Msg_Init(void);
bool Msg_Run(uint32_t timeout_ms);
bool Msg_Received(void);
bool Msg_Is_For_Me(void);
uint8_t Msg_Get_Sender_ID(void);
uint8_t Msg_Get_Payload_Size(void);
void const * Msg_Get_Payload_Ptr(void);
int16_t Msg_Get_RSSI(void);
bool Msg_Send(FB_MSG_ID_ENUM_t id, uint8_t dest, void const * payload, uint8_t payload_size, uint32_t retry_timeout_ms);
void Msg_ACK(void);

void Msg_Disbatch(void);

void Msg_Keep_Alive(uint8_t dest);
void Msg_Keep_Alive_Handle(void);

void Msg_Ping(uint8_t dest, uint32_t timeout_ms);
void Msg_Ping_Handle(void);

void Msg_Remote_Reset(uint8_t dest, uint32_t timeout_ms);
void Msg_Remote_Reset_Handle(void);

void Msg_Remote_Arm(uint8_t dest, uint32_t timeout_ms);
void Msg_Remote_Arm_Handle(void);

void Msg_Remote_Disarm(uint8_t dest, uint32_t timeout_ms);
void Msg_Remote_Disarm_Handle(void);

void Msg_Remote_Status_Request(uint8_t dest, uint32_t timeout_ms);
void Msg_Remote_Status_Request_Handle(void);

void Msg_Unexpected_Message(void);

#endif // FB_MESSAGE_H_INCLUDED