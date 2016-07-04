#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "fb_common.h"
#include "fb_remote.h"
#include "fb_remote_program_run.h"
#include "fb_remote_cue.h"

static struct
{
    // is a program running right now?
    uint8_t running;

    // how far into the show are we?
    // this is the value that should be synchronized across all remotes
    uint32_t show_time_ms;

    // have the pins been fired?
    uint8_t pin_fired[PINS_NUM_OF];

    // when this counter gets to 0, deassert the fire signal
    // this value is used instead of show_time_ms in case
    // show_times_ms gets a correction forward which would
    // make the firing time too short
    uint32_t fire_deassert_timer_ms;

    // pointer to the firing times for all the pins
    uint32_t const * p_pin_firing_times_ms;

} m_state = { 0 };


void fb_Remote_Program_Run_Init(uint32_t const * p_pin_firing_times_ms)
{
    // clear it all
    memset(&m_state, 0, sizeof(m_state));

    // set the pointer to the firing times
    m_state.p_pin_firing_times_ms = p_pin_firing_times_ms;

} // fb_Remote_Program_Run_Init()


void fb_Remote_Program_Run_Set_Run(bool running)
{
    m_state.running = running;

} // fb_Remote_Program_Run_Set_Run()


void fb_Remote_Program_Run_Update_Show_Time(uint32_t global_show_time_ms)
{
    if (m_state.running)
    {
        Task_1ms_High_Priority_Disable();
        
        m_state.show_time_ms = (m_state.show_time_ms + global_show_time_ms) >> 1;
        
        Task_1ms_High_Priority_Enable();
    }
} // fb_Remote_Program_Run_Update_Show_Time()


bool fb_Remote_Program_Run_Is_Running(void)
{
    return m_state.running;
    
} // fb_Remote_Program_Run_Is_Running()


uint32_t fb_Remote_Program_Run_Get_Show_Time_ms(void)
{
    uint32_t show_time_ms_latched;
    
    Task_1ms_High_Priority_Disable();
    
    show_time_ms_latched = m_state.show_time_ms;
    
    Task_1ms_High_Priority_Enable();

    return show_time_ms_latched;
    
} // fb_Remote_Program_Run_Get_Show_Time_ms()


void fb_Remote_Program_Run_To_Next_Cue_Info(uint32_t * p_time_ms, uint8_t * p_pin, uint8_t * p_cues_remaining)
{
    // start with the largest possible number
    uint32_t closest_ms = (uint32_t)-1;
    uint8_t  closest_pin = 0;
    uint8_t  cues_remaining = 0;
    
    // will iterate through the pins
    uint8_t pin;

    // get the current time
    uint32_t now = fb_Remote_Program_Run_Get_Show_Time_ms();
    
    for (pin = 0; pin < PINS_NUM_OF; pin++)
    {
        // if the pin hasn't been fired...
        if (!m_state.pin_fired[pin])
        {
            if (CUE_EMPTY != m_state.p_pin_firing_times_ms[pin])
            {
                // get the delta until it does get fired...
                uint32_t time_until_ms = m_state.p_pin_firing_times_ms[pin] - now;
        
                if (time_until_ms < closest_ms)
                {
                    // and keep the smaller of the two
                    closest_ms = time_until_ms;
                    
                    closest_pin = pin;
                    
                }            

                cues_remaining++;
                
            }
            else
            {
                // do nothing
            }
        }
        else
        {
            // do nothing
        }
    } // for (pin)
    
    *p_time_ms = closest_ms;
    *p_pin     = closest_pin;
    *p_cues_remaining = cues_remaining;
    
} // fb_Remote_Program_Run_Time_To_Next_Cue()


void fb_Remote_Program_Run_Handler_ms(void)
{
    if (m_state.running)
    {
        if (fb_Remote_Cue_Fire_is_Asserted())
        {
            // decrement the counter
            m_state.fire_deassert_timer_ms--;
            
            // if it's zero, then time to deassert it
            if (!m_state.fire_deassert_timer_ms)        
            {
                // 'fire' signal held long enough - release it
                fb_Remote_Cue_Clear_FireTest();
                
            }
            else
            {
                // do nothing
            } // if (deassert_time_ms)
        }
        else
        {
            if (fb_Remote_Is_Armed())
            {
                uint8_t pin = 0;

                bool cue_found = false;

                while (!cue_found && (PINS_NUM_OF > pin))
                {
                    if (!m_state.pin_fired[pin] && (m_state.show_time_ms >= m_state.p_pin_firing_times_ms[pin]))
                    {
                        // select the cue
                        fb_Remote_Cue_Select(GET_SOCKET(pin), GET_CUE(pin));

                        // assert the fire signal
                        fb_Remote_Cue_Assert_Test();
                        fb_Remote_Cue_Assert_Fire();

                        // set the time when the firing signal should be de-asserted
                        m_state.fire_deassert_timer_ms = fb_Remote_Cue_Fire_Time_ms();
                        
                        // indicate the cue has been fired
                        m_state.pin_fired[pin] = true;

                        // indicate a cue was found
                        cue_found = true;

                    }
                    else
                    {
                        // move to the next pin
                        pin++;

                    } // if (!fired && time_to_fire)
                } // while (!found && more_cues)
            }
            else
            {
                // do nothing
            } // if (running && armed)
        } // if (run_time > fire_deassert)

        // increment the show's run time
        m_state.show_time_ms++;

    } // if (running)
} // fb_Remote_Program_Run_Handler_ms()
