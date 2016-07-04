#include <string.h>

#include "fb_messages.h"
#include "fb_common.h"
#include "rfm69.h"

#include "fb_remote_cue.h"
#include "fb_remote_program_run.h"

typedef enum
{
    FB_STATE_DISARMED,
    FB_STATE_RECEIVED_ARM_CMD,
    FB_STATE_SENDING_ARM_RESP,
    FB_STATE_WAITING_FOR_ARM_CONF,
    FB_STATE_SENDING_ARMED_CONF,
    FB_STATE_ARMED

} FB_REMOTE_STATES_ARMING_t;


static REMOTE_CUES_t cues_present = { 0 };
static FB_REMOTE_STATES_ARMING_t system_state = FB_STATE_DISARMED;

static uint32_t m_firing_times_ms[128] = { 0 };

void main(void)
{
    FB_MSG_XMIT_DESCRIPTOR msg = {0};

    // initialize the C8051F920
    Init_Device();
    
    {
        uint32_t i;
        for (i = 0; i < 128; i++)
             m_firing_times_ms[i] = i * 15000;
    }
    
    // clear the fire and test signals
    fb_Remote_Cue_Init();
    fb_Remote_Program_Run_Init(m_firing_times_ms);
    
    // initialize the message module
    Msg_Init();

    // super loop
    while (true)
    {
#if 0 // HW debug - 1s pulses
        static uint32_t time = 1000;

        if (millis_expired(time))
        {
            time += 1000;
            fb_Remote_Cue_Select(0, 0);
            fb_Remote_Cue_Assert_Test();
            fb_Remote_Cue_Assert_Fire();
            Sleep(fb_Remote_Cue_Fire_Time_ms());
            fb_Remote_Cue_Clear_FireTest();
        }
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

            // is it from the master?
            if (NODEID_MASTER == RFM69_getSenderID())
            {
                // take the time-stamp and apply a correction using it
                fb_Remote_Program_Run_Update_Show_Time(payload->time_ms);

            }
            else
            {
                // do nothing
            }

            if (NODEID_BROADCAST == RFM69_getTargetID())
            {
                switch (payload->id)
                {
                    case FB_MSG_SYSTEM_RESET:
                        Reset_MCU();
                        break;

                    case FB_MSG_CMD_FIRE_ALL:
                        {
                            fb_Remote_Program_Run_Init(m_firing_times_ms);
                            fb_Remote_Program_Run_Set_Run(true);
                        }
                        break;

                    default:
                        //do nothing
                        break;
                }
            }
            else if (NODEID_LOCAL == RFM69_getTargetID())
            {
                switch (payload->id)
                {
                    case FB_MSG_PING:
                        {
                            static FB_MSG_XMIT_DESCRIPTOR      msg_descriptor = {0};
                            static FB_MSG_PONG_t               msg_pong = {0};

                            Msg_Enqueue_for_Xmit(FB_MSG_PONG, Msg_Get_Sender(), &msg_pong, sizeof(msg_pong), &msg_descriptor);

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

                            fb_Remote_Cue_Scan_All(&msg_report.cues_present);

                            Msg_Enqueue_for_Xmit(FB_MSG_RESP_SCAN_ALL_CUES, Msg_Get_Sender(), &msg_report, sizeof(msg_report), &msg_descriptor);

                        }
                        break;

                    case FB_MSG_CMD_FIRE_PROGRAM:
                        break;

                    default:
                        break;
                }
            }
            else
            {
                // message isn't for me!
            }
        }
    } // while (true)
} // main()


bool fb_Remote_Is_Armed(void)
{
    return true;
    
} // fb_Remote_Is_Armed()


void Task_1ms_High_Priority_Handler(void)
{
    fb_Remote_Program_Run_Handler_ms();
    
} // Task_1ms_High_Priority_Handler()


void Task_1ms_Low_Priority_Handler(void)
{
    // do nothing for now
} // Task_1ms_Low_Priority_Handler()
