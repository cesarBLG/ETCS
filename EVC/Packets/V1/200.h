#pragma once
#include "../0.h"
namespace V1
{
struct VirtualBaliseCoverMarker : ::VirtualBaliseCoverMarker
{
    Q_DIR_t Q_DIR;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        NID_VBCMK.copy(r);
    }
};
}