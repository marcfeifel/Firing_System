#include <stdio.h>

#include "RFM69.h"
#include "fb_messages.h"

static FB_MSG_XMIT_DESCRIPTOR * p_xmit_queue = NULL;
static bool msg_received = false;

static void Msg_Run_Xmit(void);
static void Msg_Run_Recv(void);

void Msg_Init(void)
{
    // initialize the HopeRF radio
    RFM69_construct(true);
    
    RFM69_initialize();

} // Msg_Init()


void Msg_Run(void)
{
    Msg_Run_Recv();
    
    Msg_Run_Xmit();
    
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


void Msg_Enqueue_for_Xmit(uint8_t dest, void const * payload, uint8_t payload_size, FB_MSG_XMIT_DESCRIPTOR * p_message)
{
    // be sure that the descriptor is initialized correctly
    p_message->transmit_complete = false;
    p_message->p_next_message = NULL;
    p_message->dest = dest;
    p_message->payload = payload;
    p_message->payload_size = payload_size;
    
    // if the queue is empty...
    if (NULL == p_xmit_queue)
    {
        // then just store this as the head
        p_xmit_queue = p_message;
        
    }
    else
    {
        // else the queue is not empty...
        FB_MSG_XMIT_DESCRIPTOR * p_iterator = p_xmit_queue;
        
        // find the end of the queue
        while (NULL != p_iterator->p_next_message)
        {
            // loop until we find the end of the list
        }
        
        // put it at the end of the queue
        p_iterator->p_next_message = p_message;
        
    }
} // Msg_Enqueue_for_Xmit()


bool Msg_Xmit_Is_Complete(FB_MSG_XMIT_DESCRIPTOR const * p_message)
{
    return p_message->transmit_complete;

} // Msg_Xmit_Complete()


static void Msg_Run_Xmit(void)
{
    // is the queue populated?
    if (NULL != p_xmit_queue)
    {
        // can we send?
        if (RFM69_canSend())
        {
            // send
            RFM69_send(p_xmit_queue->dest, p_xmit_queue->payload, p_xmit_queue->payload_size, false);
            
            // indicate message is transmitted
            p_xmit_queue->transmit_complete = true;
                
            // is there another message?
            if (NULL != p_xmit_queue->p_next_message)
            {
                // yes - move it to the head
                p_xmit_queue = p_xmit_queue->p_next_message;
                
            }
            else
            {
                // no - set the pointer to NULL
                p_xmit_queue = NULL;
                
            }            
        }
        else
        {
            // do nothing
        }
    }
    else
    {
        // do nothing
    }    
} // Msg_Run_Xmit()


static void Msg_Run_Recv(void)
{
    if (RFM69_receiveDone())
    {
        msg_received = true;
        
    }
    else
    {
        msg_received = false;
        
    }
} // Msg_Run_Recv()