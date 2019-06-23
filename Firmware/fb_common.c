#include "fb_common.h"

uint16_t Remote_NodeID_to_Integer(uint8_t nodeid)
{
    return nodeid - NODEID_REMOTE_BASE;
    
} //  Remote_NodeID_to_Integer()
