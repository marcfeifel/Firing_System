#include "fb_messages.h"

#include <stdio.h>

bool System_Is_Armed(void);

void Msg_Ping(uint8_t dest, uint32_t timeout_ms)
{
    // the message
    FB_MSG_PING_t ping = {0};

    // note when we are starting the transmission
    uint32_t ping_tx_ms;

    printf("Ping remote-%d... ", (uint16_t)(dest - NODEID_REMOTE_BASE));
    
    ping_tx_ms = millis();
    
    // send the message
    Msg_Send(FB_MSG_PING, dest, &ping, sizeof(ping), 0);
    
    // wait for a while to get a response
    if (Msg_Run(timeout_ms))
    {
        uint32_t pong_rx_ms = millis();
        
        FB_MSG_BASE_t * p_base = (FB_MSG_BASE_t*)Msg_Get_Payload_Ptr();

        // if the message is for me, and it's from the right destination...
        if (Msg_Is_For_Me() && (Msg_Get_Sender_ID() == dest) && (p_base->id == FB_MSG_PONG))
        {
            printf("Pong - Time: %3dms, RxRSSI: %4dBm, TxRSSI: %4dBm.\r\n", (uint16_t)(pong_rx_ms - ping_tx_ms), p_base->rssi, Msg_Get_RSSI());

        }
        else
        {
            Msg_Unexpected_Message();
            
        }
    } 
    else
    {
        // we didn't receive a message back
        printf("Silence.\r\n");
    }
} // Msg_Ping()


void Msg_Remote_Status_Request(uint8_t dest, uint32_t timeout_ms)
{
    // the message
    FB_MSG_REMOTE_STATUS_REQUEST_t status_request = {0};

    printf("Remote-%d status: ", Remote_NodeID_to_Integer(dest));
    
    // send the message
    Msg_Send(FB_MSG_REMOTE_STATUS_REQUEST, dest, &status_request, sizeof(status_request), 0);
    
    // wait for a while to get a response
    if (Msg_Run(timeout_ms))
    {
        FB_MSG_BASE_t * p_base = (FB_MSG_BASE_t*)Msg_Get_Payload_Ptr();

        // if the message is for me, and it's from the right destination...
        if (Msg_Is_For_Me() && (Msg_Get_Sender_ID() == dest) && (p_base->id == FB_MSG_REMOTE_STATUS_RESPONSE))
        {
            FB_MSG_REMOTE_STATUS_RESPONSE_t * p_status_response = (FB_MSG_REMOTE_STATUS_RESPONSE_t*)p_base;
            
            printf("0x%02X, RT: %7.2f, ST: %7.2f, NC: %6.2f\r\n", 
                   (uint16_t)p_status_response->status_word,
                   p_status_response->remote_time_ms / 1000.0f,
                   p_status_response->show_time_ms / 1000.0f,
                   p_status_response->time_to_next_cue_ms / 1000.0f);
            

        }
        else
        {
            Msg_Unexpected_Message();
            
        }
    } 
    else
    {
        // we didn't receive a message back
        printf("Silence.\r\n");
    }
} // Msg_Remote_Status_Request()


void Msg_Remote_Reset(uint8_t dest, uint32_t timeout_ms)
{
    FB_MSG_REMOTE_RESET_t reset = {0}; 
    
    if (Msg_Send(FB_MSG_REMOTE_RESET, dest, &reset, sizeof(reset), timeout_ms))
    {
        printf("Reset sent to remote-%d... ACK received.\r\n", Remote_NodeID_to_Integer(dest));
        
    }
    else
    {
        printf("Reset sent to remote-%d... ACK not received.\r\n", Remote_NodeID_to_Integer(dest));
    }
} // Msg_Remote_Reset()


void Msg_Remote_Arm(uint8_t dest, uint32_t timeout_ms)
{
    FB_MSG_REMOTE_ARM_t arm = {0}; 
    
    arm.arm_key = FB_REMOTE_ARMED_MAGIC_NUMBER;
    
    if (Msg_Send(FB_MSG_REMOTE_ARM, dest, &arm, sizeof(arm), timeout_ms))
    {
        printf("Arm sent to remote-%d... ACK received.\r\n", Remote_NodeID_to_Integer(dest));
        
    }
    else
    {
        printf("Arm sent to remote-%d... ACK not received.\r\n", Remote_NodeID_to_Integer(dest));
    }
} // Msg_Remote_Arm()


void Msg_Keep_Alive(uint8_t dest)
{
    FB_MSG_KEEP_ALIVE_t keep_alive = {0};
    
    keep_alive.status_word |= System_Is_Armed() ? (STATUS_ARMED_KEY | STATUS_ARMED_KEEP_ALIVE) : 0;
    
    Msg_Send(FB_MSG_KEEP_ALIVE, dest, &keep_alive, sizeof(keep_alive), 0);
    
} // Msg_Keep_Alive()


void Msg_Remote_Disarm(uint8_t dest, uint32_t timeout_ms)
{
    FB_MSG_REMOTE_DISARM_t arm = {0}; 
    
    if (Msg_Send(FB_MSG_REMOTE_DISARM, dest, &arm, sizeof(arm), timeout_ms))
    {
        printf("Disarm sent to remote-%d... ACK received.\r\n", Remote_NodeID_to_Integer(dest));
        
    }
    else
    {
        printf("Disarm sent to remote-%d... ACK not received.\r\n", Remote_NodeID_to_Integer(dest));
    }
} // Msg_Remote_Disarm()


void Msg_Unexpected_Message(void)
{
    FB_MSG_BASE_t * p_base = (FB_MSG_BASE_t*)Msg_Get_Payload_Ptr();
    
    printf("Received unexpected message ID %d from %f.\r\n", p_base->id, Remote_NodeID_to_Integer(Msg_Get_Sender_ID()));
    
} // Msg_Unexpected_Message()
