/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <stdint.h>
#include <limits>
#include <set>
#include "../Supervision/common.h"
#include "../Time/clock.h"
#include "../Version/version.h"
#include "types.h"
template<typename T>
class ETCS_variable_custom
{
    public:
    int size;
    T rawdata;
    std::set<T> invalid;
    ETCS_variable_custom(int size) : size(size), rawdata(T(0)) {}
    operator T() const
    {
        return rawdata;
    }
    virtual bool is_valid(int m_version)
    {
        return invalid.find(rawdata)==invalid.end();
    }
    void copy(bit_manipulator &b)
    {
        if (b.write_mode)
            b.write(this);
        else
            b.read(this);
    }
    virtual ~ETCS_variable_custom(){}
};
typedef ETCS_variable_custom<uint32_t> ETCS_variable;
struct bg_id
{
    int NID_C;
    int NID_BG;
    static const int Unknown = 16383;
    bool operator==(const bg_id &o) const
    {
        return NID_C == o.NID_C && NID_BG == o.NID_BG;
    }
    bool operator!=(const bg_id &o) const
    {
        return NID_C != o.NID_C || NID_BG != o.NID_BG;
    }
    bool operator<(const bg_id &o) const
    {
        if (NID_C == o.NID_C) return NID_BG < o.NID_BG;
        return NID_C < o.NID_C;
    }
};
struct Q_SCALE_t : ETCS_variable
{
    uint32_t cm10 = 0;
    uint32_t m1 = 1;
    uint32_t m10 = 2;
    Q_SCALE_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
    Q_SCALE_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct A_t : ETCS_variable
{
    A_t() : ETCS_variable(6) {}
    virtual double get_value() const
    {
        return rawdata * 0.05;
    }
};
struct A_NVMAXREDADH1_t : A_t
{
    uint32_t NoMaximumDisplayTargetInformation=61;
    uint32_t NoMaximumDisplayTTI=62;
    uint32_t NoMaximumNoDisplay=63;
    double get_value() const override
    {
        if (rawdata == NoMaximumDisplayTargetInformation) return -1;
        if (rawdata == NoMaximumDisplayTTI) return -2;
        if (rawdata == NoMaximumNoDisplay) return -3;
        return rawdata * 0.05;
    }
};
struct A_NVMAXREDADH2_t : A_t
{
    uint32_t NoMaximumDisplayTargetInformation=61;
    uint32_t NoMaximumDisplayTTI=62;
    uint32_t NoMaximumNoDisplay=63;
    double get_value() const override
    {
        if (rawdata == NoMaximumDisplayTargetInformation) return -1;
        if (rawdata == NoMaximumDisplayTTI) return -2;
        if (rawdata == NoMaximumNoDisplay) return -3;
        return rawdata * 0.05;
    }
};
struct A_NVMAXREDADH3_t : A_t
{
    uint32_t NoMaximumDisplayTargetInformation=61;
    uint32_t NoMaximumDisplayTTI=62;
    uint32_t NoMaximumNoDisplay=63;
    double get_value() const override
    {
        if (rawdata == NoMaximumDisplayTargetInformation) return -1;
        if (rawdata == NoMaximumDisplayTTI) return -2;
        if (rawdata == NoMaximumNoDisplay) return -3;
        return rawdata * 0.05;
    }
};
struct A_NVP12_t : A_t
{
};
struct A_NVP23_t : A_t
{
};
struct D_t : ETCS_variable
{
    D_t() : ETCS_variable(15) {}
    virtual double get_value(const Q_SCALE_t scale) const
    {
        double fact=1;
        if (scale == scale.m10)
            fact = 10;
        else if (scale == scale.cm10)
            fact = 0.1;
        return fact*rawdata;
    }
};
struct D_ADHESION_t : D_t
{
};
struct D_AXLELOAD_t : D_t
{
};
struct D_CURRENT_t : D_t
{
};
struct D_CYCLOC_t : D_t
{
    uint32_t NoCyclicalReportPosition=32767;
};
struct D_DP_t : D_t
{
};
struct D_EMERGENCYSTOP_t : D_t
{
};
struct D_ENDTIMERSTARTLOC_t : D_t
{
};
struct D_GRADIENT_t : D_t
{
};
struct D_INFILL_t : D_t
{
};
struct D_LEVELTR_t : D_t
{
    uint32_t Now=32767;
};
struct D_LINK_t : D_t
{
};
struct D_LOC_t : D_t
{
};
struct D_LRBG_t : D_t
{
    uint32_t Unknown=32767;
    void set_value(double val, Q_SCALE_t scale)
    {
        if (scale == scale.cm10) rawdata = val/10;
        else if (scale == scale.m1) rawdata = val;
        else rawdata = 10*val;
    }
    D_LRBG_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct D_LX_t : D_t
{
};
struct D_MAMODE_t : D_t
{
};
struct D_NVOVTRP_t : D_t
{
};
struct D_NVPOTRP_t : D_t
{
};
struct D_NVROLL_t : D_t
{
    uint32_t Infinity=32767;
    double get_value(Q_SCALE_t scale) const
    {
        if (rawdata == Infinity)
            return std::numeric_limits<double>::infinity();
        return D_t::get_value(scale);
    }
};
struct D_NVSTFF_t : D_t
{
    uint32_t Infinity=32767;
    double get_value(Q_SCALE_t scale) const
    {
        if (rawdata == Infinity)
            return std::numeric_limits<double>::infinity();
        return D_t::get_value(scale);
    }
};
struct D_OL_t : D_t
{
};
struct D_POSOFF_t : D_t
{
};
struct D_PBD_t : D_t
{
};
struct D_PBDSR_t : D_t
{
};
struct D_RBCTR_t : D_t
{
};
struct D_REF_t : ETCS_variable
{
    D_REF_t() : ETCS_variable(16){}
    double get_value(Q_SCALE_t scale)
    {
        double fact=1;
        if (scale == scale.m10)
            fact = 10;
        else if (scale == scale.cm10)
            fact = 0.1;
        if (rawdata>32767)
            return -fact*((rawdata ^ 65535)+1);
        else
            return rawdata*fact;
    }
};
struct D_REVERSE_t : D_t
{
    uint32_t Infinity=32767;
};
struct D_SECTIONTIMERSTOPLOC_t : D_t
{
};
struct D_SR_t : D_t
{
    uint32_t Infinity=32767;
};
struct D_STARTREVERSEAREA_t : D_t
{
};
struct D_STATIC_t : D_t
{
};
struct D_STARTOL_t : D_t
{
};
struct D_SUITABILITY_t : D_t
{
};
struct D_TAFDISPLAY_t : D_t
{
};
struct D_TEXTDISPLAY_t : D_t
{
    uint32_t NotDistanceLimited=32767;
};
struct D_TRACTION_t : D_t
{
};
struct D_TRACKCOND_t : D_t
{
};
struct D_TRACKINIT_t : D_t
{
};
struct D_TSR_t : D_t
{
};
struct D_VALIDNV_t : D_t
{
    uint32_t Now=32767;
};
struct G_t : ETCS_variable
{
    G_t() : ETCS_variable(8) {}
};
struct G_A_t : G_t
{
    uint32_t EndOfGradient=255;
};
struct G_TSR_t : G_t
{
};
struct G_PBDSR_t : G_t
{
};
struct L_NVKRINT_t : ETCS_variable
{
    L_NVKRINT_t() : ETCS_variable(5) {}
    double get_value() const
    {
        if (rawdata < 5)
            return 25*rawdata;
        else if (rawdata == 5)
            return 150;
        else
            return 100*(rawdata-4);
    }
};
struct L_ACKLEVELTR_t : D_t
{
};
struct L_ACKMAMODE_t : D_t
{
};
struct L_ADHESION_t : D_t
{
};
struct L_AXLELOAD_t : D_t
{
};
struct L_DOUBTOVER_t : D_t
{
    uint32_t Unknown=32767;
    L_DOUBTOVER_t &operator=(uint32_t data) {rawdata=data; return *this;}
    void set_value(double val, Q_SCALE_t scale)
    {
        if (scale == scale.cm10) rawdata = val/10;
        else if (scale == scale.m1) rawdata = val;
        else rawdata = 10*val;
    }
};
struct L_DOUBTUNDER_t : D_t
{
    uint32_t Unknown=32767;
    L_DOUBTUNDER_t &operator=(uint32_t data) {rawdata=data; return *this;}
    void set_value(double val, Q_SCALE_t scale)
    {
        if (scale == scale.cm10) rawdata = val/10;
        else if (scale == scale.m1) rawdata = val;
        else rawdata = 10*val;
    }
};
struct L_ENDSECTION_t : D_t
{
};
struct L_LX_t : D_t
{
};
struct L_MAMODE_t : D_t
{
    uint32_t Infinity=32767;
};
struct L_MESSAGE_t : ETCS_variable
{
    L_MESSAGE_t() : ETCS_variable(10) {}
};
struct L_PACKET_t : ETCS_variable
{
    L_PACKET_t() : ETCS_variable(13) {}
};
struct L_PBDSR_t : D_t
{
};
struct L_REVERSEAREA_t : D_t
{
};
struct L_SECTION_t : D_t
{
};
struct L_STOPLX_t : D_t
{
};
struct L_TAFDISPLAY_t : D_t
{
};
struct L_TEXT_t : ETCS_variable
{
    L_TEXT_t() : ETCS_variable(8) {}
};
struct L_TEXTDISPLAY_t : D_t
{
    uint32_t NotDistanceLimited=32767;
};
struct L_TRACKCOND_t : D_t
{
};
struct L_TRAIN_t : ETCS_variable
{
    L_TRAIN_t() : ETCS_variable(12) {}
    double get_value() const
    {
        return rawdata;
    }
    void set_value(double length)
    {
        rawdata = length;
    }
};
struct L_TRAININT_t : ETCS_variable
{
    L_TRAININT_t() : ETCS_variable(15) {}
    double get_value() const
    {
        return rawdata;
    }
};
struct L_TSR_t : D_t
{
};
struct M_ACK_t : ETCS_variable
{
    uint32_t NoAcknowledgement=0;
    uint32_t AcknowledgementRequired=1;
    M_ACK_t() : ETCS_variable(1) {}
};
struct M_ADHESION_t : ETCS_variable
{
    uint32_t SlipperyRail=0;
    uint32_t NonSlipperyRail=1;
    M_ADHESION_t() : ETCS_variable(1) {}
};
struct M_AIRTIGHT_t : ETCS_variable
{
    uint32_t NotFitted=0;
    uint32_t Fitted=1;
    M_AIRTIGHT_t() : ETCS_variable(2)
    {
        invalid.insert(2);
        invalid.insert(3);
    }
};
struct M_AXLELOADCAT_t : ETCS_variable
{
    uint32_t A=0;
    uint32_t HS17=1;
    uint32_t B1=2;
    uint32_t B2=3;
    uint32_t C2=4;
    uint32_t C3=5;
    uint32_t C4=6;
    uint32_t D2=7;
    uint32_t D3=8;
    uint32_t D4=9;
    uint32_t D4XL=10;
    uint32_t E4=11;
    uint32_t E5=12;
    M_AXLELOADCAT_t() : ETCS_variable(7) {}
    bool is_valid(int m_version) override
    {
        return rawdata < 13;
    }
};
struct M_CURRENT_t : ETCS_variable
{
    uint32_t NoRestriction=1023;
    M_CURRENT_t() : ETCS_variable(10) {}
    int get_value()
    {
        return rawdata * 10;
    }
};
struct M_DUP_t : ETCS_variable
{
    uint32_t NoDuplicates=0;
    uint32_t DuplicateOfNext=1;
    uint32_t DuplicateOfPrev=2;
    M_DUP_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct M_ERROR_t : ETCS_variable
{
    uint32_t BaliseGroupLinkingConsistencyError=0;
    uint32_t LinkedMessageConsistencyError=1;
    uint32_t UnlinkedMessageConsistencyError=2;
    uint32_t RadioMessageConsistencyError=3;
    uint32_t RadioSequenceError=4;
    uint32_t SafeRadioConnectionError=5;
    uint32_t SafetyCriticalFault=6;
    uint32_t DoubleLinkingError=7;
    uint32_t DoubleRepositioningError=8;
    M_ERROR_t() : ETCS_variable(8) {}
    bool is_valid(int m_version) override
    {
        return rawdata < (VERSION_X(m_version) == 1 ? 8 : 9);
    }
};
#if BASELINE > 3
namespace V2
{
#endif
struct M_LEVEL_t : ETCS_variable
{
    uint32_t N0=0;
    uint32_t NTC=1;
    uint32_t N1=2;
    uint32_t N2=3;
    uint32_t N3=4;
    M_LEVEL_t() : ETCS_variable(3)
    {
        invalid.insert(5);
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_level()
    {
        if (rawdata == N0) return Level::N0;
        else if (rawdata == N1) return Level::N1;
        else if (rawdata == N2) return Level::N2;
#if BASELINE < 4
        else if (rawdata == N3) return Level::N3;
#else
        else if (rawdata == N3) return Level::N2;
#endif
        else if (rawdata == NTC) return Level::NTC;
        else return Level::Unknown;
    }
    void set_value(Level lv)
    {
        if (lv == Level::N0) rawdata = N0;
        else if (lv == Level::NTC) rawdata = NTC;
        else if (lv == Level::N1) rawdata = N1;
        else if (lv == Level::N2) rawdata = N2;
#if BASELINE < 4
        else if (lv == Level::N3) rawdata = N3;
#endif
    }
};
struct M_LEVELTEXTDISPLAY_t : ETCS_variable
{
    uint32_t N0=0;
    uint32_t NTC=1;
    uint32_t N1=2;
    uint32_t N2=3;
    uint32_t N3=4;
    uint32_t NoLevelLimited=5;
    M_LEVELTEXTDISPLAY_t() : ETCS_variable(3)
    {
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_value() const
    {
        if (rawdata == N0) return Level::N0;
        else if (rawdata == N1) return Level::N1;
        else if (rawdata == N2) return Level::N2;
#if BASELINE < 4
        else if (rawdata == N3) return Level::N3;
#else
        else if (rawdata == N3) return Level::N2;
#endif
        else if (rawdata == NTC) return Level::NTC;
        else return Level::Unknown;
    }
};
struct M_LEVELTR_t : ETCS_variable
{
    uint32_t N0=0;
    uint32_t NTC=1;
    uint32_t N1=2;
    uint32_t N2=3;
    uint32_t N3=4;
    M_LEVELTR_t() : ETCS_variable(3)
    {
        invalid.insert(5);
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_level()
    {

        if (rawdata == N0) return Level::N0;
        else if (rawdata == N1) return Level::N1;
        else if (rawdata == N2) return Level::N2;
#if BASELINE < 4
        else if (rawdata == N3) return Level::N3;
#else
        else if (rawdata == N3) return Level::N2;
#endif
        else if (rawdata == NTC) return Level::NTC;
        else return Level::Unknown;
    }
};
#if BASELINE > 3
}
struct M_LEVEL_t : ETCS_variable
{
    uint32_t N0=0;
    uint32_t NTC=1;
    uint32_t N1=2;
    uint32_t N2=3;
    M_LEVEL_t() : ETCS_variable(3)
    {
        invalid.insert(4);
        invalid.insert(5);
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_level()
    {
        if (rawdata == N0) return Level::N0;
        else if (rawdata == N1) return Level::N1;
        else if (rawdata == N2) return Level::N2;
        else if (rawdata == NTC) return Level::NTC;
        else return Level::Unknown;
    }
    void set_value(Level lv)
    {
        if (lv == Level::N0) rawdata = N0;
        else if (lv == Level::NTC) rawdata = NTC;
        else if (lv == Level::N1) rawdata = N1;
        else if (lv == Level::N2) rawdata = N2;
    }
};
struct M_LEVELTEXTDISPLAY_t : ETCS_variable
{
    uint32_t N0=0;
    uint32_t NTC=1;
    uint32_t N1=2;
    uint32_t N2=3;
    uint32_t NoLevelLimited=4;
    M_LEVELTEXTDISPLAY_t() : ETCS_variable(3)
    {
        invalid.insert(5);
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_level()
    {
        if (rawdata == N0) return Level::N0;
        else if (rawdata == N1) return Level::N1;
        else if (rawdata == N2) return Level::N2;
        else if (rawdata == NTC) return Level::NTC;
        else return Level::Unknown;
    }
};
struct M_LEVELTR_t : ETCS_variable
{
    M_LEVELTR_t() : ETCS_variable(3)
    {
        invalid.insert(4);
        invalid.insert(5);
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_level()
    {
        if (rawdata == N0) return Level::N0;
        else if (rawdata == N1) return Level::N1;
        else if (rawdata == N2) return Level::N2;
        else if (rawdata == NTC) return Level::NTC;
        else return Level::Unknown;
    }
};
#endif
struct M_LINEGAUGE_t : ETCS_variable
{
    uint32_t BitG1 = 1;
    uint32_t BitGA = 2;
    uint32_t BitGB = 4;
    uint32_t BitGC = 8;
    M_LINEGAUGE_t() : ETCS_variable(8) {}
    bool is_valid(int m_version) override
    {
        return rawdata!=0 && (rawdata&0xF0)==0;
    }
};
struct M_LOADINGGAUGE_t : ETCS_variable
{
    uint32_t G1 = 0;
    uint32_t GA = 1;
    uint32_t GB = 2;
    uint32_t GC = 3;
    M_LOADINGGAUGE_t() : ETCS_variable(8) {}
    bool is_valid(int m_version) override
    {
        return VERSION_X(m_version) == 1 || rawdata<4;
    }
};
struct M_LOC_t : ETCS_variable
{
    uint32_t Now = 0;
    uint32_t EveryLRBG = 1;
    uint32_t NotEveryLRBG = 2;
    M_LOC_t() : ETCS_variable(3) {}
    bool is_valid(int m_version) override
    {
        return rawdata<3;
    }
};
struct M_MAMODE_t : ETCS_variable
{
    uint32_t OS=0;
    uint32_t SH=1;
    uint32_t LS=2;
    M_MAMODE_t() : ETCS_variable(2) {}
    bool is_valid(int m_version) override
    {
        return rawdata<(VERSION_X(m_version) == 1 ? 2 : 3);
    }
};
struct M_MCOUNT_t : ETCS_variable
{
    uint32_t NeverFitsTelegrams=254;
    uint32_t FitsAllTelegrams=255;
    M_MCOUNT_t() : ETCS_variable(8) {}
};
#if BASELINE > 3
namespace V2
{
#endif
struct M_MODE_t : ETCS_variable
{
    uint32_t FS=0;
    uint32_t OS=1;
    uint32_t SR=2;
    uint32_t SH=3;
    uint32_t UN=4;
    uint32_t SL=5;
    uint32_t SB = 6;
    uint32_t TR = 7;
    uint32_t PT = 8;
    uint32_t SF = 9;
    uint32_t IS = 10;
    uint32_t NL = 11;
    uint32_t LS = 12;
    uint32_t SN = 13;
    uint32_t RV = 14;
    uint32_t PS = 15;
    M_MODE_t() : ETCS_variable(4) {}
    void set_value(Mode m)
    {
        switch (m)
        {
            case Mode::FS:
                rawdata = FS;
                break;
            case Mode::OS:
                rawdata = OS;
                break;
            case Mode::SR:
                rawdata = SR;
                break;
            case Mode::SH:
                rawdata = SH;
                break;
            case Mode::UN:
                rawdata = UN;
                break;
            case Mode::SL:
                rawdata = SL;
                break;
            case Mode::SB:
                rawdata = SB;
                break;
            case Mode::TR:
                rawdata = TR;
                break;
            case Mode::PT:
                rawdata = PT;
                break;
            case Mode::SF:
                rawdata = SF;
                break;
            case Mode::IS:
                rawdata = IS;
                break;
            case Mode::NL:
                rawdata = NL;
                break;
            case Mode::LS:
                rawdata = LS;
                break;
            case Mode::SN:
                rawdata = SN;
                break;
            case Mode::RV:
                rawdata = RV;
                break;
            case Mode::PS:
                rawdata = PS;
                break;
            default:
                rawdata = SF;
                break;
        } 
    }
    Mode get_value() const
    {
        if (rawdata == FS) return Mode::FS;
        else if (rawdata == OS) return Mode::OS;
        else if (rawdata == SR) return Mode::SR;
        else if (rawdata == SH) return Mode::SH;
        else if (rawdata == UN) return Mode::UN;
        else if (rawdata == SL) return Mode::SL;
        else if (rawdata == SB) return Mode::SB;
        else if (rawdata == TR) return Mode::TR;
        else if (rawdata == PT) return Mode::PT;
        else if (rawdata == SF) return Mode::SF;
        else if (rawdata == IS) return Mode::IS;
        else if (rawdata == NL) return Mode::NL;
        else if (rawdata == LS) return Mode::LS;
        else if (rawdata == SN) return Mode::SN;
        else if (rawdata == RV) return Mode::RV;
        else if (rawdata == PS) return Mode::PS;
        else return Mode::SF;
    }
    
    bool is_valid(int m_version) override
    {
        return VERSION_X(m_version) > 1 || rawdata != 15;
    }
};
struct M_MODETEXTDISPLAY_t : ETCS_variable
{
    uint32_t FS=0;
    uint32_t OS=1;
    uint32_t SR=2;
    uint32_t UN=4;
    uint32_t SB = 6;
    uint32_t TR = 7;
    uint32_t PT = 8;
    uint32_t LS = 12;
    uint32_t RV = 14;
    uint32_t NoModeLimited = 15;
    M_MODETEXTDISPLAY_t() : ETCS_variable(4)
    {
        invalid.insert(3);
        invalid.insert(5);
        invalid.insert(9);
        invalid.insert(10);
        invalid.insert(11);
        invalid.insert(13);
    }
    Mode get_value() const
    {
        if (rawdata == FS) return Mode::FS;
        else if (rawdata == OS) return Mode::OS;
        else if (rawdata == SR) return Mode::SR;
        else if (rawdata == UN) return Mode::UN;
        else if (rawdata == SB) return Mode::SB;
        else if (rawdata == PT) return Mode::PT;
        else if (rawdata == LS) return Mode::LS;
        else if (rawdata == RV) return Mode::RV;
        else return Mode::TR;
    }
    bool is_valid(int m_version) override
    {
        if (VERSION_X(m_version) == 1 && rawdata == 12) return false;
        return ETCS_variable::is_valid(m_version);
    }
};
#if BASELINE > 3
}
struct M_MODE_t : ETCS_variable
{
    uint32_t FS=0;
    uint32_t OS=1;
    uint32_t SR=2;
    uint32_t SH=3;
    uint32_t UN=4;
    uint32_t SL=5;
    uint32_t SB = 6;
    uint32_t TR = 7;
    uint32_t PT = 8;
    uint32_t SF = 9;
    uint32_t IS = 10;
    uint32_t NL = 11;
    uint32_t LS = 12;
    uint32_t SN = 13;
    uint32_t RV = 14;
    uint32_t PS = 15;
    uint32_t AD = 16;
    uint32_t SM = 17;
    M_MODE_t() : ETCS_variable(5) {}
    void set_value(Mode m)
    {
        switch (m)
        {
            case Mode::FS:
                rawdata = FS;
                break;
            case Mode::OS:
                rawdata = OS;
                break;
            case Mode::SR:
                rawdata = SR;
                break;
            case Mode::SH:
                rawdata = SH;
                break;
            case Mode::UN:
                rawdata = UN;
                break;
            case Mode::SL:
                rawdata = SL;
                break;
            case Mode::SB:
                rawdata = SB;
                break;
            case Mode::TR:
                rawdata = TR;
                break;
            case Mode::PT:
                rawdata = PT;
                break;
            case Mode::SF:
                rawdata = SF;
                break;
            case Mode::IS:
                rawdata = IS;
                break;
            case Mode::NL:
                rawdata = NL;
                break;
            case Mode::LS:
                rawdata = LS;
                break;
            case Mode::SN:
                rawdata = SN;
                break;
            case Mode::RV:
                rawdata = RV;
                break;
            case Mode::PS:
                rawdata = PS;
                break;
            case Mode::AD:
                rawdata = AD;
                break;
            case Mode::SM:
                rawdata = SM;
                break;
            default:
                rawdata = SF;
                break;
        } 
    }
    Mode get_value() const
    {
        switch (rawdata)
        {
            case FS:
                return Mode::FS;
            case OS:
                return Mode::OS;
            case SR:
                return Mode::SR;
            case SH:
                return Mode::SH;
            case UN:
                return Mode::UN;
            case SL:
                return Mode::SL;
            case SB:
                return Mode::SB;
            case TR:
                return Mode::TR;
            case PT:
                return Mode::PT;
            case SF:
                return Mode::SF;
            case IS:
                return Mode::IS;
            case NL:
                return Mode::NL;
            case LS:
                return Mode::LS;
            case SN:
                return Mode::SN;
            case RV:
                return Mode::RV;
            case PS:
                return Mode::PS;
            case AD:
                return Mode::AD;
            case SM:
                return Mode::SM;
            default:
                return Mode::SF;
        }
    }
};
struct M_MODETEXTDISPLAY_t : ETCS_variable
{
    uint32_t FS=0;
    uint32_t OS=1;
    uint32_t SR=2;
    uint32_t AD=3;
    uint32_t UN=4;
    uint32_t SM=5;
    uint32_t SB = 6;
    uint32_t TR = 7;
    uint32_t PT = 8;
    uint32_t LS = 12;
    uint32_t RV = 14;
    uint32_t NoModeLimited = 15;
    M_MODETEXTDISPLAY_t() : ETCS_variable(4) 
    {
        invalid.insert(9);
        invalid.insert(10);
        invalid.insert(11);
        invalid.insert(13);
    }
    Mode get_value() const
    {
        switch (rawdata)
        {
            case FS:
                return Mode::FS;
            case OS:
                return Mode::OS;
            case SR:
                return Mode::SR;
            case AD:
                return Mode::AD;
            case UN:
                return Mode::UN;
            case SM:
                return Mode::SM;
            case SB:
                return Mode::SB;
            case PT:
                return Mode::PT;
            case LS:
                return Mode::LS;
            case RV:
                return Mode::RV;
            default:
                return Mode::TR;
        }
    }
};
#endif
struct M_NVAVADH_t : ETCS_variable
{
    M_NVAVADH_t() : ETCS_variable(5) {}
    double get_value() const
    {
        return rawdata*0.05;
    }
    bool is_valid(int m_version) override
    {
        return get_value()<1.05;
    }
};
struct M_NVCONTACT_t : ETCS_variable
{
    uint32_t TrainTrip=0;
    uint32_t ServiceBrake=1;
    uint32_t NoReaction=2;
    M_NVCONTACT_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
};
struct M_NVDERUN_t : ETCS_variable
{
    uint32_t No=0;
    uint32_t Yes=1;
    M_NVDERUN_t() : ETCS_variable(1) {}
};
struct M_NVEBCL_t : ETCS_variable
{
    M_NVEBCL_t() : ETCS_variable(4) {}
    double get_value()
    {
        switch(rawdata)
        {
            case 0:
                return 0.5;
            case 1:
                return 0.9;
            case 2:
                return 0.99;
            case 3:
                return 0.999;
            case 4:
                return 0.9999;
            case 5:
                return 0.99999;
            case 6:
                return 0.999999;
            case 7:
                return 0.9999999;
            case 8:
                return 0.99999999;
            case 9:
                return 0.999999999;
        }
        return 0;
    }
    bool is_valid(int m_version) override
    {
        return rawdata < 10;
    }
};
struct M_NVKRINT_t : ETCS_variable
{
    M_NVKRINT_t() : ETCS_variable(5) {}
    double get_value()
    {
        return rawdata * 0.05;
    }
};
struct M_NVKTINT_t : ETCS_variable
{
    M_NVKTINT_t() : ETCS_variable(5) {}
    double get_value()
    {
        return rawdata * 0.05;
    }
};
struct M_NVKVINT_t : ETCS_variable
{
    M_NVKVINT_t() : ETCS_variable(7) {}
    double get_value()
    {
        return rawdata * 0.02;
    }
};
struct M_PLATFORM_t : ETCS_variable
{
    M_PLATFORM_t() : ETCS_variable(4)
    {
        invalid.insert(14);
        invalid.insert(15);
    }
    double get_value()
    {
        switch(rawdata) {
            case 0:
                return 0.2;
            case 1:
                return 0.3;
            case 2:
                return 0.55;
            case 3:
                return 0.58;
            case 4:
                return 0.68;
            case 5:
                return 0.685;
            case 6:
                return 0.73;
            case 7:
                return 0.76;
            case 8:
                return 0.84;
            case 9:
                return 0.9;
            case 10:
                return 0.915;
            case 11:
                return 0.92;
            case 12:
                return 0.96;
            case 13:
                return 1.1;
        }
        return 0;
    }
};
struct M_POSITION_t : ETCS_variable
{
    uint32_t NoMoreCalculation=16777215UL;
    M_POSITION_t() : ETCS_variable(24) {}
    bool is_valid(int m_version) override
    {
        return VERSION_X(m_version) == 1 || rawdata<10000000UL || rawdata == 16777215UL;
    }
};
struct M_TRACKCOND_t : ETCS_variable
{
    uint32_t NonStoppingArea=0;
    uint32_t TunnelStoppingArea=1;
    uint32_t SoundHorn=2;
    uint32_t PowerlessLowerPantograph=3;
    uint32_t RadioHole=4;
    uint32_t AirTightness=5;
    uint32_t SwitchOffRegenerative=6;
    uint32_t SwitchOffEddyService=7;
    uint32_t SwitchOffShoe=8;
    uint32_t PowerlessSwitchOffPower=9;
    uint32_t SwitchOffEddyEmergency=10;
    M_TRACKCOND_t() : ETCS_variable(4) {}
    bool is_valid(int m_version) override
    {
        return rawdata < (VERSION_X(m_version) == 1 ? 10 : 11);
    }
};
struct M_VERSION_t : ETCS_variable
{
    uint32_t V1_0=16;
    uint32_t V1_1=17;
    uint32_t V2_0=32;
    uint32_t V2_1=33;
    M_VERSION_t() : ETCS_variable(7) {}
    bool is_valid(int m_version) override
    {
        return rawdata<=17 || rawdata>31;
    }
};
struct M_VOLTAGE_t : ETCS_variable
{
    uint32_t NonFitted=0;
    uint32_t AC25kV50Hz=1;
    uint32_t AC15kV16Hz7=2;
    uint32_t DC3kV=3;
    uint32_t DC1k5V=4;
    uint32_t DC600V=5;
    M_VOLTAGE_t() : ETCS_variable(4) {}
    bool is_valid(int m_version) override
    {
        return rawdata<6;
    }
};
struct NC_CDDIFF_t : ETCS_variable
{
    NC_CDDIFF_t() : ETCS_variable(4) {}
    bool is_valid(int m_version) override
    {
        return rawdata<11;
    }
    void set_value(int def)
    {
        if (def >= 300)
            rawdata = 10;
        else if (def >= 275)
            rawdata = 9;
        else if (def >= 245)
            rawdata = 8;
        else if (def >= 225)
            rawdata = 7;
        else if (def >= 210)
            rawdata = 6;
        else if (def >= 180)
            rawdata = 5;
        else if (def >= 165)
            rawdata = 4;
        else if (def >= 150)
            rawdata = 3;
        else if (def >= 130)
            rawdata = 2;
        else if (def >= 100)
            rawdata = 1;
        else
            rawdata = 0;
    }
    int get_value()
    {
        switch (rawdata)
        {
            case 0:
            default:
                return 80;
            case 1:
                return 100;
            case 2:
                return 130;
            case 3:
                return 150;
            case 4:
                return 165;
            case 5:
                return 180;
            case 6:
                return 210;
            case 7:
                return 225;
            case 8:
                return 245;
            case 9:
                return 275;
            case 10:
                return 300;
        }
    }
};
struct NC_CDTRAIN_t : ETCS_variable
{
    NC_CDTRAIN_t() : ETCS_variable(4) {}
    bool is_valid(int m_version) override
    {
        return rawdata<11;
    }
    void set_value(int def)
    {
        if (def >= 300)
            rawdata = 10;
        else if (def >= 275)
            rawdata = 9;
        else if (def >= 245)
            rawdata = 8;
        else if (def >= 225)
            rawdata = 7;
        else if (def >= 210)
            rawdata = 6;
        else if (def >= 180)
            rawdata = 5;
        else if (def >= 165)
            rawdata = 4;
        else if (def >= 150)
            rawdata = 3;
        else if (def >= 130)
            rawdata = 2;
        else if (def >= 100)
            rawdata = 1;
        else
            rawdata = 0;
    }
    int get_value()
    {
        switch (rawdata)
        {
            case 0:
            default:
                return 80;
            case 1:
                return 100;
            case 2:
                return 130;
            case 3:
                return 150;
            case 4:
                return 165;
            case 5:
                return 180;
            case 6:
                return 210;
            case 7:
                return 225;
            case 8:
                return 245;
            case 9:
                return 275;
            case 10:
                return 300;
        }
    }
};
struct NC_DIFF_t : ETCS_variable
{
    uint32_t FreightP=0;
    uint32_t FreightG=1;
    uint32_t Passenger=2;
    NC_DIFF_t() : ETCS_variable(4) {}
    bool is_valid(int m_version) override
    {
        return rawdata<3;
    }
};
struct NC_TRAIN_t : ETCS_variable
{
    uint32_t FreightPBit=0;
    uint32_t FreightGBit=1;
    uint32_t PassengerBit=2;
    NC_TRAIN_t() : ETCS_variable(15) {}
    bool is_valid(int m_version) override
    {
        return rawdata<(VERSION_X(m_version) ? 16384 : 8);
    }
};
struct NID_BG_t : ETCS_variable
{
    uint32_t Unknown=16383;
    NID_BG_t() : ETCS_variable(14) {}
};
struct NID_C_t : ETCS_variable
{
    NID_C_t() : ETCS_variable(10) {}
};
struct NID_CTRACTION_t : ETCS_variable
{
    NID_CTRACTION_t() : ETCS_variable(10) {}
};
struct NID_EM_t : ETCS_variable
{
    NID_EM_t() : ETCS_variable(4) {}
};
struct NID_ENGINE_t : ETCS_variable
{
    NID_ENGINE_t() : ETCS_variable(24) {}
};
struct NID_LRBG_t : ETCS_variable
{
    uint32_t Unknown=16777215;
    NID_LRBG_t() : ETCS_variable(24) {}
    bg_id get_value()
    {
        if (rawdata == Unknown) return {-1, -1};
        return {(int)(rawdata>>14), (int)(rawdata&16383)};
    }
    void set_value(bg_id id)
    {
        if (id.NID_BG < 0)
            rawdata = Unknown;
        else
            rawdata = (id.NID_C<<14) | id.NID_BG;
    }
    NID_LRBG_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct NID_PRVLRBG_t : ETCS_variable
{
    uint32_t Unknown=16777215;
    NID_PRVLRBG_t() : ETCS_variable(24) {}
    bg_id get_value()
    {
        if (rawdata == Unknown) return {-1, -1};
        return {(int)(rawdata>>14), (int)(rawdata&16383)};
    }
    void set_value(bg_id id)
    {
        if (id.NID_BG < 0)
            rawdata = Unknown;
        else
            rawdata = (id.NID_C<<14) | id.NID_BG;
    }
    NID_PRVLRBG_t &operator=(uint32_t data) {rawdata=data; return *this;}
};

struct NID_LTRBG_t : ETCS_variable
{
    NID_LTRBG_t() : ETCS_variable(24) {}
    bg_id get_value()
    {
        return {(int)(rawdata>>14), (int)(rawdata&16383)};
    }
    void set_value(bg_id id)
    {
        rawdata = (id.NID_C<<14) | id.NID_BG;
    }
};
struct NID_LX_t : ETCS_variable
{
    NID_LX_t() : ETCS_variable(8) {}
};
struct NID_MESSAGE_t : ETCS_variable
{
    NID_MESSAGE_t() : ETCS_variable(8) {}
};
struct NID_MN_t : ETCS_variable
{
    NID_MN_t() : ETCS_variable(24) {}
    uint32_t get_value()
    {
        uint32_t value=0;
        for (int i=5; i>=0; i--)
        {
            int c = (rawdata>>(4*i))&15;
            if (c == 15)
                continue;
            value = 10*value + c;
        }
        return value;
    }
};
struct NID_NTC_t : ETCS_variable
{
    NID_NTC_t() : ETCS_variable(8) {}
};
struct NID_RADIO_t : ETCS_variable_custom<uint64_t>
{   
    uint64_t UseShortNumber=std::numeric_limits<uint64_t>::max();
    NID_RADIO_t() : ETCS_variable_custom<uint64_t>(64) {}
};
struct NID_RBC_t : ETCS_variable
{
    uint32_t ContactLastRBC=16383;
    NID_RBC_t() : ETCS_variable(14) {}
};
struct NID_RIU_t : ETCS_variable
{
    NID_RIU_t() : ETCS_variable(14) {}
};
struct NID_TEXTMESSAGE_t : ETCS_variable
{
    NID_TEXTMESSAGE_t() : ETCS_variable(8) {}
};
struct NID_OPERATIONAL_t : ETCS_variable
{
    NID_OPERATIONAL_t() : ETCS_variable(32) {}
    unsigned int get_value()
    {
        unsigned int value=0;
        for (int i=7; i>=0; i--)
        {
            int c = (rawdata>>(4*i))&15;
            if (c == 15)
                continue;
            value = 10*value + c;
        }
        return value;
    }
    bool is_valid(int m_version) override
    {
        if (rawdata == 0xFFFFFFFF)
            return VERSION_X(m_version) == 1;
        for (int i=0; i<8; i++)
        {
            int c = (rawdata>>(4*i))&15;
            if (c > 9 && c < 15)
                return false;
        }
        return true;
    }
};
struct NID_PACKET_t : ETCS_variable
{
    NID_PACKET_t() : ETCS_variable(8) {}
    NID_PACKET_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct NID_TSR_t : ETCS_variable
{
    uint32_t NonRevocable=255;
    NID_TSR_t() : ETCS_variable(8) {}
};
struct NID_VBCMK_t : ETCS_variable
{
    NID_VBCMK_t() : ETCS_variable(6) {}
};
struct NID_XUSER_t : ETCS_variable
{
    NID_XUSER_t() : ETCS_variable(9) {}
};
struct N_AXLE_t : ETCS_variable
{
    uint32_t Unknown=1023;
    N_AXLE_t() : ETCS_variable(10) {}
};
struct N_ITER_t : ETCS_variable
{
    N_ITER_t() : ETCS_variable(5) {}
};
struct N_PIG_t : ETCS_variable
{
    N_PIG_t() : ETCS_variable(3) {}
};
struct N_TOTAL_t : ETCS_variable
{
    N_TOTAL_t() : ETCS_variable(3) {}
};
struct Q_ASPECT_t : ETCS_variable
{
    uint32_t StopIfInSH=0;
    uint32_t GoIfInSH=1;
    Q_ASPECT_t() : ETCS_variable(1) {}
};
struct Q_CONFTEXTDISPLAY_t : ETCS_variable
{
    uint32_t AcknowledgeEnds=0;
    uint32_t AcknowledgeRequired=1;
    Q_CONFTEXTDISPLAY_t() : ETCS_variable(1) {}
};
struct Q_DANGERPOINT_t : ETCS_variable
{
    uint32_t NoDangerpoint=0;
    uint32_t ExistsDangerpoint=1;
    Q_DANGERPOINT_t() : ETCS_variable(1) {}
};
struct Q_DIFF_t : ETCS_variable
{
    uint32_t CantDeficiency = 0;
    uint32_t OtherSpecificReplacesCant = 1;
    uint32_t OtherSpecificNotReplacesCant = 2;
    Q_DIFF_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct Q_DIR_t : ETCS_variable
{
    uint32_t Reverse = 0;
    uint32_t Nominal = 1;
    uint32_t Both = 2;
    Q_DIR_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
};
struct Q_DIRLRBG_t : ETCS_variable
{
    uint32_t Reverse = 0;
    uint32_t Nominal = 1;
    uint32_t Unknown = 2;
    Q_DIRLRBG_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
    void set_value(bool reverse)
    {
        rawdata = reverse ? Reverse : Nominal;
    }
    Q_DIRLRBG_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct Q_DIRTRAIN_t : ETCS_variable
{
    uint32_t Reverse = 0;
    uint32_t Nominal = 1;
    uint32_t Unknown = 2;
    Q_DIRTRAIN_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
    void set_value(bool reverse)
    {
        rawdata = reverse ? Reverse : Nominal;
    }
    Q_DIRTRAIN_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct Q_DLRBG_t : ETCS_variable
{
    uint32_t Reverse = 0;
    uint32_t Nominal = 1;
    uint32_t Unknown = 2;
    Q_DLRBG_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
    void set_value(bool reverse)
    {
        rawdata = reverse ? Reverse : Nominal;
    }
    Q_DLRBG_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct Q_EMERGENCYSTOP_t : ETCS_variable
{
    uint32_t AcceptedEoAChanged=0;
    uint32_t AcceptedEoAUnchanged=1;
    uint32_t NotRelevant=2;
    uint32_t Rejected=3;
    Q_EMERGENCYSTOP_t() : ETCS_variable(2) {}
    bool is_valid(int m_version) override
    {
        return VERSION_X(m_version) > 1 || rawdata < 3;
    }
};
struct Q_ENDTIMER_t : ETCS_variable
{
    uint32_t NoTimer=0;
    uint32_t HasTimer=1;
    Q_ENDTIMER_t() : ETCS_variable(1) {}
};
struct Q_FRONT_t : ETCS_variable
{
    uint32_t TrainLengthDelay=0;
    uint32_t NoTrainLengthDelay=1;
    Q_FRONT_t() : ETCS_variable(1) {}
};
struct Q_GDIR_t : ETCS_variable
{
    uint32_t Downhill=0;
    uint32_t Uphill=1;
    Q_GDIR_t() : ETCS_variable(1) {}
};
struct Q_LENGTH_t : ETCS_variable
{
    uint32_t NoTrainIntegrityAvailable=0;
    uint32_t TrainIntegrityConfirmedByMonitoringDevice=1;
    uint32_t TrainIntegrityConfirmedByDriver=2;
    uint32_t TrainIntegrityLost=3;
    Q_LENGTH_t() : ETCS_variable(2) {}
    Q_LENGTH_t &operator=(uint32_t data) {rawdata=data; return *this;}
};
struct Q_LGTLOC_t : ETCS_variable
{
    uint32_t MinSafeRearEnd=0;
    uint32_t MaxSafeFrontEnd=1;
    Q_LGTLOC_t() : ETCS_variable(1) {}
};
struct Q_LINK_t : ETCS_variable
{
    uint32_t Unlinked=0;
    uint32_t Linked=1;
    Q_LINK_t() : ETCS_variable(1) {}
};
struct Q_LOCACC_t : ETCS_variable
{
    Q_LOCACC_t() : ETCS_variable(6) {}
};
struct Q_LINKORIENTATION_t : ETCS_variable
{
    uint32_t Reverse=0;
    uint32_t Nominal=1;
    Q_LINKORIENTATION_t() : ETCS_variable(1) {}
};
struct Q_LINKREACTION_t : ETCS_variable
{
    uint32_t TrainTrip=0;
    uint32_t ServiceBrake=1;
    uint32_t NoReaction=2;
    Q_LINKREACTION_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct Q_LSSMA_t : ETCS_variable
{
    uint32_t ToggleOff=0;
    uint32_t ToggleOn=1;
    Q_LSSMA_t() : ETCS_variable(1) {}
};
struct Q_LXSTATUS_t : ETCS_variable
{
    uint32_t Protected=0;
    uint32_t NotProtected=1;
    Q_LXSTATUS_t() : ETCS_variable(1) {}
};
struct Q_MAMODE_t : ETCS_variable
{
    uint32_t DeriveSvL=0;
    uint32_t BeginningIsSvL=1;
    Q_MAMODE_t() : ETCS_variable(1) {}
};
struct Q_MARQSTREASON_t : ETCS_variable
{
    uint8_t StartSelectedByDriverBit=0;
    uint8_t TimeBeforePerturbationBit=1;
    uint8_t TimeBeforeTimerBit=2;
    uint8_t TrackDescriptionDeletedBit=3;
    uint8_t TrackAheadFreeBit=4;
    Q_MARQSTREASON_t() : ETCS_variable(5) {}
};
struct Q_MEDIA_t : ETCS_variable
{
    uint32_t Balise=0;
    uint32_t Loop=1;
    Q_MEDIA_t() : ETCS_variable(1) {}
};
struct Q_MPOSITION_t : ETCS_variable
{
    uint32_t Opposite=0;
    uint32_t Same=1;
    Q_MPOSITION_t() : ETCS_variable(1) {}
};
struct Q_NEWCOUNTRY_t : ETCS_variable
{
    uint32_t SameCountry=0;
    uint32_t NewCountry=1;
    Q_NEWCOUNTRY_t() : ETCS_variable(1) {}
};
struct Q_NVDRIVER_ADHES_t : ETCS_variable
{
    uint32_t NotAllowed=0;
    uint32_t Allowed=1;
    Q_NVDRIVER_ADHES_t() : ETCS_variable(1) {}
};
struct Q_NVEMRRLS_t : ETCS_variable
{
    uint32_t RevokeAtStandstill=0;
    uint32_t RevokeNotExceeded=1;
    Q_NVEMRRLS_t() : ETCS_variable(1) {}
};
struct Q_NVGUIPERM_t : ETCS_variable
{
    uint32_t No=0;
    uint32_t Yes=1;
    Q_NVGUIPERM_t() : ETCS_variable(1) {}
};
struct Q_NVINHSMICPERM_t : ETCS_variable
{
    uint32_t No=0;
    uint32_t Yes=1;
    Q_NVINHSMICPERM_t() : ETCS_variable(1) {}
};
struct Q_NVKINT_t : ETCS_variable
{
    uint32_t NoCorrectionFollow=0;
    uint32_t CorrectionFollow=1;
    Q_NVKINT_t() : ETCS_variable(1) {}
};
struct Q_NVKVINTSET_t : ETCS_variable
{
    uint32_t FreightTrains=0;
    uint32_t ConventionalPassengerTrains=1;
    Q_NVKVINTSET_t() : ETCS_variable(2) 
    {
        invalid.insert(2);
        invalid.insert(3);
    }
};
struct Q_NVLOCACC_t : ETCS_variable
{
    Q_NVLOCACC_t() : ETCS_variable(6) {}
    double get_value()
    {
        return rawdata;
    }
};
struct Q_NVSBFBPERM_t : ETCS_variable
{
    uint32_t No=0;
    uint32_t Yes=1;
    Q_NVSBFBPERM_t() : ETCS_variable(1) {}
};
struct Q_NVSBTSMPERM_t : ETCS_variable
{
    uint32_t No=0;
    uint32_t Yes=1;
    Q_NVSBTSMPERM_t() : ETCS_variable(1) {}
};
struct Q_ORIENTATION_t : ETCS_variable
{
    uint32_t Reverse=0;
    uint32_t Nominal=1;
    Q_ORIENTATION_t() : ETCS_variable(1) {}
};
struct Q_OVERLAP_t : ETCS_variable
{
    uint32_t NoOverlap=0;
    uint32_t ExistsOverlap=1;
    Q_OVERLAP_t() : ETCS_variable(1) {}
};
struct Q_PBDSR_t : ETCS_variable
{
    uint32_t EBIntervention=0;
    uint32_t SBIntervention=1;
    Q_PBDSR_t() : ETCS_variable(1) {}
};
struct Q_PLATFORM_t : ETCS_variable
{
    uint32_t LeftSide=0;
    uint32_t RightSide=1;
    uint32_t BothSides=2;
    Q_PLATFORM_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
};
struct Q_SECTIONTIMER_t : ETCS_variable
{
    uint32_t NoTimer=0;
    uint32_t HasTimer=1;
    Q_SECTIONTIMER_t() : ETCS_variable(1) {}
};
struct Q_SLEEPSESSION_t : ETCS_variable
{
    uint32_t IgnoreOrder=0;
    uint32_t ExecuteOrder=1;
    Q_SLEEPSESSION_t() : ETCS_variable(1) {}
};
struct Q_SRSTOP_t : ETCS_variable
{
    uint32_t StopIfInSR=0;
    uint32_t GoIfInSR=1;
    Q_SRSTOP_t() : ETCS_variable(1) {}
};
struct Q_STATUS_t : ETCS_variable
{
    uint32_t Invalid=0;
    uint32_t Valid=1;
    uint32_t Unknown=2;
    Q_STATUS_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct Q_STOPLX_t : ETCS_variable
{
    uint32_t NoStopRequired=0;
    uint32_t StopRequired=1;
    Q_STOPLX_t() : ETCS_variable(1) {}
};
struct Q_SUITABILITY_t : ETCS_variable
{
    uint32_t LoadingGauge=0;
    uint32_t MaxAxleLoad=1;
    uint32_t TractionSystem=2;
    Q_SUITABILITY_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct Q_RBC_t : ETCS_variable
{
    uint32_t TerminateSession=0;
    uint32_t EstablishSession=1;
    Q_RBC_t() : ETCS_variable(1) {}
};
struct Q_RIU_t : ETCS_variable
{
    uint32_t TerminateSession=0;
    uint32_t EstablishSession=1;
    Q_RIU_t() : ETCS_variable(1) {}
};
struct Q_TEXT_t : ETCS_variable
{
    uint32_t LXNotProtected=0;
    uint32_t Acknowledgement=1;
    Q_TEXT_t() : ETCS_variable(8) {}
    bool is_valid(int m_version) override
    {
        return rawdata<2;
    }
};
struct Q_TEXTCLASS_t : ETCS_variable
{
    uint32_t AuxiliaryInformation=0;
    uint32_t ImportantInformation=1;
    Q_TEXTCLASS_t() : ETCS_variable(2)
    {
        invalid.insert(2);
        invalid.insert(3);
    }
};
struct Q_TEXTDISPLAY_t : ETCS_variable
{
    uint32_t WaitOne=0;
    uint32_t WaitAll=1;
    Q_TEXTDISPLAY_t() : ETCS_variable(1) {}
};
struct Q_TEXTCONFIRM_t : ETCS_variable
{
    uint32_t NoConfirm=0;
    uint32_t Confirm=1;
    uint32_t ConfirmSB=2;
    uint32_t ConfirmEB=3;
    Q_TEXTCONFIRM_t() : ETCS_variable(2) {}
    bool is_valid(int m_version) override
    {
        return VERSION_X(m_version) > 1 || rawdata < 3;
    }
};
struct Q_TEXTREPORT_t : ETCS_variable
{
    uint32_t NoAckReport=0;
    uint32_t AckReport=1;
    Q_TEXTREPORT_t() : ETCS_variable(1) {}
};
struct Q_TRACKINIT_t : ETCS_variable
{
    uint32_t NoInitialState=0;
    uint32_t InitialState=1;
    Q_TRACKINIT_t() : ETCS_variable(1) {}
};
struct Q_VBCO_t : ETCS_variable
{
    uint32_t RemoveVBC=0;
    uint32_t SetVBC=1;
    Q_VBCO_t() : ETCS_variable(1) {}
};
struct Q_UPDOWN_t : ETCS_variable
{
    uint32_t TrainToTrack=0;
    uint32_t TrackToTrain=1;
    Q_UPDOWN_t() : ETCS_variable(1) {}
};
struct T_t : ETCS_variable
{
    T_t() : ETCS_variable(10) {}
};
struct T_CYCRQST_t : ETCS_variable
{
    uint32_t NoRepetition=255;
    T_CYCRQST_t() : ETCS_variable(8) {}
};
struct T_CYCLOC_t : ETCS_variable
{
    uint32_t Infinity=255;
    T_CYCLOC_t() : ETCS_variable(8) {}
};
struct T_EMA_t : T_t
{
    uint32_t NoTimeout=1023;
};
struct T_ENDTIMER_t : T_t
{
    uint32_t Infinity=1023;
};
struct T_LSSMA_t : ETCS_variable
{
    T_LSSMA_t() : ETCS_variable(8) {}
};
struct T_MAR_t : ETCS_variable
{
    uint32_t NoMaRequest=255;
    T_MAR_t() : ETCS_variable(8) {}
};
struct T_NVCONTACT_t : ETCS_variable
{
    uint32_t Infinity=255;
    T_NVCONTACT_t() : ETCS_variable(8) {}
};
struct T_NVOVTRP_t : ETCS_variable
{
    T_NVOVTRP_t() : ETCS_variable(8) {}
};
struct T_OL_t : T_t
{
    uint32_t Infinity=1023;
};
struct T_SECTIONTIMER_t : T_t
{
    uint32_t Infinity=1023;
};
struct T_TEXTDISPLAY_t : T_t
{
    uint32_t NoTimeLimited=1023;
};
struct T_TIMEOUTRQST_t : T_t
{
    uint32_t NoMaRequest=1023;
};
struct T_TRAIN_t : ETCS_variable
{
    uint32_t Unknown=4294967295ULL;
    T_TRAIN_t() : ETCS_variable(32) {}
    int64_t get_value() 
    {
        int64_t time = get_milliseconds();
        int64_t timestamp = time + rawdata*10LL-10LL*(uint32_t)(time/10);
        if (time-timestamp > 2147483647LL)
            timestamp += 1ULL<<32;
        else if (time-timestamp < -2147483647LL)
            timestamp -= 1ULL<<32;
        return timestamp;
    }
};
struct T_VBC_t : ETCS_variable
{
    T_VBC_t() : ETCS_variable(8) {}
    int64_t get_value()
    {
        return rawdata*86400000ULL;
    }
};
struct V_t : ETCS_variable
{
    V_t() : ETCS_variable(7) {}
    double get_value()
    {
        return (rawdata*5)/3.6;
    }
    void set_value(double val)
    {
        rawdata = (uint32_t)(val*3.6/5);
    }
    bool is_valid(int m_version) override
    {
        return rawdata < 121;
    }
};
struct V_AXLELOAD_t : V_t
{
};
struct V_DIFF_t : V_t
{
};
struct V_EMA_t : V_t
{
};
struct V_LX_t : V_t
{
};
struct V_MAIN_t : V_t
{
};
struct V_MAMODE_t : V_t
{
    uint32_t UseNationalValue=127;
    bool is_valid(int m_version) override
    {
        return rawdata<121 || rawdata>126;
    }
};
struct V_MAXTRAIN_t : V_t
{
};
struct V_NVALLOWOVTRP_t : V_t
{
};
struct V_NVKVINT_t : V_t
{
};
struct V_NVLIMSUPERV_t : V_t
{
};
struct V_NVONSIGHT_t : V_t
{
};
struct V_NVSUPOVTRP_t : V_t
{
};
struct V_NVREL_t : V_t
{
};
struct V_NVSHUNT_t : V_t
{
};
struct V_NVSTFF_t : V_t
{
};
struct V_NVUNFIT_t : V_t
{
};
struct V_release_t : V_t
{
    uint32_t CalculateOnBoard=126;
    uint32_t UseNationalValue=127;
    bool is_valid(int m_version) override
    {
        return rawdata<121 || rawdata>125;
    }
};
struct V_RELEASEDP_t : V_release_t
{
};
struct V_RELEASEOL_t : V_release_t
{
};
struct V_REVERSE_t : V_t
{
};
struct V_STATIC_t : V_t
{
    uint32_t EndOfProfile=127;
    bool is_valid(int m_version) override
    {
        return rawdata<121 || rawdata==127;
    }
};
struct V_TRAIN_t : V_t
{
};
struct V_TSR_t : V_t
{
    bool is_valid(int m_version) override
    {
        return rawdata<121;
    }
};
struct X_TEXT_t : ETCS_variable
{
    X_TEXT_t() : ETCS_variable(8) {}
    static std::string getUTF8(const std::vector<X_TEXT_t> &chars)
    {
        std::string text;
        for (int i=0; i<chars.size(); i++) {
            unsigned char c = chars[i];

#if SIMRAIL
            // UTF-8, due to multi-language and non-latin support
            text += c;
#else
            // ISO-8859-1, according to S.R.S 7.5.1.174
            if (chars[i] < 0x80) {
                text += c;
            } else {
                text += 0xc2+(c>0xbf);
                text += (c&0x3f)+0x80;
            }
#endif
        }
        return text;
    }
};