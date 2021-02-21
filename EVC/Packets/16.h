
#include "packets.h"
struct RepositioningInformation : ETCS_directional_packet
{
    Q_SCALE_t Q_SCALE;
    L_SECTION_t L_SECTION;
    RepositioningInformation() {}
    RepositioningInformation(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&Q_DIR);
        r.read(&L_PACKET);
        r.read(&Q_SCALE);
        r.read(&L_SECTION);
    }
    RepositioningInformation *create(bit_read_temp &r) override
    {
        return new RepositioningInformation(r);
    }
};