#pragma once
#include <list>
#include <memory>
#include "../Position/distance.h"
#include "../Position/linking.h"
#include "messages.h"
struct etcs_information
{
    int index;
    virtual void handle() = 0;
    distance ref;
    int64_t timestamp;
    bool infill;
    bool fromRBC;
    int dir;
    bg_id nid_bg;
    std::list<std::shared_ptr<ETCS_packet>> linked_packets;
    etcs_information() : index(-1){}
    etcs_information(int index) : index(index){}
};