#include "packets.h"
#include "5.h"
#include "12.h"
#include "21.h"
#include "27.h"
#include "41.h"
#include "65.h"
#include "66.h"
#include "68.h"
#include "80.h"
#include "136.h"
std::map<int, ETCS_packet*> ETCS_packet::packet_factory;
void ETCS_packet::initialize()
{
    packet_factory[5] = new Linking();
    packet_factory[12] = new Level1_MA();
    packet_factory[21] = new GradientProfile();
    packet_factory[27] = new InternationalSSP();
    packet_factory[41] = new LevelTransitionOrder();
    packet_factory[65] = new TemporarySpeedRestriction();
    packet_factory[66] = new TemporarySpeedRevocation();
    packet_factory[68] = new TrackCondition();
    packet_factory[80] = new ModeProfile();
    packet_factory[136] = new InfillLocationReference();
}