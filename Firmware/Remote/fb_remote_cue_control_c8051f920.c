#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "fb_common.h"
#include "fb_remote.h"
#include "fb_remote_cue.h"
#include "fb_c51_target.h"

// how long to assert power when firing a cue
#define FB_FIRE_HOLD_TIME_MS   5

// minimum pulse-width is 5ns
#define FB_74HC393_DELAY()     do { NOP(); NOP(); NOP(); NOP(); } while (0)
#define FB_74HC393_PULSE(_pin) do { FB_74HC393_DELAY(); (_pin) = 1; FB_74HC393_DELAY(); (_pin) = 0; } while (0)

static struct
{
    uint8_t fire_asserted;
    
    uint8_t test_asserted;
    
} m_state = { 0 };


void fb_Remote_Cue_Init(void)
{
    fb_Remote_Cue_Clear_FireTest();

} // fb_Remote_Cue_Init()


void fb_Remote_Cue_Clear_FireTest(void)
{
    // assert the reset/clear signal
    PIN_74HC393_CLEAR_O = 1;
    FB_74HC393_DELAY();

    // set both clock pins high to put them into their idle state
    PIN_74HC393_TEST_CLK_O = 0;
    PIN_74HC393_FIRE_CLK_O = 0;
    FB_74HC393_DELAY();

    // de-assert the reset/clear signal
    PIN_74HC393_CLEAR_O = 0;
    FB_74HC393_DELAY();
    
    m_state.fire_asserted = false;
    m_state.test_asserted = false;

} // fb_Remote_Cue_Clear_FireTest()


bool fb_Remote_Cue_Assert_Fire(void)
{
    if (fb_Remote_Is_Armed())
    {
        // pulse it 8 times
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 1
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 2
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 3
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 4
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 5
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 6
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 7
        FB_74HC393_PULSE(PIN_74HC393_FIRE_CLK_O); // 8

        // fire was indeed asserted
        m_state.fire_asserted = true;

    }
    else
    {
        // system isn't armed
        m_state.fire_asserted = false;
        
    } // if (fb_Remote_Is_Armed())

    // indicate whether we fired the cue or not
    return m_state.fire_asserted;

} // fb_Remote_Cue_Assert_Fire()


bool fb_Remote_Cue_Fire_is_Asserted(void)
{
    return m_state.fire_asserted;

} // fb_Remote_Cue_Fire_is_Asserted()


void fb_Remote_Cue_Assert_Test(void)
{
    // make sure the signals aren't asserted
    fb_Remote_Cue_Clear_FireTest();

    // pulse it 8 times
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 1
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 2
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 3
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 4
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 5
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 6
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 7
    FB_74HC393_PULSE(PIN_74HC393_TEST_CLK_O); // 8

    m_state.test_asserted = true;
    
} // FireTest_Assert_Test()


bool fb_Remote_Cue_Test_is_Asserted(void)
{
    return m_state.test_asserted;

} // fb_Remote_Cue_Test_is_Asserted()


void fb_Remote_Cue_Select(SOCKET_ENUM_t socket, CUE_ENUM_t cue)
{
    /* *********************
       TODO - REMOVE THIS!!!
       ********************* */
//    cue = cue & 0x3;
//    socket = 0;
    
    // make sure we don't immediately fire
    fb_Remote_Cue_Clear_FireTest();

    // setup the cue address
    PIN_ADDRESS_CUEb0_O = cue & BIT0 ? 1 : 0;
    PIN_ADDRESS_CUEb1_O = cue & BIT1 ? 1 : 0;
    PIN_ADDRESS_CUEb2_O = cue & BIT2 ? 1 : 0;
    PIN_ADDRESS_CUEb3_O = cue & BIT3 ? 1 : 0;

    // setup the socket address
    PIN_ADDRESS_SOCKb0_O = socket & BIT0 ? 1 : 0;
    PIN_ADDRESS_SOCKb1_O = socket & BIT1 ? 1 : 0;
    PIN_ADDRESS_SOCKb2_O = socket & BIT2 ? 1 : 0;

} // Cue_Select()


bool fb_Remote_Cue_Is_Present(void)
{
    // assume the cue is present
    bool cue_is_present = false;

    // assert the test signal
    fb_Remote_Cue_Assert_Test();

    // wait for things to settle
    Sleep(2);

    // see if the cue is not empty
    cue_is_present = !PIN_CUE_EMPTY_I;

    // de-assert the test signal
    fb_Remote_Cue_Clear_FireTest();

    // return the result
    return cue_is_present;

} // Cur_Is_Present()


void fb_Remote_Cue_Scan_All(REMOTE_CUES_t * p_remote)
{
    // will iterate through the sockets
    uint8_t socket = 0;

    // assume all cues empty by clearing all memory
    memset(p_remote, 0, sizeof(REMOTE_CUES_t));

    // iterate through the sockets
    for (socket = 0; socket < SOCKETS_NUM_OF; socket++)
    {
        // will iterate through the cues
        uint8_t cue;

        // iterate through the cues
        for (cue = 0; cue < CUES_NUM_OF; cue++)
        {
            // select the cue
            fb_Remote_Cue_Select(socket, cue);

            // check if it is present or not
            if (fb_Remote_Cue_Is_Present())
            {
                // it is present - set the appropriate bit
                BIT_SET(p_remote->sockets[socket], cue);
            }
            else
            {
                // it is not present - already cleared memory
            }
        } // for (cue)
    } // for (socket)
} // Cue_Scan_All()


uint32_t fb_Remote_Cue_Fire_Time_ms(void)
{
    return FB_FIRE_HOLD_TIME_MS;

} // fb_Remote_Cue_Fire_Time_ms()
