#pragma once
#include "../packets.h"
struct STMOverrideStatus : ETCS_packet
{
    STMOverrideStatus()
    {
        NID_PACKET.rawdata = 7;
    }
    Q_OVR_STATUS_t Q_OVR_STATUS;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        Q_OVR_STATUS.copy(w);
    }
};
