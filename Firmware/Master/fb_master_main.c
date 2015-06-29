#include <stdio.h>

#include "fb_common.h"
#include "fb_messages.h"
#include "rfm69.h"

typedef enum
{
    KEYSWITCH_STATE_OFF,
    KEYSWITCH_STATE_ARMED,
    KEYSWITCH_STATE_TEST,
    KEYSWITCH_STATE_FAULT,
    
} KEYSWITCH_STATE_t;

typedef enum
{
    FB_STATE_DISARMED,
    FB_STATE_KEY_TURNED,
    FB_STATE_CONFIRMING_KEY_TURN,
    FB_STATE_SENDING_ARM_CMD,
    FB_STATE_WAITING_FOR_ARM_RESP,
    FB_STATE_SENDING_ARM_CONF,
    FB_STATE_WAITING_FOR_ARMED_CONF,
    FB_STATE_ARMED
    
} FB_STATES_MASTER_ARMING_t;


KEYSWITCH_STATE_t Get_Keyswitch_State(void);

static FB_STATES_MASTER_ARMING_t system_state = FB_STATE_DISARMED;

static uint8_t mode = 0;
static bool mode_changed = false;

static REMOTE_CUES_t scan_results = {0};

void main(void)
{
    bool pong_received = false;
    uint32_t ping_timer = 0;
    uint32_t ping_sent = 0;
    uint16_t ping_response_time_ms = 0;
    int16_t ping_rx_rssi;
    int16_t ping_tx_rssi;
    
    // will store the keyswitch state
    KEYSWITCH_STATE_t keyswitch_state_prev = Get_Keyswitch_State();
    
    // initialize the C8051F920
    Init_Device();
    // clear the CTS signal so the FTDI will transmit
    PIN_UART_RTS_O = 0;
    
    printf("Start.\r\n");

    Msg_Init();

    printf("Super loop started.\r\n");
    
    ping_timer = millis() + 1000;
    
    while (1)
    {
        // get the current 
        const KEYSWITCH_STATE_t keyswitch_state_this = Get_Keyswitch_State();
        
        Msg_Run();

        if (keyswitch_state_this != keyswitch_state_prev)
        {
            switch (keyswitch_state_this)
            {
                case KEYSWITCH_STATE_OFF:
                    printf("Keyswitch: Off\r\n");
                    break;
                
                case KEYSWITCH_STATE_TEST:
                    printf("Keyswitch: Test\r\n");
                    break;
                
                case KEYSWITCH_STATE_ARMED:
                    printf("Keyswitch: Armed\r\n");
                    break;
                
                case KEYSWITCH_STATE_FAULT:
                default:
                    printf("Keyswitch: Fault\r\n");
                    break;
                
            } // switch (mode change)
            
            // store it for next time
            keyswitch_state_prev = keyswitch_state_this;
            
        }
        else
        {
            // do nothing
        }
        
        if (Msg_Received())
        {
            const uint8_t payload_size = Msg_Get_Payload_Size();
            const FB_MSG_BASE_t * payload = Msg_Get_Payload_Ptr();
            
            switch (payload->id)
            {
                case FB_MSG_PING:
                    {
                        static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                        static FB_MSG_PONG_t               msg_pong = {FB_MSG_PONG};
                        
                         Msg_Enqueue_for_Xmit(Msg_Get_Sender(), &msg_pong, sizeof(msg_pong), &msg_descriptor);
                       
                    }
                    break;
                case FB_MSG_PONG:
                    {
                        pong_received = true;
                        ping_response_time_ms = millis() - ping_sent;
                        ping_rx_rssi = Msg_Get_RSSI();
                        ping_tx_rssi = payload->rssi;
                    }
                    break;
                case FB_MSG_CUE_FIRED:
                    {
                        const FB_MSG_CUE_FIRED_t * msg_fired = (FB_MSG_CUE_FIRED_t*)payload;
                        
                        printf("Cue fired.\r\n");
                        printf("RSSI %ddBm\r\n", msg_fired->base.rssi);
                        printf("Socket %d, cue %d fired.\r\n", (uint16_t)msg_fired->socket, (uint16_t)msg_fired->cue);
                        
                    }
                        
                    break;
                case FB_MSG_RESP_SCAN_ALL_CUES:
                    {
                        const FB_MSG_RESP_SCAN_ALL_CUES_t * p_resp_scan = (FB_MSG_RESP_SCAN_ALL_CUES_t*)payload;
                        uint16_t socket = 0;
                        uint16_t cue = 0;
                        
                        scan_results = p_resp_scan->cues_present;
                        
                        printf("Scan response:\r\n");
                        printf("RSSI: %d\r\n", p_resp_scan->base.rssi);
                        for (socket = 0; socket < SOCKETS_NUM_OF; socket++)
                        {
                            printf("Socket %d: ", socket);
                            for (cue = 0; cue < CUES_NUM_OF; cue++)
                            {
                                if (BIT_IS_SET(p_resp_scan->cues_present.sockets[socket], cue))
                                {
                                    printf("Q%d ", cue);
                                }
                                else
                                {
                                    // do nothing
                                }
                            }
                            printf("\r\n");
                        }
                    }
                    break;
                default:
                    break;
            }
        }   
        
        if (_kbhit())
        {
            uint8_t old_mode = mode;
            mode = _getkey();
            
            if (old_mode != mode)
            {
                mode_changed = true;
            }
        }
        else
        {
            mode_changed = false;
        }

        if ('p' == mode)
        {
            if (ping_timer < millis())
            {
                if (pong_received)
                {
                    printf("Pong time: %3dms, RxRSSI: %3ddBm, TxRSSI: %3ddBm\r\n", ping_response_time_ms, ping_rx_rssi, ping_tx_rssi);
                }
                else
                {
                    printf("<silence>\r\n");
                }
                
                pong_received = false;

                printf("Ping... ");
                
                ping_sent = millis();
                {
                    static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                    static FB_MSG_PING_t               msg_ping = {0};
                    
                    msg_ping.id = FB_MSG_PING;
                    msg_ping.rssi = Msg_Get_RSSI();
                    Msg_Enqueue_for_Xmit(NODEID_REMOTE0, &msg_ping, sizeof(msg_ping), &msg_descriptor);
                   
                    ping_timer = millis() + 500;

                }
            }
        }
        else if ('q' == mode)
        {
            if (ping_timer < millis())
            {
                static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                static FB_MSG_CMD_SCAN_ALL_CUES_t  msg_scan_req = {0};

                printf("Scan request sent.\r\n");
                
                msg_scan_req.id = FB_MSG_CMD_SCAN_ALL_CUES;
                msg_scan_req.rssi = Msg_Get_RSSI();
                Msg_Enqueue_for_Xmit(NODEID_REMOTE0, &msg_scan_req, sizeof(msg_scan_req), &msg_descriptor);

                ping_timer = millis() + 10000;
                
            }
        }
        else if ('f' == mode)
        {
            static uint16_t cue_index = 0;

            if (mode_changed)
            {
                cue_index = 0;
            }
            
            if (ping_timer < millis())
            {
                static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                static FB_MSG_CMD_FIRE_CUE_t       msg_cmd_fire = {0};
                
                static uint16_t local_socket = 0;
                static uint16_t local_cue = 0;
                
                while (cue_index < 128)
                {
                    local_socket = cue_index >> 4;
                    local_cue = cue_index & 0x0F;

                    cue_index++;
                    
                    if (BIT_IS_SET(scan_results.sockets[local_socket], local_cue))
                    {
                        break;
                    }
                    else
                    {
                        // do nothing
                    }
                }
               
                if (cue_index <= 128)
                {
                    printf("Fire command sent for socket %d, cue %d.\r\n", local_socket, local_cue);
                    
                    msg_cmd_fire.base.id = FB_MSG_CMD_FIRE_CUE;
                    msg_cmd_fire.base.rssi = Msg_Get_RSSI();
                    msg_cmd_fire.socket = local_socket;
                    msg_cmd_fire.cue = local_cue;
                    Msg_Enqueue_for_Xmit(NODEID_REMOTE0, &msg_cmd_fire, sizeof(msg_cmd_fire), &msg_descriptor);

                    ping_timer = millis() + 2000;
                    
                    if (128 == cue_index)
                    {
                        printf("Done firing\r\n");
                        cue_index++;
                    }
                }                    
            }
        }
        else
        {
            // do nothing
        }
    }
} // main()


// read the key-switch inputs and interpret
KEYSWITCH_STATE_t Get_Keyswitch_State(void)
{
    // get the state of the pins
    const bool pin_armed = PIN_SWITCH_ARMED_I;
    const bool pin_test = PIN_SWITCH_TEST_I;
    
    // assume a faulted state
    KEYSWITCH_STATE_t state = KEYSWITCH_STATE_FAULT;
    
    if (!pin_armed && !pin_test)
    {
        // both pins low
        state = KEYSWITCH_STATE_OFF;
        
    }
    else if (pin_armed && !pin_test)
    {
        // armed high, test low
        state = KEYSWITCH_STATE_ARMED;
        
    }
    else if (!pin_armed && pin_test)
    {
        // armed low, test high
        state = KEYSWITCH_STATE_TEST;
        
    }
    else
    {
        // both pins high
        // do nothing - already assumed faulted state
    } // if (switch pins)
    
    // return the state
    return state;
    
} // Get_Keyswitch_State()


void Task_Arming_Master(void)
{
    switch (system_state)
    {
        case FB_STATE_DISARMED:
            break;
        
        case FB_STATE_KEY_TURNED:
            break;
        
        case FB_STATE_CONFIRMING_KEY_TURN:
            break;
        
        case FB_STATE_SENDING_ARM_CMD:
            break;
        
        case FB_STATE_WAITING_FOR_ARM_RESP:
            break;
        
        case FB_STATE_SENDING_ARM_CONF:
            break;
        
        case FB_STATE_WAITING_FOR_ARMED_CONF:
            break;
        
        case FB_STATE_ARMED:
            break;
        
        default:
            system_state = FB_STATE_DISARMED;
            break;
    }
} // Task_Arming_Master()
