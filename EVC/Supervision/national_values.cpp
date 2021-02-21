/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "national_values.h"
#include "../optional.h"
#define TO_MPS(kph) kph/3.6
#include <limits>
bool Q_NVDRIVER_ADHES;

double V_NVSHUNT;
double V_NVSTFF;
double V_NVONSIGHT;
double V_NVLIMSUPERV;
double V_NVUNFIT;
double V_NVREL;

double D_NVROLL;

bool Q_NVSBTSMPERM;
bool Q_NVEMRRLS;
bool Q_NVGUIPERM;
bool Q_NVSBFBPERM;
bool Q_NVINHSMICPERM;

double V_NVALLOWOVTRP;
double V_NVSUPOVTRP;

double D_NVOVTRP;

double T_NVOVTRP;

int M_NVDERUN;
int M_NVCONTACT;

double T_NVCONTACT;

double D_NVPOTRP;

double D_NVSTFF;

double Q_NVLOCACC;

int M_NVAVADH;

double M_NVEBCL;

std::map<double,double> NV_KRINT;
std::map<double,double> NV_KVINT_freight;
std::map<double,kvint_pass_step> NV_KVINT_pass;
double M_NVKTINT;

double A_NVMAXREDADH1;
double A_NVMAXREDADH2;
double A_NVMAXREDADH3;
#include <fstream>
void nv_changed()
{
    /*set_conversion_correction_values();
    SR_dist;
    SR_speed;
    recalculate_MRSP();*/
}
void set_default_national()
{
    Q_NVDRIVER_ADHES = false;
    V_NVSHUNT = TO_MPS(30);
    V_NVSTFF = TO_MPS(40);
    V_NVONSIGHT = TO_MPS(30);
    V_NVLIMSUPERV = TO_MPS(100);
    V_NVUNFIT = TO_MPS(100);
    V_NVREL = TO_MPS(40);

    D_NVROLL = 2;

    Q_NVSBTSMPERM = true;
    Q_NVEMRRLS = false;
    Q_NVGUIPERM = false;
    Q_NVSBFBPERM = false;
    Q_NVINHSMICPERM = false;

    V_NVALLOWOVTRP = 0;
    V_NVSUPOVTRP = TO_MPS(30);

    D_NVOVTRP = 200;

    T_NVOVTRP = 60;

    M_NVDERUN = 0;
    M_NVCONTACT = 0;

    T_NVCONTACT = std::numeric_limits<double>::infinity();

    D_NVPOTRP = 200;

    D_NVSTFF = std::numeric_limits<double>::infinity();

    Q_NVLOCACC = 12;

    M_NVAVADH = 0;

    M_NVEBCL = 0.999999999f;

    NV_KRINT[0] = 0.9;
    NV_KVINT_freight[0] = 0.7;
    NV_KVINT_pass[0] = {0.7,0.7,0,0};
    M_NVKTINT=1.1;

    A_NVMAXREDADH1 = 1;
    A_NVMAXREDADH2 = 0.7;
    A_NVMAXREDADH3 = 0.7;
}
void load_national_values(NationalValues nv)
{
    set_default_national();
    V_NVSHUNT = nv.V_NVSHUNT.get_value();
    V_NVSTFF = nv.V_NVSTFF.get_value();
    V_NVONSIGHT = nv.V_NVONSIGHT.get_value();
    V_NVLIMSUPERV = nv.V_NVLIMSUPERV.get_value();
    V_NVUNFIT = nv.V_NVUNFIT.get_value();
    V_NVREL = nv.V_NVREL.get_value();

    D_NVROLL = nv.D_NVROLL.get_value(nv.Q_SCALE);

    V_NVALLOWOVTRP = nv.V_NVALLOWOVTRP.get_value();
    V_NVSUPOVTRP = nv.V_NVSUPOVTRP.get_value();

    D_NVOVTRP = nv.D_NVOVTRP.get_value(nv.Q_SCALE);
    T_NVOVTRP = nv.T_NVOVTRP;
    D_NVPOTRP = nv.D_NVPOTRP.get_value(nv.Q_SCALE);

    M_NVCONTACT = nv.M_NVCONTACT;
    T_NVCONTACT = nv.T_NVCONTACT;

    M_NVDERUN = nv.M_NVDERUN == M_NVDERUN_t::Yes;

    D_NVSTFF = nv.D_NVSTFF.get_value(nv.Q_SCALE);

    Q_NVDRIVER_ADHES = nv.Q_NVDRIVER_ADHES == Q_NVDRIVER_ADHES_t::Allowed;

    A_NVMAXREDADH1 = nv.A_NVMAXREDADH1.get_value();
    A_NVMAXREDADH2 = nv.A_NVMAXREDADH2.get_value();
    A_NVMAXREDADH3 = nv.A_NVMAXREDADH3.get_value();

    Q_NVLOCACC = nv.Q_NVLOCACC.get_value();

    M_NVAVADH = nv.M_NVAVADH;

    M_NVEBCL = nv.M_NVEBCL.get_value();

    if (nv.Q_NVKINT == Q_NVKINT_t::CorrectionFollow) {
        std::vector<KVINT_element> kv;
        kv.push_back(nv.element_kv);
        kv.insert(kv.end(), nv.elements_kv.begin(), nv.elements_kv.end());
        for (KVINT_element e : kv) {
            if (e.Q_NVKVINTSET == Q_NVKVINTSET_t::ConventionalPassengerTrains) {
                NV_KVINT_pass.clear();
                std::vector<KVINT_step_element> kvp;
                kvp.push_back(e.element);
                kvp.insert(kvp.end(), e.elements.begin(), e.elements.end());
                for (KVINT_step_element s : kvp) {
                    kvint_pass_step ps;
                    ps.a = s.M_NVKVINT1.get_value();
                    ps.b = s.M_NVKVINT2.get_value();
                    ps.A_NVP12 = e.A_NVP12.get_value();
                    ps.A_NVP23 = e.A_NVP23.get_value();
                    NV_KVINT_pass[s.V_NVKVINT.get_value()] = ps;
                }
            } else if (e.Q_NVKVINTSET == Q_NVKVINTSET_t::FreightTrains) {
                NV_KVINT_freight.clear();
                std::vector<KVINT_step_element> kvf;
                kvf.push_back(e.element);
                kvf.insert(kvf.end(), e.elements.begin(), e.elements.end());
                for (KVINT_step_element s : kvf) {
                    NV_KVINT_freight[s.V_NVKVINT.get_value()] = s.M_NVKVINT1.get_value();
                }
            }
        }
        std::vector<KRINT_element> kr;
        kr.push_back(nv.element_kr);
        kr.insert(kr.end(), nv.elements_kr.begin(), nv.elements_kr.end());
        NV_KRINT.clear();
        for (KRINT_element e : kr) {
            NV_KRINT[e.L_NVKRINT.get_value()] = e.M_NVKRINT.get_value();
        }
        M_NVKTINT = nv.M_NVKTINT.get_value();
    }
    nv_changed();

#ifdef __ANDROID__
    std::ofstream file("/data/data/com.etcs.dmi/nationalvalues.bin", std::ios::binary);
#else
    std::ofstream file("nationalvalues.bin", std::ios::binary);
#endif
    bit_write w;
    nv.serialize(w);
    for (int i=0; i<w.bits.size()/8; i++) {
        char value = 0;
        for (int j=0; j<8; j++) {
            int index = i*8 + j;
            bool bit = false;
            if (index < w.bits.size() && w.bits[index]) bit = true;
            value = (value<<1) | bit;
        }
        file.write(&value, 1);
    }
}
struct StoredNationalValueSet
{
    distance first_applicable;
    NationalValues nv;
};
optional<StoredNationalValueSet> not_yet_applicable_nv;
void national_values_received(NationalValues nv, distance reference)
{
    not_yet_applicable_nv = {};
    if (nv.D_VALIDNV == D_VALIDNV_t::Now) {
        load_national_values(nv);
    } else {
        not_yet_applicable_nv = {reference, nv};
    }
}
void setup_national_values()
{
#ifdef __ANDROID__
    std::ifstream file("/data/data/com.etcs.dmi/nationalvalues.bin", std::ios::binary);
#else
    std::ifstream file("nationalvalues.bin", std::ios::binary);
#endif
    std::vector<bool> message;
    while (!file.eof() && !file.fail()) {
        char c;
        file.read(&c, 1);
        for (int i=7; i>=0; i--) {
            message.push_back((c>>i)&1);
        }
    }
    bit_read_temp r(message);
    NationalValues nv = NationalValues(r);
    if (r.error || r.sparefound || r.position != nv.L_PACKET) {
        set_default_national();
        nv_changed();
    } else{
        load_national_values(nv);
    }
}
void update_national_values()
{
    if (not_yet_applicable_nv && not_yet_applicable_nv->first_applicable < d_estfront) {
        load_national_values(not_yet_applicable_nv->nv);
        not_yet_applicable_nv = {};
    }
}
