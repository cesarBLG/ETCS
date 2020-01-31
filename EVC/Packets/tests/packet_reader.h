#ifndef _PACKET_READER_H
#define _PACKET_READER_H
#include "antenna.h"
#include "parsed_packet.h"
#include "MA/movement_authority.h"
void start_packet_reader(){}
void handle_packet(parsed_packet &packet)
{
    int pid=0;
    if(packet.peek("NID_PACKET", pid)!=-1)
    {
        switch(pid)
        {
            case 11:
                new movement_authority(packet);
                break;
        }
    }
}
#endif
