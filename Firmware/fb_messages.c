#include <stdio.h>

#include "RFM69.h"
#include "fb_messages.h"

static bool msg_received = false;

static void Msg_Header_Fill(FB_MSG_ID_ENUM_t id, FB_MSG_BASE_t * p_msg);

void Msg_Init(void)
{
    // initialize the HopeRF radio
    RFM69_construct(true);

    RFM69_initialize();
    
    RFM69_promiscuous(true);

} // Msg_Init()


void Msg_Run(void)
{
    if (RFM69_receiveDone())
    {
        msg_received = true;

    }
    else
    {
        msg_received = false;

    }
} // Msg_Run()


bool Msg_Received(void)
{
    return msg_received;

} // Msg_Was_Received()


bool Msg_Is_For_Me(void)
{
    return msg_received;

} // Msg_Is_For_Me()


uint8_t Msg_Get_Sender(void)
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


void Msg_Enqueue_for_Xmit(FB_MSG_ID_ENUM_t id, uint8_t dest, void const * payload, uint8_t payload_size)
{
    
    Msg_Header_Fill(id, (FB_MSG_BASE_t*)payload);
    
    // send
    RFM69_send(dest, payload, payload_size, false);

} // Msg_Enqueue_for_Xmit()


bool Msg_Xmit_Is_Complete(void)
{
    return true;

} // Msg_Xmit_Complete()


static void Msg_Header_Fill(FB_MSG_ID_ENUM_t id, FB_MSG_BASE_t * p_msg)
{
    p_msg->id = id;
    p_msg->time_ms = millis();
    p_msg->rssi = Msg_Get_RSSI();
    
} // Msg_Header_Fill()
