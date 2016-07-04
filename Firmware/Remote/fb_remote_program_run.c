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
    m_state.show_time_ms = (m_state.show_time_ms + global_show_time_ms) >> 1;
    
} // fb_Remote_Program_Run_Update_Show_Time()


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
    } // if (running)
    
    // increment the show's run time
    m_state.show_time_ms++;

} // fb_Remote_Program_Run_Handler_ms()
