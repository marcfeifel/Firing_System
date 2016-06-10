#include <string.h>

#include "fb_messages.h"
#include "fb_common.h"
#include "rfm69.h"
#include "fb_tasks.h"

// how long to assert power when firing a cue
#define FB_FIRE_HOLD_TIME_MS   5

// minimum pulse-width is 5ns
#define FB_74HC393_DELAY()     do { NOP(); NOP(); NOP(); NOP(); } while (0)
#define FB_74HC393_PULSE(_pin) do { FB_74HC393_DELAY(); (_pin) = 1; FB_74HC393_DELAY(); (_pin) = 0; } while (0)

typedef enum
{
    FB_STATE_DISARMED,
    FB_STATE_RECEIVED_ARM_CMD,
    FB_STATE_SENDING_ARM_RESP,
    FB_STATE_WAITING_FOR_ARM_CONF,
    FB_STATE_SENDING_ARMED_CONF,
    FB_STATE_ARMED

} FB_REMOTE_STATES_ARMING_t;

static struct
{
    SOCKET_ENUM_t socket;
    CUE_ENUM_t    cue;
    uint32_t      delay_ms;

} program[SOCKETS_NUM_OF * CUES_NUM_OF] =
{
    { SOCKET0, CUE0,     0 },
    { SOCKET1, CUE0,  5000 },
    { SOCKET0, CUE1,  4000 },
    { SOCKET0, CUE2,   100 },
    { SOCKET1, CUE1,  4000 },
    { SOCKET1, CUE2,   100 },
    { SOCKET0, CUE3,  3000 },
    { SOCKET0, CUE4,   100 },
    { SOCKET0, CUE5,   100 },
    { SOCKET1, CUE3,  3000 },
    { SOCKET1, CUE4,   100 },
    { SOCKET1, CUE5,   100 },
    { SOCKET0, CUE6,  3000 },
    { SOCKET1, CUE6,    50 },
    { SOCKET0, CUE7,    50 },
    { SOCKET1, CUE7,    50 },
    { SOCKET0, CUE8,    50 },
    { SOCKET1, CUE8,    50 },
    { SOCKET0, CUE9,    50 },
    { SOCKET1, CUE9,    50 },
    { SOCKET0, CUE10,   50 },
    { SOCKET1, CUE10,   50 },
    { SOCKET0, CUE11,   50 },
    { SOCKET1, CUE11,   50 },
    { SOCKET0, CUE12,   50 },
    { SOCKET1, CUE12,   50 },
    { SOCKET0, CUE13,   50 },
    { SOCKET1, CUE13,   50 },
    { SOCKET0, CUE14,   50 },
    { SOCKET1, CUE14,   50 },
    { SOCKET0, CUE15,   50 },
    { SOCKET1, CUE15,   50 },

    { SOCKETS_NUM_OF, CUES_NUM_OF, -1 }
};

void FireTest_Clear(void);
void FireTest_Assert_Fire(void);
void FireTest_Assert_Test(void);

void Cue_Select(SOCKET_ENUM_t socket, CUE_ENUM_t cue);
bool Cue_Is_Present(void);
void Cue_Fire(void);
void Cue_Scan_All(REMOTE_CUES_t * p_remote);

static REMOTE_CUES_t cues_present = { 0 };
static FB_REMOTE_STATES_ARMING_t system_state = FB_STATE_DISARMED;

void main(void)
{
    FB_MSG_XMIT_DESCRIPTOR msg = {0};

    // initialize the C8051F920
    Init_Device();

    // clear the fire and test signals
    FireTest_Clear();

    // initialize the message module
    Msg_Init();

    // super loop
    while (true)
    {
#if 0 // HW debug
        static uint8_t cue = 0;

        Cue_Select(0, (cue++)&0x0F);
        FireTest_Assert_Test();
        FireTest_Assert_Fire();
#endif // 0/1
        // process any incoming and outgoing messages
        Msg_Run();
/*
#if 1
        Task_Scan_Cues();
        Task_Ping();
        Task_Pong();
        Task_Arming();
        Task_Scan_Cues();
        Task_Run_Program();
#else
        Task_Echo();
#endif // 0*/
        if (Msg_Received())
        {
            const FB_MSG_BASE_t * payload = Msg_Get_Payload_Ptr();

            if (RF69_BROADCAST_ADDR == RFM69_getTargetID())
            {
                Cue_Select(0, 0);
                FireTest_Assert_Test();
                FireTest_Assert_Fire();
            }
            
            switch (payload->id)
            {
                case FB_MSG_PING:
                    {
                        static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                        static FB_MSG_PONG_t               msg_pong = {0};

                        msg_pong.id = FB_MSG_PONG;
                        msg_pong.rssi = Msg_Get_RSSI();
                        Msg_Enqueue_for_Xmit(Msg_Get_Sender(), &msg_pong, sizeof(msg_pong), &msg_descriptor);

                    }
                    break;
                case FB_MSG_PONG:
                    {
                        // do something about receiving a pong
                    }
                    break;
                case FB_MSG_ACK:
                    break;
                case FB_MSG_NACK:
                    break;
                case FB_MSG_CMD_SCAN_ALL_CUES:
                    {
                        static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                        static FB_MSG_RESP_SCAN_ALL_CUES_t msg_report = {0};

                        Cue_Scan_All(&msg_report.cues_present);

                        msg_report.base.id = FB_MSG_RESP_SCAN_ALL_CUES;
                        msg_report.base.rssi = Msg_Get_RSSI();
                        Msg_Enqueue_for_Xmit(Msg_Get_Sender(), &msg_report, sizeof(msg_report), &msg_descriptor);

                    }
                    break;
                case FB_MSG_CMD_FIRE_CUE:
/*                    {
                        const FB_MSG_CMD_FIRE_CUE_t * fire_cue_cmd = (FB_MSG_CMD_FIRE_CUE_t*)payload;

                        FireTest_Clear();
                        Cue_Select(fire_cue_cmd->socket, fire_cue_cmd->cue);
                        FireTest_Assert_Test();
                        FireTest_Assert_Fire();
                        Sleep(5);
                        FireTest_Clear();

                        {
                            static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                            static FB_MSG_CUE_FIRED_t          msg_cue_fired = {0};

                            msg_cue_fired.base.id   = FB_MSG_CUE_FIRED;
                            msg_cue_fired.base.rssi = Msg_Get_RSSI();
                            msg_cue_fired.socket = fire_cue_cmd->socket;
                            msg_cue_fired.cue    = fire_cue_cmd->cue;
                            Msg_Enqueue_for_Xmit(Msg_Get_Sender(), &msg_cue_fired, sizeof(msg_cue_fired), &msg_descriptor);

                        }

                    }
                    break;*/
                case FB_MSG_CMD_FIRE_PROGRAM:
                    {
                        uint32_t next_event_time_ms = millis();
                        uint32_t next_step = 0;

                        while ((program[next_step].socket != SOCKETS_NUM_OF) && (program[next_step].cue != CUES_NUM_OF))
                        {
                            next_event_time_ms += program[next_step].delay_ms;

                            while (millis() < next_event_time_ms)
                            {
                                // loop
                            }

                            FireTest_Clear();
                            Cue_Select(program[next_step].socket, program[next_step].cue);
                            FireTest_Assert_Test();
                            FireTest_Assert_Fire();
                            Sleep(5);

                            {
                                static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                                static FB_MSG_CUE_FIRED_t          msg_cue_fired = {0};

                                msg_cue_fired.base.id   = FB_MSG_CUE_FIRED;
                                msg_cue_fired.base.rssi = Msg_Get_RSSI();
                                msg_cue_fired.socket =    program[next_step].socket;
                                msg_cue_fired.cue    =    program[next_step].cue;
                                Msg_Enqueue_for_Xmit(Msg_Get_Sender(), &msg_cue_fired, sizeof(msg_cue_fired), &msg_descriptor);

                            }

                            next_step++;

                        }

                        FireTest_Clear();

                    }
                    break;
                default:
                    break;
            }
        }
    } // while (true)
} // main()


void FireTest_Clear(void)
{
    // assert the reset/clear signal
    PIN_74HC393_CLEAR_O = 1;
    FB_74HC393_DELAY();
    
    // set both clock pins high to put them into their idle state
    PIN_74HC393_TEST_CLK_O = 0;
    PIN_74HC393_FIRE_CLK_O = 0;
    FB_74HC393_DELAY();

    // de-assert the reset/clear signal
    PIN_74HC393_CLEAR_O = 0;
    FB_74HC393_DELAY();

} // FireTest_Clear()


void FireTest_Assert_Fire(void)
{
//    if (FB_STATE_ARMED == system_state)
    if (1)
    {
        // pulse it 8 times
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 1
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 2
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 3
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 4
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 5
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 6
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 7
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 8
        
        // wait for the match to ignite
        Sleep(FB_FIRE_HOLD_TIME_MS);

        // clear it
        FireTest_Clear();
        
    }
    else
    {
        while(1)
        {
            // this should never happen - need to decide what this means
        }
    }
} // FireTest_Assert_Fire()


void FireTest_Assert_Test(void)
{
    // pulse it 8 times
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 1
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 2
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 3
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 4
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 5
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 6
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 7
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 8

} // FireTest_Assert_Test()


void Cue_Select(SOCKET_ENUM_t socket, CUE_ENUM_t cue)
{
    // make sure we don't immediately fire
    FireTest_Clear();

    // setup the cue address
    PIN_ADDRESS_CUEb0_O = cue & BIT0 ? 1 : 0;
    PIN_ADDRESS_CUEb1_O = cue & BIT1 ? 1 : 0;
    PIN_ADDRESS_CUEb2_O = cue & BIT2 ? 1 : 0;
    PIN_ADDRESS_CUEb3_O = cue & BIT3 ? 1 : 0;

    // setup the socket address
    PIN_ADDRESS_SOCKb0_O = socket & BIT0 ? 1 : 0;
    PIN_ADDRESS_SOCKb1_O = socket & BIT1 ? 1 : 0;
    PIN_ADDRESS_SOCKb2_O = socket & BIT2 ? 1 : 0;
    
} // Cue_Select()


bool Cue_Is_Present(void)
{
    // will store whether the cue is present or not
    bool cue_is_present = false;

    // assert the test signal
    FireTest_Assert_Test();
    Sleep(2);

    // see if the cue is not empty
    cue_is_present = !PIN_CUE_EMPTY_I;

    // de-assert the test signal
    FireTest_Clear();

    // return the result
    return cue_is_present;

} // Cur_Is_Present()


void Cue_Scan_All(REMOTE_CUES_t * p_remote)
{
    // will iterate through the sockets
    uint8_t socket = 0;

    // assume all cues empty by clearing all memory
    memset(p_remote, 0, sizeof(REMOTE_CUES_t));

    // iterate through the sockets
    for (socket = 0; socket < SOCKETS_NUM_OF; socket++)
    {
        // will iterate through the cues
        uint8_t cue;

        // iterate through the cues
        for (cue = 0; cue < CUES_NUM_OF; cue++)
        {
            // select the cue
            Cue_Select(socket, cue);

            // check if it is present or not
            if (Cue_Is_Present())
            {
                // it is present - set the appropriate bit
                BIT_SET(p_remote->sockets[socket], cue);
            }
            else
            {
                // it is not present - already cleared memory
            }
        } // for (cue)
    } // for (socket)
} // Cue_Scan_All()


void Task_Arming_Remote(void)
{
    switch (system_state)
    {
        case FB_STATE_DISARMED:
            break;

        case FB_STATE_RECEIVED_ARM_CMD:
            break;

        case FB_STATE_SENDING_ARM_RESP:
            break;

        case FB_STATE_WAITING_FOR_ARM_CONF:
            break;

        case FB_STATE_SENDING_ARMED_CONF:
            break;

        case FB_STATE_ARMED:
            break;

        default:
            system_state = FB_STATE_DISARMED;
            break;
    }
} // Task_Arming_Remote()
