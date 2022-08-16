#pragma once
#include <set>
#include "../Time/clock.h"
struct virtual_balise_cover
{
    int NID_C;
    int NID_VBCMK;
    long validity;
    bool operator<(const virtual_balise_cover &o) const
    {
        return NID_VBCMK < o.NID_VBCMK;
    }
};
extern std::set<virtual_balise_cover> vbcs;
void set_vbc(virtual_balise_cover vbc);
void remove_vbc(virtual_balise_cover vbc);
bool vbc_ignored(int nid_c, int nid_vbcmk);
void load_vbcs();