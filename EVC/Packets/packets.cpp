#include "packets.h"
#include "5.h"
#include "12.h"
#include "21.h"
#include "27.h"
std::map<int, ETCS_packet*> ETCS_packet::packet_factory;
void ETCS_packet::initialize()
{
    packet_factory[5] = new Linking();
    packet_factory[12] = new Level1_MA();
    packet_factory[21] = new GradientProfile();
    packet_factory[27] = new InternationalSSP();
}