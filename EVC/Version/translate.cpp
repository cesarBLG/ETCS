/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "translate.h"
#include "version.h"
#include "../Supervision/supervision.h"
#include "../Supervision/train_data.h"
#include "../Packets/radio.h"
#include "../Packets/3.h"
#include "../Packets/V1/3.h"
#include "../Packets/V1/203.h"
#include "../Packets/27.h"
#include "../Packets/V1/27.h"
#include "../Packets/39.h"
#include "../Packets/V1/39.h"
#include "../Packets/51.h"
#include "../Packets/V1/51.h"
#include "../Packets/68.h"
#include "../Packets/79.h"
#include "../Packets/V1/79.h"
#include "../Packets/80.h"
#include "../Packets/V1/80.h"
#include "../Packets/TrainToTrack/4.h"
#include "../Packets/TrainToTrack/11.h"
#include "../Packets/TrainToTrack/V1/11.h"
std::shared_ptr<ETCS_packet> translate_packet(std::shared_ptr<ETCS_packet> packet, std::vector<std::shared_ptr<ETCS_packet>> packets, int version)
{
    if (VERSION_X(version) != 1)
        return packet;
    std::shared_ptr<ETCS_packet> trans = packet;
    switch (packet->NID_PACKET)
    {
        case 3: {
            auto *nv = (V1::NationalValues*)packet.get();
            auto *nv2 = new NationalValues();
            nv2->adhesion_nv_provided = false;
            nv2->version_2_provided = false;
            for (auto &p2 : packets) {
                if (p2->NID_PACKET == 203) {
                    nv2->adhesion_nv_provided = true;
                    auto *brak = (V1::NationalValuesBraking*)p2.get();
                    nv2->Q_NVGUIPERM = brak->Q_NVGUIPERM;
                    nv2->Q_NVSBFBPERM = brak->Q_NVSBFBPERM;
                    nv2->Q_NVINHSMICPERM = brak->Q_NVINHSMICPERM;
                    nv2->A_NVMAXREDADH1 = brak->A_NVMAXREDADH1;
                    nv2->A_NVMAXREDADH2 = brak->A_NVMAXREDADH2;
                    nv2->A_NVMAXREDADH3 = brak->A_NVMAXREDADH3;
                    nv2->M_NVAVADH = brak->M_NVAVADH;
                    nv2->M_NVEBCL = brak->M_NVEBCL;
                    nv2->Q_NVKINT = brak->Q_NVKINT;
                    nv2->element_kv = brak->element_kv;
                    nv2->N_ITER_kv = brak->N_ITER_kv;
                    nv2->elements_kv = brak->elements_kv;
                    nv2->element_kr = brak->element_kr;
                    nv2->N_ITER_kr = brak->N_ITER_kr;
                    nv2->elements_kr = brak->elements_kr;
                }
            }
            nv2->NID_PACKET = nv->NID_PACKET;
            nv2->Q_DIR = nv->Q_DIR;
            nv2->Q_SCALE = nv->Q_SCALE;
            nv2->D_VALIDNV = nv->D_VALIDNV;
            nv2->D_VALIDNV.Now = 32768;
            nv2->NID_C = nv->NID_Cs[0];
            nv2->N_ITER_c.rawdata = nv->N_ITER_c-1;
            for (int i=0; i<nv2->N_ITER_c; i++) {
                nv2->NID_Cs.push_back(nv->NID_Cs[i+1]);
            }
            nv2->V_NVSHUNT = nv->V_NVSHUNT;
            nv2->V_NVSTFF = nv->V_NVSTFF;
            nv2->V_NVONSIGHT = nv->V_NVONSIGHT;
            nv2->V_NVUNFIT = nv->V_NVUNFIT;
            nv2->V_NVREL = nv->V_NVREL;
            nv2->D_NVROLL = nv->D_NVROLL;
            nv2->Q_NVSBTSMPERM = nv->Q_NVSBTSMPERM;
            nv2->Q_NVEMRRLS = nv->Q_NVEMRRLS;
            nv2->V_NVALLOWOVTRP = nv->V_NVALLOWOVTRP;
            nv2->V_NVSUPOVTRP = nv->V_NVSUPOVTRP;
            nv2->D_NVOVTRP = nv->D_NVOVTRP;
            nv2->T_NVOVTRP = nv->T_NVOVTRP;
            nv2->D_NVPOTRP = nv->D_NVPOTRP;
            nv2->M_NVCONTACT = nv->M_NVCONTACT;
            nv2->T_NVCONTACT = nv->T_NVCONTACT;
            nv2->M_NVDERUN = nv->M_NVDERUN;
            nv2->D_NVSTFF = nv->D_NVSTFF;
            nv2->Q_NVDRIVER_ADHES = nv->Q_NVDRIVER_ADHES;
            trans = std::shared_ptr<ETCS_packet>(nv2);
            break;
        }
        case 27: {
            auto *sp = (V1::InternationalSSP*)packet.get();
            auto *sp2 = new InternationalSSP();
            sp2->NID_PACKET = sp->NID_PACKET;
            sp2->Q_DIR = sp->Q_DIR;
            sp2->Q_SCALE = sp->Q_SCALE;
            auto translate_element = [](const V1::SSP_element_packet &e) {
                SSP_element_packet e2;
                e2.Q_FRONT = e.Q_FRONT;
                e2.D_STATIC = e.D_STATIC;
                e2.V_STATIC = e.V_STATIC;
                auto translate_subelement = [](V1::SSP_diff d) {
                    SSP_diff d2;
                    d2.V_DIFF = d.V_DIFF;
                    d2.Q_DIFF.rawdata = d.NC_DIFF == 9 || d.NC_DIFF == 10 || d.NC_DIFF == 11;
                    switch(d.NC_DIFF.rawdata) {
                        case 0: d2.NC_CDDIFF.rawdata = 9; break;
                        case 1: d2.NC_CDDIFF.rawdata = 0; break;
                        case 2: d2.NC_CDDIFF.rawdata = 1; break;
                        case 3: d2.NC_CDDIFF.rawdata = 2; break;
                        case 4: d2.NC_CDDIFF.rawdata = 3; break;
                        case 5: d2.NC_CDDIFF.rawdata = 4; break;
                        case 6: d2.NC_CDDIFF.rawdata = 5; break;
                        case 7: d2.NC_CDDIFF.rawdata = 6; break;
                        case 8: d2.NC_CDDIFF.rawdata = 7; break;
                        case 9: d2.NC_DIFF.rawdata = 0; break;
                        case 10: d2.NC_DIFF.rawdata = 1; break;
                        case 11: d2.NC_DIFF.rawdata = 2; break;
                        case 12: d2.NC_CDDIFF.rawdata = 8; break;
                        case 13: d2.NC_CDDIFF.rawdata = 6; break;
                    }
                    return d2;
                };
                e2.N_ITER = e.N_ITER;
                for (auto &d : e.diffs) {
                    e2.diffs.push_back(translate_subelement(d));
                }
                return e2;
            };
            sp2->element = translate_element(sp->element);
            sp2->N_ITER = sp->N_ITER;
            for (auto &e : sp->elements) {
                sp2->elements.push_back(translate_element(e));
            }
            trans = std::shared_ptr<ETCS_packet>(sp2);
            break;
        }
        case 39: {
            for (auto &p2 : packets) {
                if (p2->NID_PACKET == 206) {
                    return nullptr;
                }
            }
            auto *tc = (V1::TrackConditionChangeTractionSystem*)packet.get();
            switch (tc->M_TRACTION.rawdata) {
                case 0: tc->M_VOLTAGE.rawdata = 0; break;
                case 1: tc->M_VOLTAGE.rawdata = 3; tc->NID_CTRACTION.rawdata = 10; break;
                case 2: tc->M_VOLTAGE.rawdata = 1; tc->NID_CTRACTION.rawdata = 12; break;
                case 3: tc->M_VOLTAGE.rawdata = 1; tc->NID_CTRACTION.rawdata = 13; break;
                case 5: tc->M_VOLTAGE.rawdata = 4; tc->NID_CTRACTION.rawdata = 14; break;
                case 6: tc->M_VOLTAGE.rawdata = 4; tc->NID_CTRACTION.rawdata = 1; break;
                case 7: tc->M_VOLTAGE.rawdata = 1; tc->NID_CTRACTION.rawdata = 2; break;
                case 8: tc->M_VOLTAGE.rawdata = 1; tc->NID_CTRACTION.rawdata = 3; break;
                case 11: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 19; break;
                case 12: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 20; break;
                case 13: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 21; break;
                case 15: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 22; break;
                case 26: tc->M_VOLTAGE.rawdata = 1; tc->NID_CTRACTION.rawdata = 11; break;
                case 31: tc->M_VOLTAGE.rawdata = 1; tc->NID_CTRACTION.rawdata = 18; break;
                case 32: tc->M_VOLTAGE.rawdata = 3; tc->NID_CTRACTION.rawdata = 15; break;
                case 33: tc->M_VOLTAGE.rawdata = 3; tc->NID_CTRACTION.rawdata = 16; break;
                case 34: tc->M_VOLTAGE.rawdata = 1; tc->NID_CTRACTION.rawdata = 17; break;
                case 41: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 4; break;
                case 42: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 5; break;
                case 43: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 6; break;
                case 44: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 7; break;
                case 45: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 8; break;
                case 46: tc->M_VOLTAGE.rawdata = 2; tc->NID_CTRACTION.rawdata = 9; break;
                default: return nullptr;
            }
            break;
        }
        case 51: {
            auto *asp = (V1::AxleLoadSpeedProfile*)packet.get();
            auto *asp2 = new AxleLoadSpeedProfile();
            asp2->NID_PACKET = asp->NID_PACKET;
            asp2->Q_DIR = asp->Q_DIR;
            asp2->Q_SCALE = asp->Q_SCALE;
            auto translate_element = [](V1::ASP_element_packet e) {
                ASP_element_packet e2;
                e2.D_AXLELOAD = e.D_AXLELOAD;
                e2.L_AXLELOAD = e.L_AXLELOAD;
                e2.Q_FRONT = e.Q_FRONT;
                e2.N_ITER = e.N_ITER;
                auto translate_diff = [](V1::ASP_diff d) {
                    ASP_diff d2;
                    float load = d.M_AXLELOAD.rawdata / 2.0f;
                    if (load <= 16) d2.M_AXLELOADCAT.rawdata = d2.M_AXLELOADCAT.A;
                    else if (load <= 17) d2.M_AXLELOADCAT.rawdata = d2.M_AXLELOADCAT.HS17;
                    else if (load <= 18) d2.M_AXLELOADCAT.rawdata = d2.M_AXLELOADCAT.B1;
                    else if (load <= 20) d2.M_AXLELOADCAT.rawdata = d2.M_AXLELOADCAT.C2;
                    else if (load <= 22.51) d2.M_AXLELOADCAT.rawdata = d2.M_AXLELOADCAT.D2;
                    else if (load <= 22.51) d2.M_AXLELOADCAT.rawdata = d2.M_AXLELOADCAT.E4;
                    d2.V_AXLELOAD = d.V_AXLELOAD;
                    return d2;
                };
                for (auto &d : e.diffs) {
                    e2.diffs.push_back(translate_diff(d));
                }
                return e2;
            };
            asp2->element = translate_element(asp->element);
            asp2->N_ITER = asp->N_ITER;
            for (auto &e : asp->elements) {
                asp2->elements.push_back(translate_element(e));
            }
            trans = std::shared_ptr<ETCS_packet>(asp2);
            break;
        }
        case 68: {
            for (auto &p2 : packets) {
                if (p2->NID_PACKET == 206) {
                    return nullptr;
                }
            }
            auto *tc = (TrackCondition*)packet.get();
            if (tc->element.M_TRACKCOND == 1 || tc->element.M_TRACKCOND == 2)
                tc->element.M_TRACKCOND.rawdata = 0;
            for (auto &e: tc->elements) {
                if (e.M_TRACKCOND == 1 || e.M_TRACKCOND == 2)
                    e.M_TRACKCOND.rawdata = 0;
            }
            break;
        }
        case 79: {
            auto *geo = (V1::GeographicalPosition*)packet.get();
            auto *geo2 = new GeographicalPosition();
            geo2->NID_PACKET = geo->NID_PACKET;
            geo2->Q_DIR = geo->Q_DIR;
            geo2->Q_SCALE = geo->Q_SCALE;
            auto translate_element = [](const V1::GeographicalPosition_element_packet &e) {
                GeographicalPosition_element_packet e2;
                e2.D_POSOFF = e.D_POSOFF;
                e2.NID_BG = e.NID_BG;
                e2.NID_C = e.NID_C;
                e2.Q_NEWCOUNTRY = e.Q_NEWCOUNTRY;
                e2.Q_MPOSITION = e.Q_MPOSITION;
                if (e.M_POSITION == e.M_POSITION.NoMoreCalculation)
                    e2.M_POSITION.rawdata = e2.M_POSITION.NoMoreCalculation;
                else
                    e2.M_POSITION.rawdata = e.M_POSITION;
                return e2;
            };
            geo2->element = translate_element(geo->element);
            geo2->N_ITER = geo->N_ITER;
            for (auto &e : geo->elements) {
                geo2->elements.push_back(translate_element(e));
            }
            trans = std::shared_ptr<ETCS_packet>(geo2);
            break;
        }
        case 80: {
            auto *mp = (V1::ModeProfile*)packet.get();
            auto *mp2 = new ModeProfile();
            mp2->NID_PACKET = mp->NID_PACKET;
            mp2->Q_DIR = mp->Q_DIR;
            mp2->Q_SCALE = mp->Q_SCALE;
            auto translate_element = [](const V1::MP_element_packet &e) {
                MP_element_packet e2;
                e2.D_MAMODE = e.D_MAMODE;
                e2.L_MAMODE = e.L_MAMODE;
                e2.M_MAMODE.rawdata = e.M_MAMODE.rawdata;
                e2.V_MAMODE = e.V_MAMODE;
                e2.L_ACKMAMODE = e.L_ACKMAMODE;
                e2.Q_MAMODE.rawdata = 1;
                return e2;
            };
            mp2->element = translate_element(mp->element);
            mp2->N_ITER = mp->N_ITER;
            for (auto &e : mp->elements) {
                mp2->elements.push_back(translate_element(e));
            }
            break;
        }
        case 200:
            packet->NID_PACKET.rawdata = 0;
            break;
        case 203:
            return nullptr;
        case 206:
            packet->NID_PACKET.rawdata = 68;
            break;
        case 207:
            packet->NID_PACKET.rawdata = 70;
            break;
        case 239:
            packet->NID_PACKET.rawdata = 39;
            break;
        default:
            break;
    }
    return trans;
}
std::shared_ptr<euroradio_message> translate_message(std::shared_ptr<euroradio_message> message, int version)
{
    if (VERSION_X(version)==1) {
        if (message->NID_MESSAGE == 40 || message->NID_MESSAGE == 41) {
            message->NID_LRBG.rawdata = message->NID_LRBG.Unknown;
        } else if (message->NID_MESSAGE == 15) {
            auto *emerg = (conditional_emergency_stop*)message.get();
            emerg->D_REF.rawdata = 0;
        } else if (message->NID_MESSAGE == 34) {
            auto *taf = (taf_request_message*)message.get();
            taf->D_REF.rawdata = 0;
        }
        auto packets = message->packets;
        message->packets.clear();
        for (auto &p : packets) {
            auto p2 = translate_packet(p, packets, version);
            if (p2 != nullptr)
                message->packets.push_back(p2);
        }
    }
    return message;
}
std::shared_ptr<euroradio_message_traintotrack> translate_message(std::shared_ptr<euroradio_message_traintotrack> message, int version)
{
    std::shared_ptr<euroradio_message_traintotrack> trans = message;
    if (VERSION_X(version) == 1) {
        switch (message->NID_MESSAGE.rawdata) {
            case 129: {
                auto *data = (validated_train_data_message*)message.get();
                auto *p = data->TrainData.get();
                auto *p2 = new V1::TrainDataPacket();
                p2->NID_PACKET = p->NID_PACKET;
                p2->NID_OPERATIONAL.rawdata = 0;
                int tmp = train_running_number;
                for (int i=0; i<8; i++) {
                    p2->NID_OPERATIONAL.rawdata |= (tmp % 10)<<(4*i);
                    tmp /= 10;
                }
                p2->NC_TRAIN.rawdata = (p->NC_TRAIN.rawdata&7)<<9;
                if (p->NC_CDTRAIN < 6)
                    p2->NC_TRAIN.rawdata |= 1<<(p->NC_CDTRAIN+1);
                else if (p->NC_CDTRAIN == 6)
                    p2->NC_TRAIN.rawdata |= 1<<13;
                else if (p->NC_CDTRAIN == 7)
                    p2->NC_TRAIN.rawdata |= 1<<7;
                else if (p->NC_CDTRAIN == 8)
                    p2->NC_TRAIN.rawdata |= 1<<12;
                else if (p->NC_CDTRAIN == 9)
                    p2->NC_TRAIN.rawdata |= 1;
                else if (p->NC_CDTRAIN == 10)
                    p2->NC_TRAIN.rawdata |= 1<<8;
                p2->L_TRAIN = p->L_TRAIN;
                p2->V_MAXTRAIN = p->V_MAXTRAIN;
                p2->M_LOADINGGAUGE.rawdata = 0;
                float load = 0;
                if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.A) load = 16;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.HS17) load = 17;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.B1) load = 18;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.B2) load = 18;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.C2) load = 20;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.C3) load = 20;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.C4) load = 20;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.D2) load = 22.5f;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.D3) load = 22.5f;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.D4) load = 22.5f;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.D4XL) load = 22.5f;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.E4) load = 25;
                else if (p->M_AXLELOADCAT.rawdata == p->M_AXLELOADCAT.E5) load = 25;
                p2->M_AXLELOAD.rawdata = (int)(load*2);
                p2->M_AIRTIGHT = p->M_AIRTIGHT;
                p2->N_ITERtraction.rawdata = p2->M_TRACTIONs.size();
                p2->N_ITERntc = p->N_ITERntc;
                p2->NID_NTCs = p->NID_NTCs;
                data->TrainData = std::shared_ptr<TrainDataPacket>(p2);
                break;
            }
            case 132: {
                auto *ma = (MA_request*)message.get();
                auto *ma2 = new V1::MA_request();
                ma2->NID_MESSAGE = ma->NID_MESSAGE;
                ma2->Q_TRACKDEL.rawdata = (ma->Q_MARQSTREASON.rawdata>>ma->Q_MARQSTREASON.TrackDescriptionDeletedBit)&1;
                trans = std::shared_ptr<euroradio_message_traintotrack>(ma2);
                break;
            }
            case 147: {
                auto *emerg = (emergency_acknowledgement_message*)message.get();
                if (emerg->Q_EMERGENCYSTOP == 1)
                    emerg->Q_EMERGENCYSTOP.rawdata = 0;
                else if (emerg->Q_EMERGENCYSTOP == 3)
                    emerg->Q_EMERGENCYSTOP.rawdata = 1;
                break;
            }
            case 159: {
                auto *msg2 = new V1::communication_session_established();
                msg2->NID_MESSAGE = message->NID_MESSAGE;
                msg2->PhoneNumbers->N_ITER.rawdata = 0;
                trans = std::shared_ptr<euroradio_message_traintotrack>(msg2);
                break;
            }
        }
    } else if (version == 32) {
        if (message->NID_MESSAGE == 159) {
            auto *msg2 = new V1::communication_session_established();
            msg2->NID_MESSAGE = message->NID_MESSAGE;
            msg2->PhoneNumbers->N_ITER.rawdata = 0;
            trans = std::shared_ptr<euroradio_message_traintotrack>(msg2);
        }
    }
    trans->PositionReport1BG = message->PositionReport1BG;
    trans->PositionReport2BG = message->PositionReport2BG;
    if (VERSION_X(version) == 1) {
        if (trans->PositionReport1BG) {
            auto &mode = message->PositionReport1BG->get()->M_MODE;
            if (mode == 15)
                mode.rawdata = 3;
        }
        if (trans->PositionReport2BG) {
            auto &mode = trans->PositionReport2BG->get()->M_MODE;
            if (mode == 15)
                mode.rawdata = 3;
        }
    }
    trans->NID_ENGINE = message->NID_ENGINE;
    trans->T_TRAIN = message->T_TRAIN;
    auto packs = message->optional_packets;
    trans->optional_packets.clear();
    for (auto &p : packs) {
        if (VERSION_X(version) == 1) {
            if (p->NID_PACKET == 4) {
                auto *err = (ErrorReporting*)p.get();
                if (err->M_ERROR == 6)
                    err->M_ERROR.rawdata = 7;
                else if (err->M_ERROR == 7 || err->M_ERROR == 8)
                    continue;
            }
        }
        trans->optional_packets.push_back(p);
    }
    return trans;
}