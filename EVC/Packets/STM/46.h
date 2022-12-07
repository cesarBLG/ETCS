#pragma once
#include "../packets.h"
#include "stm_variables.h"
#include <vector>
struct STMSoundPart
{
    M_FREQ_t M_FREQ;
    T_SOUND_t T_SOUND;
    void copy(bit_manipulator &w)
    {
        M_FREQ.copy(w);
        T_SOUND.copy(w);
    }
};
struct STMSoundDefinition
{
    NID_SOUND_t NID_SOUND;
    Q_SOUND_t Q_SOUND;
    N_ITER_t N_ITER;
    std::vector<STMSoundPart> parts;
    void copy(bit_manipulator &w)
    {
        NID_SOUND.copy(w);
        Q_SOUND.copy(w);
        N_ITER.copy(w);
        parts.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            parts[i].copy(w);
        }
    }
};
struct STMSoundCommand : ETCS_packet
{
    N_ITER_t N_ITER;
    std::vector<STMSoundDefinition> sounds;
    void copy(bit_manipulator &w) override
    {
        NID_PACKET.copy(w);
        L_PACKET.copy(w);
        N_ITER.copy(w);
        sounds.resize(N_ITER);
        for (int i=0; i<N_ITER; i++) {
            sounds[i].copy(w);
        }
    }
};
