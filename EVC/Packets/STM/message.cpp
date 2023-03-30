#include "message.h"
#include "6.h"
#include "13.h"
#include "15.h"
#include "18.h"
#include "32.h"
#include "35.h"
#include "38.h"
#include "39.h"
#include "43.h"
#include "46.h"
#include "128.h"
#include "129.h"
#include "130.h"
#include "175.h"
#include "179.h"
#include "181.h"
#include "183.h"
#include "184.h"
ETCS_packet *construct_stm_packet(bit_manipulator &r)
{
    int pos = r.position;
    NID_PACKET_t NID_PACKET;
    r.peek(&NID_PACKET);
    ETCS_packet *p;
    switch ((unsigned char)NID_PACKET) {
        case 6: p = new STMOverrideActivation(); break;
        case 13: p = new STMStateRequest(); break;
        case 15: p = new STMStateReport(); break;
        case 18: p = new STMNationalTrip(); break;
        case 32: p = new STMIconRequest(); break;
        case 35: p = new STMIconRequest(); break;
        case 38: p = new STMTextMessage(); break;
        case 39: p = new STMDeleteTextMessage(); break;
        case 43: p = new STMSupervisionInformation(); break;
        case 46: p = new STMSoundCommand(); break;
        case 128: p = new STMBrakeCommand(); break;
        case 129: p = new STMSpecificBrakeCommand(); break;
        case 130: p = new STMTrainCommand(); break;
        case 175: p = new STMTrainData(); break;
        case 179: p = new STMSpecificDataEntryRequest(); break;
        case 181: p = new STMSpecificDataNeed(); break;
        case 183: p = new STMSpecificDataView(); break;
        case 184: p = new STMDataEntryFlag(); break;
        default: p = new ETCS_packet();
    }
    p->copy(r);
    if (NID_PACKET != 0 && r.position-pos != p->L_PACKET)
         r.error = true;
    return p;
}
stm_message::stm_message(bit_manipulator &b)
{
    NID_STM.copy(b);
    L_MESSAGE.copy(b);
    while (!b.error && b.position<=(b.bits.size()*8-8))
    {
        NID_PACKET_t NID_PACKET;
        b.peek(&NID_PACKET);
        if (NID_PACKET==255)
            break;
        
        packets.push_back(std::shared_ptr<ETCS_packet>(construct_stm_packet(b)));
    }
    readerror = b.error;
    valid = !b.sparefound;
}