#include <stdio.h>
#include <string.h>

#include "fb_common.h"
#include "fb_master_hal.h"
#include "fb_messages.h"
#include "rfm69.h"

#define CON_ESC     "\x1B"
#define CON_CLEAR_LINE  CON_ESC "[2K"

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

static code uint32_t m_program[128][6] = {
    #include "program.inc."
};

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
    
    printf("\r\n" CON_CLEAR_LINE);
    
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
    HAL_deassert_uart_rts();


    printf("Start.\r\n" CON_CLEAR_LINE);

    Msg_Init();

    printf("Super loop started.\r\n" CON_CLEAR_LINE);

    printf( "\x1B[2J" );

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
                        Msg_Remote_Scan_Request(NODEID_REMOTE3, 1000);
                        Msg_Remote_Scan_Request(NODEID_REMOTE4, 1000);
                        Msg_Remote_Scan_Request(NODEID_REMOTE5, 1000);
                    }
                    break;
                
                case 'd':
                case 'D':
                    // download a show
                    printf("Download a new show?");
                    if (Get_Confirmation())
                    {
                        uint8_t i;
                        uint32_t the_show[128];
                        
                        for (i = 0; i < 128; i++)
                        {
                            the_show[i] = m_program[i][0];
                        }
                        Msg_Remote_Program(NODEID_REMOTE0, the_show);

                        for (i = 0; i < 128; i++)
                        {
                            the_show[i] = m_program[i][1];
                        }
                        Msg_Remote_Program(NODEID_REMOTE1, the_show);
  
                        for (i = 0; i < 128; i++)
                        {
                            the_show[i] = m_program[i][2];
                        }
                        Msg_Remote_Program(NODEID_REMOTE2, the_show);
  
                        for (i = 0; i < 128; i++)
                        {
                            the_show[i] = m_program[i][3];
                        }
                        Msg_Remote_Program(NODEID_REMOTE3, the_show);
  
                        for (i = 0; i < 128; i++)
                        {
                            the_show[i] = m_program[i][4];
                        }
                        Msg_Remote_Program(NODEID_REMOTE4, the_show);
  
                        for (i = 0; i < 128; i++)
                        {
                            the_show[i] = m_program[i][5];
                        }
                        Msg_Remote_Program(NODEID_REMOTE5, the_show);
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
                        Msg_Remote_Program_Commit(NODEID_REMOTE3);
                        Msg_Remote_Program_Commit(NODEID_REMOTE4);
                        Msg_Remote_Program_Commit(NODEID_REMOTE5);
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
                        Msg_Remote_Reset(NODEID_REMOTE3, 500);
                        Msg_Remote_Reset(NODEID_REMOTE4, 500);
                        Msg_Remote_Reset(NODEID_REMOTE5, 500);
                        Reset_MCU();
                    }
                    break;
                    
                default:

                    printf( "\x1B[0;0H" CON_CLEAR_LINE );

                    printf("\r\n" CON_CLEAR_LINE);
                    printf("Options:\r\n" CON_CLEAR_LINE);
                    printf("'s' - scan all cues\r\n" CON_CLEAR_LINE);
                    printf("'d' - download a show\r\n" CON_CLEAR_LINE);
                    printf("'r' - start/continue a show\r\n" CON_CLEAR_LINE);
                    printf("'f' - advance 30 seconds into show\r\n" CON_CLEAR_LINE);
                    printf("'p' - pause a show\r\n" CON_CLEAR_LINE);
                    printf("'c' - commit a show to flash\r\n" CON_CLEAR_LINE);
                    printf("'0' - global reset\r\n" CON_CLEAR_LINE);
                    printf("\r\n" CON_CLEAR_LINE);
                    break;
            }
        }
        else
        {
            
            printf( "\x1B[12;0H" CON_CLEAR_LINE );

            Msg_Keep_Alive(NODEID_BROADCAST);

            Msg_Ping(NODEID_REMOTE0, 100);
            Msg_Ping(NODEID_REMOTE1, 100);
            Msg_Ping(NODEID_REMOTE2, 100);
            Msg_Ping(NODEID_REMOTE3, 100);
            Msg_Ping(NODEID_REMOTE4, 100);
            Msg_Ping(NODEID_REMOTE5, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE0, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE1, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE2, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE3, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE4, 100);
            Msg_Remote_Status_Request(NODEID_REMOTE5, 100);
            printf("\r\n" CON_CLEAR_LINE);
            
            Sleep(500);
        }
    }
} // main()


// read the key-switch inputs and interpret
static KEYSWITCH_STATE_t Get_Keyswitch_State(void)
{
    // get the state of the pins
    const bool pin_armed = HAL_get_pin_switch_armed();
    const bool pin_test = HAL_get_pin_switch_test();

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
                printf("System arming...\r\n" CON_CLEAR_LINE);
                Msg_Remote_Arm(NODEID_REMOTE0, 100);
                Msg_Remote_Arm(NODEID_REMOTE1, 100);
                Msg_Remote_Arm(NODEID_REMOTE2, 100);
                Msg_Remote_Arm(NODEID_REMOTE3, 100);
                Msg_Remote_Arm(NODEID_REMOTE4, 100);
                Msg_Remote_Arm(NODEID_REMOTE5, 100);
                m_system_armed = true;
                break;

            case KEYSWITCH_STATE_OFF:
            case KEYSWITCH_STATE_TEST:
            case KEYSWITCH_STATE_FAULT:
            default:
                printf("System disarming...\r\n" CON_CLEAR_LINE);
                Msg_Remote_Disarm(NODEID_REMOTE0, 100);
                Msg_Remote_Disarm(NODEID_REMOTE1, 100);
                Msg_Remote_Disarm(NODEID_REMOTE2, 100);
                Msg_Remote_Disarm(NODEID_REMOTE3, 100);
                Msg_Remote_Disarm(NODEID_REMOTE4, 100);
                Msg_Remote_Disarm(NODEID_REMOTE5, 100);
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
