#include "fb_messages.h"

#include <stdio.h>

bool System_Is_Armed(void);
bool System_Show_Is_Running(void);
uint32_t System_Get_Show_Time_ms(void);

#define CON_ESC     "\x1B"
#define CON_RESET   CON_ESC "[0m"
#define CON_RED     CON_ESC "[31m"
#define CON_GREEN   CON_ESC "[32m"
#define CON_YELLOW  CON_ESC "[33m"
#define CON_BLUE    CON_ESC "[34m"
#define CON_MAGENTA CON_ESC "[35m"
#define CON_CYAN    CON_ESC "[36m"
#define CON_WHITE   CON_ESC "[37m"

#define CON_CLEAR_LINE  CON_ESC "[2K"
#define CON_SET_CURSOR  CON_ESC "[%d;%dH"

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
            printf("Pong - Time: %3dms, RxRSSI: %4dBm, TxRSSI: %4dBm.\r\n" CON_CLEAR_LINE, (uint16_t)(pong_rx_ms - ping_tx_ms), p_base->rssi, Msg_Get_RSSI());

        }
        else
        {
            Msg_Unexpected_Message();
            
        }
    } 
    else
    {
        // we didn't receive a message back
        printf(CON_RED "Silence.\r\n" CON_CLEAR_LINE CON_RESET);
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
            
            printf("0x%02X RT:%7.2fs ST:%7.2fs NC:%6.2fs s:%d c:%2d r:%3d\r\n" CON_CLEAR_LINE, 
                   (uint16_t)p_status_response->status_word,
                   p_status_response->remote_time_ms / 1000.0f,
                   p_status_response->show_time_ms / 1000.0f,
                   (int32_t)p_status_response->time_to_next_cue_ms / 1000.0f,
                   (uint16_t)p_status_response->next_socket,
                   (uint16_t)p_status_response->next_cue,
                   (uint16_t)p_status_response->cues_remaining);
            

        }
        else
        {
            Msg_Unexpected_Message();
            
        }
    } 
    else
    {
        // we didn't receive a message back
        printf(CON_RED "Silence.\r\n" CON_CLEAR_LINE CON_RESET);
    }
} // Msg_Remote_Status_Request()


void Msg_Remote_Reset(uint8_t dest, uint32_t timeout_ms)
{
    FB_MSG_REMOTE_RESET_t reset = {0}; 
    
    if (Msg_Send(FB_MSG_REMOTE_RESET, dest, &reset, sizeof(reset), timeout_ms))
    {
        printf("Reset sent to remote-%d... ACK received.\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
        
    }
    else
    {
        printf("Reset sent to remote-%d... ACK not received.\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
    }
} // Msg_Remote_Reset()


void Msg_Remote_Arm(uint8_t dest, uint32_t timeout_ms)
{
    FB_MSG_REMOTE_ARM_t arm = {0}; 
    
    arm.arm_key = FB_REMOTE_ARMED_MAGIC_NUMBER;
    
    if (Msg_Send(FB_MSG_REMOTE_ARM, dest, &arm, sizeof(arm), timeout_ms))
    {
        printf("Arm sent to remote-%d... ACK received.\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
        
    }
    else
    {
        printf("Arm sent to remote-%d... ACK not received.\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
    }
} // Msg_Remote_Arm()


void Msg_Keep_Alive(uint8_t dest)
{
    FB_MSG_KEEP_ALIVE_t keep_alive = {0};
    
    keep_alive.status_word |= System_Is_Armed()        ? (STATUS_ARMED_KEY | STATUS_ARMED_KEEP_ALIVE) : 0;
    keep_alive.status_word |= System_Show_Is_Running() ? STATUS_SHOW_IS_RUNNING                       : 0;
    
    keep_alive.show_time_ms = System_Get_Show_Time_ms();
    
    Msg_Send(FB_MSG_KEEP_ALIVE, dest, &keep_alive, sizeof(keep_alive), 0);
    
} // Msg_Keep_Alive()


void Msg_Remote_Disarm(uint8_t dest, uint32_t timeout_ms)
{
    FB_MSG_REMOTE_DISARM_t arm = {0}; 
    
    if (Msg_Send(FB_MSG_REMOTE_DISARM, dest, &arm, sizeof(arm), timeout_ms))
    {
        printf("Disarm sent to remote-%d... ACK received.\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
        
    }
    else
    {
        printf("Disarm sent to remote-%d... ACK not received.\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
    }
} // Msg_Remote_Disarm()



void Msg_Remote_Program_Commit(uint8_t dest)
{
    bool success;
        
    FB_MSG_REMOTE_PROGRAM_COMMIT_t program_commit = {0};
    
    printf("Committing program to remote-%d: ", Remote_NodeID_to_Integer(dest));

    success = Msg_Send(FB_MSG_REMOTE_PROGRAM_COMMIT, dest, &program_commit, sizeof(program_commit), 500);
    
    printf("%s\r\n" CON_CLEAR_LINE CON_RESET, success ? CON_GREEN "success" : CON_RED "fail");

}


void Msg_Remote_Program(uint8_t dest, uint32_t * firing_times_ms)
{
    uint16_t pin;

    printf("Programming remote-%d\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
    
    for (pin = 0; pin < PINS_NUM_OF; pin++)
    {
        bool program_success;
        
        FB_MSG_REMOTE_PROGRAM_t program = {0};
        
        program.pin = pin;
        program.firing_time_ms = firing_times_ms[pin] + 10000; // show will always start 10s late in order to ensure sync time
        
        program_success = Msg_Send(FB_MSG_REMOTE_PROGRAM, dest, &program, sizeof(program), 150);
        
        printf("Pin %3d - %6ldms: %s\r\n" CON_CLEAR_LINE CON_RESET, pin, firing_times_ms[pin], program_success ? CON_GREEN "success" : CON_RED "fail");
        
    }
} // Msg_Remote_Program()


void Msg_Remote_Scan_Request(uint8_t dest, uint32_t timeout_ms)
{
    // the message
    FB_MSG_REMOTE_SCAN_REQUEST_t scan_request = {0};

    printf("Remote-%d scan response:\r\n" CON_CLEAR_LINE, Remote_NodeID_to_Integer(dest));
    
    // send the message
    Msg_Send(FB_MSG_REMOTE_SCAN_REQUEST, dest, &scan_request, sizeof(scan_request), 0);
    
    // wait for a while to get a response
    if (Msg_Run(timeout_ms))
    {
        FB_MSG_BASE_t * p_base = (FB_MSG_BASE_t*)Msg_Get_Payload_Ptr();

        // if the message is for me, and it's from the right destination...
        if (Msg_Is_For_Me() && (Msg_Get_Sender_ID() == dest) && (p_base->id == FB_MSG_REMOTE_SCAN_RESPONSE))
        {
            FB_MSG_REMOTE_SCAN_RESPONSE_t * p_scan_response = (FB_MSG_REMOTE_SCAN_RESPONSE_t*)p_base;
            
            REMOTE_CUES_t * p_cues = &p_scan_response->cues;
            
            do
            {
                uint16_t socket;
                
                for (socket = 0; socket < 8; socket++)
                {
                    uint16_t cue;

                    printf("Socket %d: ", socket);
                    
                    for (cue = 0; cue < 16; cue++)
                    {
                        if (p_cues->sockets[socket] & 0x00001)
                            printf("Q%02d ", cue);
                        else
                            printf("    ");
                        
                        p_cues->sockets[socket] >>= 1;
                        
                    }
                    
                    printf("\r\n" CON_CLEAR_LINE);
                    
                }
            } while (0);

        }
        else
        {
            Msg_Unexpected_Message();
            
        }
    } 
    else
    {
        // we didn't receive a message back
        printf(CON_RED "Silence.\r\n" CON_CLEAR_LINE CON_CLEAR_LINE CON_RESET);
    }
} // Msg_Remote_Status_Request()


void Msg_Unexpected_Message(void)
{
    FB_MSG_BASE_t * p_base = (FB_MSG_BASE_t*)Msg_Get_Payload_Ptr();
    
    printf(CON_RED "Received unexpected message ID %d from %f.\r\n" CON_CLEAR_LINE CON_CLEAR_LINE CON_RESET, p_base->id, Remote_NodeID_to_Integer(Msg_Get_Sender_ID()));
    
} // Msg_Unexpected_Message()
