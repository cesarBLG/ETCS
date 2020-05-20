#include "packets.h"
#include "5.h"
#include "12.h"
#include "21.h"
#include "27.h"
#include "41.h"
#include "47.h"
#include "65.h"
#include "66.h"
#include "68.h"
#include "72.h"
#include "76.h"
#include "79.h"
#include "80.h"
#include "136.h"
#include "137.h"
std::map<int, ETCS_packet*> ETCS_packet::packet_factory;
void ETCS_packet::initialize()
{
    packet_factory[5] = new Linking();
    packet_factory[12] = new Level1_MA();
    packet_factory[21] = new GradientProfile();
    packet_factory[27] = new InternationalSSP();
    packet_factory[41] = new LevelTransitionOrder();
    packet_factory[47] = new ConditionalLevelTransitionOrder();
    packet_factory[65] = new TemporarySpeedRestriction();
    packet_factory[66] = new TemporarySpeedRevocation();
    packet_factory[68] = new TrackCondition();
    packet_factory[72] = new PlainTextMessage();
    packet_factory[76] = new FixedTextMessage();
    packet_factory[79] = new GeographicalPosition();
    packet_factory[80] = new ModeProfile();
    packet_factory[136] = new InfillLocationReference();
    packet_factory[137] = new StopIfInSR();
}
ETCS_packet *ETCS_packet::construct(bit_read_temp &r)
{
    int pos = r.position;
    NID_PACKET_t NID_PACKET;
    r.peek(&NID_PACKET);
    ETCS_packet *p;
    if (packet_factory.find(NID_PACKET) == packet_factory.end()) {
        p = new ETCS_directional_packet(r);
    } else {
        p = packet_factory[NID_PACKET]->create(r);
    }
    if (r.position-pos != p->L_PACKET)
         r.error = true;
    return p;
}