#ifndef FB_REMOTE_CUE_H_INCLUDED
#define FB_REMOTE_CUE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// initialize the subsystem
void fb_Remote_Cue_Init(void);

// clear both the 'fire' and 'test' signals
void fb_Remote_Cue_Clear_FireTest(void);

// select a particular cue
void fb_Remote_Cue_Select(SOCKET_ENUM_t socket, CUE_ENUM_t cue);

// attempt to assert the 'fire' signal
bool fb_Remote_Cue_Assert_Fire(void);
bool fb_Remote_Cue_Fire_is_Asserted(void);

// attempt to assert the 'test' signal
void fb_Remote_Cue_Assert_Test(void);
bool fb_Remote_Cue_Test_is_Asserted(void);

// check if a cue is present
bool fb_Remote_Cue_Is_Present(void);

// scan all of the cues to see if they are present
void fb_Remote_Cue_Scan_All(REMOTE_CUES_t * p_remote);

// return the number of milliseconds the 'fire' signal needs to be asserted
uint32_t fb_Remote_Cue_Fire_Time_ms(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FB_REMOTE_CUE_H_INCLUDED