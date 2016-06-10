#include "fb_messages.h"

static FB_MSG_XMIT_DESCRIPTOR msg = {0};

static struct
{
    uint8_t in_progress;

    uint32_t start_ms;

    uint32_t timeout_ms;

} m_ping_state;


#if 0
void Ping(uint8_t dest, uint32_t timeout_ms)
{
} // Ping()
#endif // 0

bool Ping_Complete(void)
{
    return true;

} // Ping_Complete


void Task_Ping(void)
{
    if (m_ping_state.in_progress)
    {
    }
    else
    {
        // do nothing
    }
} // Task_Ping()


void Task_Pong(void)
{
} // Task_Pong()