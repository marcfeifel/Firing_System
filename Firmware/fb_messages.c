#include "fb_messages.h"
#include "RFM69.h"

static bool msg_received = false;

static void Msg_Header_Fill(FB_MSG_ID_ENUM_t id, FB_MSG_BASE_t * p_msg);

void Msg_Init(void)
{
    // initialize the HopeRF radio
    RFM69_construct(true);

    RFM69_initialize();
    
    RFM69_promiscuous(false);

} // Msg_Init()


bool Msg_Run(uint32_t timeout_ms)
{
    timeout_ms += millis();
    
    do
    {
        if (RFM69_receiveDone())
        {
            msg_received = true;

        }
        else
        {
            msg_received = false;

        }
    } while (!msg_received && (millis() < timeout_ms));
    
    return msg_received;
} // Msg_Run()


void Msg_ACK(void)
{
    RFM69_sendACK(NULL, 0);

} // Msg_ACK()


bool Msg_Received(void)
{
    return msg_received;

} // Msg_Was_Received()


bool Msg_Is_For_Me(void)
{
    return RFM69_getTargetID() == NODEID_LOCAL;

} // Msg_Is_For_Me()


uint8_t Msg_Get_Sender_ID(void)
{
    return RFM69_getSenderID();

} // Msg_Get_Sender()


uint8_t Msg_Get_Payload_Size(void)
{
    return RFM69_getDataLen();

} // Msg_Get_Payload_Size()


void const * Msg_Get_Payload_Ptr(void)
{
    return RFM69_getDataPtr();

} // Msg_Get_Ptr()


int16_t Msg_Get_RSSI(void)
{
    return RFM69_getRSSI();

} // Msg_Get_RSSI()


bool Msg_Send(FB_MSG_ID_ENUM_t id, uint8_t dest, void const * payload, uint8_t payload_size, uint32_t retry_timeout_ms)
{
    bool success = true;
    
    Msg_Header_Fill(id, (FB_MSG_BASE_t*)payload);
    
    // send
    if (retry_timeout_ms)
    {
        // 3 retries, 40ms per retry
        success = RFM69_sendWithRetry(dest, payload, payload_size, 3, retry_timeout_ms);
    }
    else
    {
        RFM69_send(dest, payload, payload_size, false);
        
    }
    
    return success;

} // Msg_Enqueue_for_Xmit()


static void Msg_Header_Fill(FB_MSG_ID_ENUM_t id, FB_MSG_BASE_t * p_msg)
{
    p_msg->id = id;
    
    p_msg->rssi = Msg_Get_RSSI();

} // Msg_Header_Fill()
