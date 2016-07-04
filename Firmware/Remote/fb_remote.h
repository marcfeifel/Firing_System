#ifndef FB_REMOTE_H_INCLUDED
#define FB_REMOTE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

bool fb_Remote_Is_Armed(void);
void fb_Remote_Set_Armed_Flag(bool armed_flag);
void fb_Remote_Set_Arming_Key(uint32_t arming_key);
bool fb_Remote_Arming_Key_Is_Set(void);
bool fb_Remote_Arming_Flag_Is_Set(void);
void fb_Remote_Disarm(void);

void fb_Keep_Alive_Received(void);
    
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FB_REMOTE_H_INCLUDED
