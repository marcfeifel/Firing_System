#ifndef FB_REMOTE_PROGRAM_RUN_H_INCLUDED
#define FB_REMOTE_PROGRAM_RUN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void fb_Remote_Program_Run_Init(uint32_t const * p_pin_firing_times_ms);
void fb_Remote_Program_Run_Set_Run(bool running);
void fb_Remote_Program_Run_Update_Show_Time(uint32_t global_show_time_ms);
bool fb_Remote_Program_Run_Is_Running(void);
uint32_t fb_Remote_Program_Run_Get_Show_Time_ms(void);
void fb_Remote_Program_Run_To_Next_Cue_Info(uint32_t * p_time_ms, uint8_t * p_pin, uint8_t * p_cues_remaining);

void fb_Remote_Program_Run_Handler_ms(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FB_REMOTE_PROGRAM_RUN_H_INCLUDED
