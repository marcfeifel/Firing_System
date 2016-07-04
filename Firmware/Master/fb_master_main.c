#include <stdio.h>

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

static bool m_system_armed = false;

static KEYSWITCH_STATE_t Get_Keyswitch_State(void);
static void Update_Keyswitch_State(void);

static struct
{
    KEYSWITCH_STATE_t keyswitch_state_prev;
    
} m_state;

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
                default:
                    break;
            }
        }
        else
        {
            Msg_Keep_Alive(NODEID_REMOTE0);
            Msg_Keep_Alive(NODEID_REMOTE1);
            Msg_Keep_Alive(NODEID_REMOTE2);

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


void Task_1ms_Low_Priority_Handler(void)
{
} // Task_1ms_Low_Priority_Handler()


void Task_1ms_High_Priority_Handler(void)
{
} // Task_1ms_High_Priority_Handler()
