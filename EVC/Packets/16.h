
#include "packets.h"
struct RepositioningInformation : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    L_SECTION_t L_SECTION;
    void copy(bit_manipulator &r) override
    {
        NID_PACKET.copy(r);
        Q_DIR.copy(r);
        L_PACKET.copy(r);
        Q_SCALE.copy(r);
        L_SECTION.copy(r);
    }
};