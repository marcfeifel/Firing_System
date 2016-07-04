#include <string.h>

#include "fb_messages.h"
#include "fb_common.h"
#include "rfm69.h"

#include "fb_remote.h"
#include "fb_remote_cue.h"
#include "fb_remote_program_run.h"

static uint32_t m_firing_times_ms[128] = { 0 };

static bool m_armed_flag = false;
static uint32_t m_arming_key = 0;

static uint32_t m_keep_alive_received_ms;

void fb_Remote_Program_Load(uint32_t * p_firing_times_ms);

void main(void)
{
    // initialize the C8051F920
    Init_Device();

    fb_Remote_Program_Load(m_firing_times_ms);
    
    // clear the fire and test signals
    fb_Remote_Cue_Init();
    fb_Remote_Program_Run_Init(m_firing_times_ms);
    
    // initialize the message module
    Msg_Init();

    // super loop
    while (true)
    {
        // process any incoming and outgoing messages
        Msg_Run(0);

        Msg_Disbatch();
        
        if (millis() > (m_keep_alive_received_ms + 10000))
        {
            Msg_Init();
            
            m_keep_alive_received_ms = millis();
        }

    } // while (true)
} // main()


bool fb_Remote_Is_Armed(void)
{
    return fb_Remote_Arming_Key_Is_Set() && fb_Remote_Arming_Flag_Is_Set();
    
} // fb_Remote_Is_Armed()


void fb_Remote_Set_Armed_Flag(bool armed_flag)
{
    m_armed_flag = armed_flag;
    
} // fb_Remote_Set_Armed_Flag()


bool fb_Remote_Arming_Key_Is_Set(void)
{
    return FB_REMOTE_ARMED_MAGIC_NUMBER == m_arming_key;
    
} // fb_Remote_Arming_Key_Is_Set()


bool fb_Remote_Arming_Flag_Is_Set(void)
{
    return m_armed_flag;
    
} // fb_Remote_Arming_Flag_Is_Set()


void fb_Remote_Set_Arming_Key(uint32_t arming_key)
{
    m_arming_key = arming_key;
    
} // fb_Remote_Set_Arming_Key()


void fb_Remote_Disarm(void)
{
    m_armed_flag = false;
    m_arming_key = 0;
    
} // fb_Remote_Disarm()


void fb_Keep_Alive_Received(void)
{
    m_keep_alive_received_ms = millis();
    
} // fb_Keep_Alive_Received()


void Task_1ms_High_Priority_Handler(void)
{
    fb_Remote_Program_Run_Handler_ms();
    
} // Task_1ms_High_Priority_Handler()


void Task_1ms_Low_Priority_Handler(void)
{
    // do nothing for now
} // Task_1ms_Low_Priority_Handler()
