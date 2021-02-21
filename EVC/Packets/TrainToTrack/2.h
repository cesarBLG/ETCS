#pragma once
#include "../types.h"
#include "../variables.h"
#include "../packets.h"
#include <vector>

struct OnboardSupportedSystemVersion : ETCS_packet
{
    N_ITER_t N_ITER;
    std::vector<M_VERSION_t> M_VERSIONs;
    OnboardSupporSysVer() = default;
    OnboardSupporSysVer(bit_read_temp &r)
    {
        r.read(&NID_PACKET);
        r.read(&L_PACKET);
        M_VERSION_t m_version;
        r.read(&m_version);
        M_VERSIONs.push_back(m_version);
        r.read(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            r.read(&m_version);
            M_VERSIONs.push_back(m_version);
        }
    }
    OnboardSupportedSystemVersion *create(bit_read_temp &r) override
    {
        return new OnboardSupportedSystemVersion(r);
    }
    void serialize(bit_write &w) override
    {
        w.write(&NID_PACKET);
        w.write(&L_PACKET);
        w.write(&M_VERSIONs[0]);
        w.write(&N_ITER);
        for (int i=0; i<N_ITER; i++) {
            w.write(&M_VERSIONs[i+1]);
        }
    }
};