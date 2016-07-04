#include <stdio.h>

#include "fb_common.h"
#include "fb_messages.h"
#include "rfm69.h"

#define NODEID_TARGET NODEID_REMOTE1

typedef enum
{
    KEYSWITCH_STATE_OFF,
    KEYSWITCH_STATE_ARMED,
    KEYSWITCH_STATE_TEST,
    KEYSWITCH_STATE_FAULT,

} KEYSWITCH_STATE_t;

KEYSWITCH_STATE_t Get_Keyswitch_State(void);

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

    do
    {
        FB_MSG_SYSTEM_RESET_t       msg_reset = {0};
        uint16_t i;
        
        // reset all remotes
        for (i = 0; i < 10; i++)
        {

            Msg_Enqueue_for_Xmit(FB_MSG_SYSTEM_RESET, RF69_BROADCAST_ADDR, &msg_reset, sizeof(msg_reset));
            
            printf("Sending reset command %d...\r\n", i);
            
            while (!Msg_Xmit_Is_Complete())
            {
                Msg_Run();
            }
            
            Sleep(100);
            
        }    
    } while (0);

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
                        FB_MSG_PONG_t               msg_pong = {0};

                        Msg_Enqueue_for_Xmit(FB_MSG_PONG, Msg_Get_Sender(), &msg_pong, sizeof(msg_pong));

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

                        printf("Scan response from remote-%d:\r\n", (uint16_t)Msg_Get_Sender() - NODEID_REMOTE1 + 1);
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
                static uint16_t ping_target = NODEID_REMOTE1;
                
                if (pong_received)
                {
                    printf("Pong - Time: %3dms, RxRSSI: %3ddBm, TxRSSI: %3ddBm\r\n", ping_response_time_ms, ping_rx_rssi, ping_tx_rssi);
                }
                else
                {
                    printf("<silence>\r\n");
                }

                pong_received = false;
                
                if (NODEID_REMOTE0 == ping_target)
                    ping_target = NODEID_REMOTE1;
                else if (NODEID_REMOTE1 == ping_target)
                    ping_target = NODEID_REMOTE2;
                else
                    ping_target = NODEID_REMOTE0;

                printf("Ping remote-%d... ", ping_target - NODEID_REMOTE_BASE);

                ping_sent = millis();
                {
                    FB_MSG_PING_t               msg_ping = {0};

                    Msg_Enqueue_for_Xmit(FB_MSG_PING, ping_target, &msg_ping, sizeof(msg_ping));

                    ping_timer = millis() + 500;

                }
            }
        }
        else if ('q' == mode)
        {
            static uint16_t ping_target = NODEID_MASTER;

            if (ping_timer < millis())
            {
                if (NODEID_MASTER == ping_target)
                    ping_target = NODEID_REMOTE0;
                else if (NODEID_REMOTE0 == ping_target)
                    ping_target = NODEID_REMOTE1;
                else if (NODEID_REMOTE1 == ping_target)
                    ping_target = NODEID_REMOTE2;
                else
                {
                    ping_target = NODEID_MASTER;
                    mode = ' ';
                }

                if (NODEID_MASTER != ping_target)
                {
                    FB_MSG_CMD_SCAN_ALL_CUES_t  msg_scan_req = {0};

                    printf("Scan request sent to remote-%d.\r\n", ping_target - NODEID_REMOTE_BASE);

                    Msg_Enqueue_for_Xmit(FB_MSG_CMD_SCAN_ALL_CUES, ping_target, &msg_scan_req, sizeof(msg_scan_req));

                    ping_timer = millis() + 2000;
                }
            }
        }
        else if ('f' == mode)
        {

            if (ping_timer < millis())
            {
                FB_MSG_CMD_FIRE_CUE_t       msg_cmd_fire = {0};

                printf("Fire-all request sent.\r\n");

                Msg_Enqueue_for_Xmit(FB_MSG_CMD_FIRE_ALL, RF69_BROADCAST_ADDR, &msg_cmd_fire, sizeof(msg_cmd_fire));

                ping_timer = millis() + 2000;

                mode = ' ';
                
            }
        }
        else if ('r' == mode)
        {
            Reset_MCU();
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


void Task_1ms_Low_Priority_Handler(void)
{
} // Task_1ms_Low_Priority_Handler()


void Task_1ms_High_Priority_Handler(void)
{
} // Task_1ms_High_Priority_Handler()
