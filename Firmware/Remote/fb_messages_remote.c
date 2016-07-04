#include "fb_messages.h"

#include "fb_remote.h"
#include "fb_remote_program_run.h"
#include "fb_remote_cue.h"

void Msg_Disbatch(void)
{
    if (Msg_Received())
    {
        FB_MSG_BASE_t * p_base = (FB_MSG_BASE_t*)Msg_Get_Payload_Ptr();
        
        if (Msg_Is_For_Me())
        {
            // private messages
            switch (p_base->id)
            {
                case FB_MSG_REMOTE_RESET:
                    Msg_Remote_Reset_Handle();
                    break;
                
                case FB_MSG_PING:
                    Msg_Ping_Handle();                    
                    break;
                
                case FB_MSG_REMOTE_ARM:
                    Msg_Remote_Arm_Handle();
                    break;
                
                case FB_MSG_REMOTE_DISARM:
                    Msg_Remote_Disarm_Handle();
                    break;
                
                case FB_MSG_REMOTE_STATUS_REQUEST:
                    Msg_Remote_Status_Request_Handle();
                    break;
                
                case FB_MSG_REMOTE_PROGRAM_COMMIT:
                    Msg_Remote_Program_Commit_Handle();
                    break;
                
                case FB_MSG_REMOTE_PROGRAM:
                    Msg_Remote_Program_Handle();
                    break;
                
                case FB_MSG_REMOTE_SCAN_REQUEST:
                    Msg_Remote_Scan_Request_Handle();
                    break;
                
                default:
                    break;
            }
        }
        else
        {
            // broadcast messages
            switch (p_base->id)
            {
                case FB_MSG_KEEP_ALIVE:
                    Msg_Keep_Alive_Handle();
                    break;
                
                default:
                    break;
            }
        }        
    }
} // Msg_Disbatch()


void Msg_Keep_Alive_Handle(void)
{
    FB_MSG_KEEP_ALIVE_t * keep_alive = (FB_MSG_KEEP_ALIVE_t*)Msg_Get_Payload_Ptr();
    
    if (keep_alive->status_word & STATUS_ARMED_KEEP_ALIVE)
    {
        fb_Remote_Set_Armed_Flag(true);
    }
    else
    {
        fb_Remote_Set_Armed_Flag(false);
    }
    
    if (keep_alive->status_word & STATUS_SHOW_IS_RUNNING)
    {
        fb_Remote_Program_Run_Set_Run(true);
    }
    else
    {
        fb_Remote_Program_Run_Set_Run(false);
    }
    
    fb_Remote_Program_Run_Update_Show_Time(keep_alive->show_time_ms);
    
    fb_Keep_Alive_Received();
    
} // Msg_Keep_Alive_Handle()


void Msg_Ping_Handle(void)
{
    // received a ping - respond with a pong
    FB_MSG_PONG_t pong = {0};
    
    Msg_Send(FB_MSG_PONG, Msg_Get_Sender_ID(), &pong, sizeof(pong), false);
    
} // Msg_Ping_Handle()


void Msg_Remote_Reset_Handle(void)
{
    Msg_ACK();
    
    Reset_MCU();
    
} // Msg_System_Reset_Handle()


void Msg_Remote_Arm_Handle(void)
{
    FB_MSG_REMOTE_ARM_t * arm = (FB_MSG_REMOTE_ARM_t*)Msg_Get_Payload_Ptr();
    
    Msg_ACK();
    
    fb_Remote_Set_Arming_Key(arm->arm_key);
    
} // Msg_System_Arm_Handle()


void Msg_Remote_Disarm_Handle(void)
{
    Msg_ACK();
    
    fb_Remote_Disarm();
    
} // Msg_System_Disarm_Handle()


void Msg_Remote_Status_Request_Handle(void)
{
    FB_MSG_REMOTE_STATUS_RESPONSE_t status_response = {0};
    
    status_response.remote_time_ms = millis();
    status_response.show_time_ms = fb_Remote_Program_Run_Get_Show_Time_ms();
    
    do
    {
        uint8_t next_pin;
        uint32_t time_to_next_pin_ms;
        uint8_t cues_remaining;
    
        fb_Remote_Program_Run_To_Next_Cue_Info(&time_to_next_pin_ms,
                                               &next_pin,
                                               &cues_remaining);
        
        status_response.time_to_next_cue_ms = time_to_next_pin_ms;
        status_response.next_cue = GET_CUE(next_pin);
        status_response.next_socket = GET_SOCKET(next_pin);
        status_response.cues_remaining = cues_remaining;
        
    } while (0);

    status_response.status_word |= fb_Remote_Arming_Flag_Is_Set()     ? STATUS_ARMED_KEEP_ALIVE : 0;
    status_response.status_word |= fb_Remote_Arming_Key_Is_Set()      ? STATUS_ARMED_KEY        : 0;
    status_response.status_word |= fb_Remote_Program_Run_Is_Running() ? STATUS_SHOW_IS_RUNNING  : 0;
    
    Msg_Send(FB_MSG_REMOTE_STATUS_RESPONSE, Msg_Get_Sender_ID(), &status_response, sizeof(status_response), false);

} // Msg_Remote_Status_Request_Handle()


void Msg_Remote_Scan_Request_Handle(void)
{
    FB_MSG_REMOTE_SCAN_RESPONSE_t scan_response = {0};
    
    fb_Remote_Cue_Scan_All(&scan_response.cues);
    
    Msg_Send(FB_MSG_REMOTE_SCAN_RESPONSE, Msg_Get_Sender_ID(), &scan_response, sizeof(scan_response), false);

} // Msg_Remote_Status_Request_Handle()
