#include <stdio.h>
#include <string.h>

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

static uint32_t m_show_time_ms = 0;
static bool m_system_armed = false;
static bool m_show_is_running = false;

static KEYSWITCH_STATE_t Get_Keyswitch_State(void);
static void Update_Keyswitch_State(void);

static struct
{
    KEYSWITCH_STATE_t keyswitch_state_prev;
    
} m_state;

void System_Bump_Show_Time_ms(uint32_t time_ms);


bool Get_Confirmation(void)
{
    char key_in;
    
    while (!_kbhit)
    {
    }
    
    key_in = _getkey();
    
    printf("\r\n");
    
    if (('Y' == key_in) || ('y' == key_in))
    {
        return true;
    }
    else
    {
        return false;
    }    
} // Get_Confirmation()


void main(void)
{
    // will store the keyswitch state
    m_state.keyswitch_state_prev = Get_Keyswitch_State();

    // initialize the C8051F920
    Init_Device();
    // clear the CTS signal so the FTDI will transmit
    PIN_UART_RTS_O = 0;


    printf("Start.\r\n");

    Msg_Init();

    printf("Super loop started.\r\n");

    while (1)
    {
        Update_Keyswitch_State();
        
        if (_kbhit())
        {
            switch(_getkey())
            {
                case 's':
                case 'S':
                    // test all the cues
                    printf("Scan all cues?");
                    if (Get_Confirmation())
                    {
                        Msg_Remote_Scan_Request(NODEID_REMOTE0, 1000);
                        Msg_Remote_Scan_Request(NODEID_REMOTE1, 1000);
                        Msg_Remote_Scan_Request(NODEID_REMOTE2, 1000);
                    }
                    break;
                
                case 'd':
                case 'D':
                    // download a show
                    printf("Download a new show?");
                    if (Get_Confirmation())
                    {
                        uint32_t the_show[128] = {0};
                        memset(the_show, 0xFF, sizeof(the_show));
                        
                        Msg_Remote_Program(NODEID_REMOTE0, the_show);

                        {
                            uint32_t i;
                            memset(the_show, 0xFF, sizeof(the_show));
                            for (i = 0; i < 128; i++)
                                 the_show[i] = (uint32_t)((uint32_t)127 * (uint32_t)1500) - i * 1500 + 10000;
                        }
                        Msg_Remote_Program(NODEID_REMOTE1, the_show);
  
                        {
                            uint32_t i;
                            memset(the_show, 0xFF, sizeof(the_show));
                            for (i = 0; i < 128; i++)
                                 the_show[i] = i * 1500 + 10000;
                        }
                        Msg_Remote_Program(NODEID_REMOTE2, the_show);
                    }
                    break;
                
                case 'r':
                case 'R':
                    // start/continue a show
                    printf("Start/continue the show?");
                    if (Get_Confirmation())
                    {
                        m_show_is_running = true;
                        
                    }
                    break;
                
                case 'f':
                case 'F':
                    // bump show-time
                    printf("Fast-forward the show?");
                    if (Get_Confirmation())
                    {
                        System_Bump_Show_Time_ms(30000);
                    }
                    break;
                
                case 'p':
                case 'P':
                    // pause the show
                    printf("Pause the show?");
                    if (Get_Confirmation())
                    {
                        m_show_is_running = false;

                    }
                    break;
                
                case 'c':
                case 'C':
                    // commit program
                    printf("Commit program?");
                    if (Get_Confirmation())
                    {
                        Msg_Remote_Program_Commit(NODEID_REMOTE0);
                        Msg_Remote_Program_Commit(NODEID_REMOTE1);
                        Msg_Remote_Program_Commit(NODEID_REMOTE2);
                    }
                    break;
                
                case '0':
                    // global reset
                    printf("Reset system?");
                    if (Get_Confirmation())
                    {
                        Msg_Remote_Reset(NODEID_REMOTE0, 500);
                        Msg_Remote_Reset(NODEID_REMOTE1, 500);
                        Msg_Remote_Reset(NODEID_REMOTE2, 500);
                        Reset_MCU();
                    }
                    break;
                    
                default:
                    printf("\r\n");
                    printf("Options:\r\n");
                    printf("'s' - scan all cues\r\n");
                    printf("'d' - download a show\r\n");
                    printf("'r' - start/continue a show\r\n");
                    printf("'f' - advance 30 seconds into show\r\n");
                    printf("'p' - pause a show\r\n");
                    printf("'c' - commit a show to flash\r\n");
                    printf("'0' - global reset\r\n");
                    printf("\r\n");
                    break;
            }
        }
        else
        {
            Msg_Keep_Alive(NODEID_BROADCAST);

            Msg_Ping(NODEID_REMOTE0, 100);
            Msg_Ping(NODEID_REMOTE1, 100);
            Msg_Ping(NODEID_REMOTE2, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE0, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE1, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE2, 100);
            printf("\r\n");
            
            Sleep(500);
        }
    }
} // main()


// read the key-switch inputs and interpret
static KEYSWITCH_STATE_t Get_Keyswitch_State(void)
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


static void Update_Keyswitch_State(void)
{
    // get the current switch state
    const KEYSWITCH_STATE_t keyswitch_state_this = Get_Keyswitch_State();
    
    if (keyswitch_state_this != m_state.keyswitch_state_prev)
    {
        switch (keyswitch_state_this)
        {
            case KEYSWITCH_STATE_ARMED:
                printf("System arming...\r\n");
                Msg_Remote_Arm(NODEID_REMOTE0, 100);
                Msg_Remote_Arm(NODEID_REMOTE1, 100);
                Msg_Remote_Arm(NODEID_REMOTE2, 100);
                m_system_armed = true;
                break;

            case KEYSWITCH_STATE_OFF:
            case KEYSWITCH_STATE_TEST:
            case KEYSWITCH_STATE_FAULT:
            default:
                printf("System disarming...\r\n");
                Msg_Remote_Disarm(NODEID_REMOTE0, 100);
                Msg_Remote_Disarm(NODEID_REMOTE1, 100);
                Msg_Remote_Disarm(NODEID_REMOTE2, 100);
                m_system_armed = false;
                break;

        } // switch (mode change)

        // store it for next time
        m_state.keyswitch_state_prev = keyswitch_state_this;

    }
    else
    {
        // do nothing
    }
} // Update_Keyswitch_State()


bool System_Is_Armed(void)
{
    return m_system_armed;
    
} // System_Is_Armed()


bool System_Show_Is_Running(void)
{
    return m_show_is_running;
    
} // System_Show_Is_Running()


void System_Bump_Show_Time_ms(uint32_t time_ms)
{
    // disable interrupt
    Task_1ms_High_Priority_Disable();
    
    // get the count
    m_show_time_ms += time_ms;

    // restore the interrupt enable
    Task_1ms_High_Priority_Enable();
} // System_Bump_Show_Time_ms()


uint32_t System_Get_Show_Time_ms(void)
{
    // this is not atomic, so need to disable interrupts

    // will store the count
    uint32_t show_time_ms_latched = 0;

    // disable interrupt
    Task_1ms_High_Priority_Disable();
    
    // get the count
    show_time_ms_latched = m_show_time_ms;

    // restore the interrupt enable
    Task_1ms_High_Priority_Enable();

    // return the count
    return show_time_ms_latched;

} // System_Get_Show_Time_ms()


void Task_1ms_Low_Priority_Handler(void)
{
} // Task_1ms_Low_Priority_Handler()


void Task_1ms_High_Priority_Handler(void)
{
    if (System_Show_Is_Running())
    {
        m_show_time_ms++;
    }
} // Task_1ms_High_Priority_Handler()
