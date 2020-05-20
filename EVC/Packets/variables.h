#pragma once
#include <stdint.h>
#include <limits>
#include <set>
#include "../Supervision/supervision.h"
struct ETCS_variable
{
    int size;
    uint32_t rawdata;
    std::set<uint32_t> invalid;
    ETCS_variable(int size) : size(size){}
    operator uint32_t() const
    {
        return rawdata;
    }
    virtual bool is_valid()
    {
        return invalid.find(rawdata)==invalid.end();
    }
};
struct Q_SCALE_t : ETCS_variable
{
    static const uint32_t cm10 = 0;
    static const uint32_t m1 = 1;
    static const uint32_t m10 = 2;
    Q_SCALE_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
};
struct D_t : ETCS_variable
{
    D_t() : ETCS_variable(15) {}
    virtual double get_value(const Q_SCALE_t scale) const
    {
        double fact=1;
        if (scale == Q_SCALE_t::m10)
            fact = 10;
        else if (scale == Q_SCALE_t::cm10)
            fact = 0.1;
        return fact*rawdata;
    }
};
struct D_CYCLOC_t : D_t
{
    static const uint32_t NoCyclicalReportPosition=32767;
};
struct D_DP_t : D_t
{
};
struct D_ENDTIMERSTARTLOC_t : D_t
{
};
struct D_GRADIENT_t : D_t
{
};
struct D_LEVELTR_t : D_t
{
    static const uint32_t Now=32767;
};
struct D_LINK_t : D_t
{
};
struct D_MAMODE_t : D_t
{
};
struct D_NVROLL_t : D_t
{
    static const uint32_t Infinity=32767;
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
struct D_REF_t : ETCS_variable
{
    D_REF_t() : ETCS_variable(16){}
    double get_value(Q_SCALE_t scale)
    {
        double fact=1;
        if (scale == Q_SCALE_t::m10)
            fact = 10;
        else if (scale == Q_SCALE_t::cm10)
            fact = 0.1;
        if (rawdata>32767)
            return -fact*((rawdata ^ 65535)+1);
        else
            return rawdata*fact;
    }
};
struct D_SECTIONTIMERSTOPLOC_t : D_t
{
};
struct D_STATIC_t : D_t
{
};
struct D_STARTOL_t : D_t
{
};
struct D_TEXTDISPLAY_t : D_t
{
    static const uint32_t NotDistanceLimited=32767;
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
struct G_A_t : ETCS_variable
{
    static const uint32_t EndOfGradient=255;
    G_A_t() : ETCS_variable(8) {}
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
struct L_PACKET_t : ETCS_variable
{
    L_PACKET_t() : ETCS_variable(13) {}
};
struct L_ACKLEVELTR_t : D_t
{
};
struct L_ACKMAMODE_t : D_t
{
};
struct L_ENDSECTION_t : D_t
{
};
struct L_MAMODE_t : D_t
{
};
struct L_SECTION_t : D_t
{
};
struct L_TEXT_t : ETCS_variable
{
    L_TEXT_t() : ETCS_variable(8) {}
};
struct L_TEXTDISPLAY_t : D_t
{
    static const uint32_t NotDistanceLimited=32767;
};
struct L_TRACKCOND_t : D_t
{
};
struct L_TSR_t : D_t
{
};
struct M_AIRTIGHT_t : ETCS_variable
{
    static const uint32_t NotFitted=0;
    static const uint32_t Fitted=1;
    M_AIRTIGHT_t() : ETCS_variable(2)
    {
        invalid.insert(2);
        invalid.insert(3);
    }
};
struct M_DUP_t : ETCS_variable
{
    static const uint32_t NoDuplicates=0;
    static const uint32_t DuplicateOfNext=1;
    static const uint32_t DuplicateOfPrev=2;
    M_DUP_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct M_LEVELTEXTDISPLAY_t : ETCS_variable
{
    static const uint32_t N0=0;
    static const uint32_t NTC=1;
    static const uint32_t N1=2;
    static const uint32_t N2=3;
    static const uint32_t N3=4;
    static const uint32_t NoLevelLimited=5;
    M_LEVELTEXTDISPLAY_t() : ETCS_variable(3)
    {
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_value() const
    {
        switch (rawdata) {
            case N0:
                return Level::N0;
            case N1:
                return Level::N1;
            case N2:
                return Level::N2;
            case N3:
                return Level::N3;
            case NTC:
                return Level::NTC;
            default:
                return Level::Unknown;
        }
    }
};
struct M_LEVELTR_t : ETCS_variable
{
    M_LEVELTR_t() : ETCS_variable(3)
    {
        invalid.insert(5);
        invalid.insert(6);
        invalid.insert(7);
    }
    Level get_level()
    {
        switch (rawdata) {
            case 0:
                return Level::N0;
            case 1:
                return Level::NTC;
            case 2:
                return Level::N1;
            case 3:
                return Level::N2;
            case 4:
                return Level::N3;
            default:
                return Level::N0;
        }
    }
};
struct M_LINEGAUGE_t : ETCS_variable
{
    static const uint32_t G1 = 1;
    static const uint32_t GA = 2;
    static const uint32_t GB = 4;
    static const uint32_t GC = 8;
    M_LINEGAUGE_t() : ETCS_variable(8) {}
    bool is_valid() override
    {
        return rawdata!=0 && (rawdata&0xF0)==0;
    }
};
struct M_MAMODE_t : ETCS_variable
{
    static const uint32_t OS=0;
    static const uint32_t SH=1;
    static const uint32_t LS=2;
    M_MAMODE_t() : ETCS_variable(3) 
    {
        invalid.insert(3);
    }
};
struct M_MCOUNT_t : ETCS_variable
{
    static const uint32_t NeverFitsTelegrams=254;
    static const uint32_t FitsAllTelegrams=255;
    M_MCOUNT_t() : ETCS_variable(8) {}
};
struct M_MODETEXTDISPLAY_t : ETCS_variable
{
    static const uint32_t FS=0;
    static const uint32_t OS=1;
    static const uint32_t SR=2;
    static const uint32_t UN=4;
    static const uint32_t SB = 6;
    static const uint32_t TR = 7;
    static const uint32_t PT = 8;
    static const uint32_t LS = 12;
    static const uint32_t RV = 14;
    static const uint32_t NoModeLimited = 15;
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
        switch (rawdata)
        {
            case FS:
                return Mode::FS;
            case OS:
                return Mode::OS;
            case SR:
                return Mode::SR;
            case UN:
                return Mode::UN;
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
struct M_POSITION_t : ETCS_variable
{
    static const uint32_t NoMoreCalculation=16777215UL;
    M_POSITION_t() : ETCS_variable(24) {}
    bool is_valid() override
    {
        return rawdata<10000000UL || rawdata == 16777215UL;
    }
};
struct M_TRACKCOND_t : ETCS_variable
{
    static const uint32_t NonStoppingArea=0;
    static const uint32_t TunnelStoppingArea=1;
    static const uint32_t SoundHorn=2;
    static const uint32_t PowerlessLowerPantograph=3;
    static const uint32_t RadioHole=4;
    static const uint32_t AirTightness=5;
    static const uint32_t SwitchOffRegenerative=6;
    static const uint32_t SwitchOffEddyService=7;
    static const uint32_t SwitchOffShoe=8;
    static const uint32_t PowerlessSwitchOffPower=9;
    static const uint32_t SwitchOffEddyEmergency=10;
    M_TRACKCOND_t() : ETCS_variable(4) {}
    bool is_valid() override
    {
        return rawdata<11;
    }
};
struct M_VERSION_t : ETCS_variable
{
    static const uint32_t V1_0=16;
    static const uint32_t V1_1=17;
    static const uint32_t V2_0=32;
    static const uint32_t V2_1=33;
    M_VERSION_t() : ETCS_variable(7) {}
    bool is_valid() override
    {
        return rawdata<17 || rawdata>31;
    }
};
struct NC_CDDIFF_t : ETCS_variable
{
    NC_CDDIFF_t() : ETCS_variable(4) {}
    bool is_valid() override
    {
        return rawdata<11;
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
    static const uint32_t FreightP=0;
    static const uint32_t FreightG=1;
    static const uint32_t Passenger=2;
    NC_DIFF_t() : ETCS_variable(4) {}
    bool is_valid() override
    {
        return rawdata<3;
    }
};
struct NID_BG_t : ETCS_variable
{
    NID_BG_t() : ETCS_variable(14) {}
};
struct NID_C_t : ETCS_variable
{
    NID_C_t() : ETCS_variable(10) {}
};
struct NID_NTC_t : ETCS_variable
{
    NID_NTC_t() : ETCS_variable(8) {}
};
struct NID_RBC_t : ETCS_variable
{
    static const uint32_t ContactLastRBC=16383;
    NID_RBC_t() : ETCS_variable(14) {}
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
                break;
            value = 10*value + c;
        }
        return value;
    }
    bool is_valid() override
    {
        if (rawdata == 0xFFFFFFFF)
            return false;
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
};
struct NID_TSR_t : ETCS_variable
{
    static const uint32_t NonRevocable=255;
    NID_TSR_t() : ETCS_variable(8) {}
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
struct T_t : ETCS_variable
{
    T_t() : ETCS_variable(10) {}
};
struct T_EMA_t : T_t
{
    static const uint32_t NoTimeout=1023;
};
struct T_ENDTIMER_t : T_t
{
    static const uint32_t Infinity=1023;
};
struct T_OL_t : T_t
{
    static const uint32_t Infinity=1023;
};
struct T_SECTIONTIMER_t : T_t
{
    static const uint32_t Infinity=1023;
};
struct T_TEXTDISPLAY_t : T_t
{
    static const uint32_t NoTimeLimited=1023;
};
struct Q_CONFTEXTDISPLAY_t : ETCS_variable
{
    static const uint32_t AcknowledgeEnds=0;
    static const uint32_t AcknowledgeRequired=1;
    Q_CONFTEXTDISPLAY_t() : ETCS_variable(1) {}
};
struct Q_DANGERPOINT_t : ETCS_variable
{
    static const uint32_t NoDangerpoint=0;
    static const uint32_t ExistsDangerpoint=1;
    Q_DANGERPOINT_t() : ETCS_variable(1) {}
};
struct Q_DIFF_t : ETCS_variable
{
    static const uint32_t CantDeficiency = 0;
    static const uint32_t OtherSpecificReplacesCant = 1;
    static const uint32_t OtherSpecificNotReplacesCant = 2;
    Q_DIFF_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct Q_DIR_t : ETCS_variable
{
    static const uint32_t Reverse = 0;
    static const uint32_t Nominal = 1;
    static const uint32_t Both = 2;
    Q_DIR_t() : ETCS_variable(2) 
    {
        invalid.insert(3);
    }
};
struct Q_ENDTIMER_t : ETCS_variable
{
    static const uint32_t NoTimer=0;
    static const uint32_t HasTimer=1;
    Q_ENDTIMER_t() : ETCS_variable(1) {}
};
struct Q_FRONT_t : ETCS_variable
{
    static const uint32_t TrainLengthDelay=0;
    static const uint32_t NoTrainLengthDelay=1;
    Q_FRONT_t() : ETCS_variable(1) {}
};
struct Q_GDIR_t : ETCS_variable
{
    static const uint32_t Downhill=0;
    static const uint32_t Uphill=1;
    Q_GDIR_t() : ETCS_variable(1) {}
};
struct Q_LINK_t : ETCS_variable
{
    static const uint32_t Unlinked=0;
    static const uint32_t Linked=1;
    Q_LINK_t() : ETCS_variable(1) {}
};
struct Q_LOCACC_t : ETCS_variable
{
    Q_LOCACC_t() : ETCS_variable(6) {}
};
struct Q_LINKORIENTATION_t : ETCS_variable
{
    static const uint32_t Reverse=0;
    static const uint32_t Nominal=1;
    Q_LINKORIENTATION_t() : ETCS_variable(1) {}
};
struct Q_LINKREACTION_t : ETCS_variable
{
    static const uint32_t TrainTrip=0;
    static const uint32_t ServiceBrake=1;
    static const uint32_t NoReaction=2;
    Q_LINKREACTION_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct Q_MAMODE_t : ETCS_variable
{
    static const uint32_t DeriveSvL=0;
    static const uint32_t BeginningIsSvL=1;
    Q_MAMODE_t() : ETCS_variable(1) {}
};
struct Q_MEDIA_t : ETCS_variable
{
    static const uint32_t Balise=0;
    static const uint32_t Loop=1;
    Q_MEDIA_t() : ETCS_variable(1) {}
};
struct Q_MPOSITION_t : ETCS_variable
{
    static const uint32_t Opposite=0;
    static const uint32_t Same=1;
    Q_MPOSITION_t() : ETCS_variable(1) {}
};
struct Q_NEWCOUNTRY_t : ETCS_variable
{
    static const uint32_t SameCountry=0;
    static const uint32_t NewCountry=1;
    Q_NEWCOUNTRY_t() : ETCS_variable(1) {}
};
struct Q_OVERLAP_t : ETCS_variable
{
    static const uint32_t NoOverlap=0;
    static const uint32_t ExistsOverlap=1;
    Q_OVERLAP_t() : ETCS_variable(1) {}
};
struct Q_SECTIONTIMER_t : ETCS_variable
{
    static const uint32_t NoTimer=0;
    static const uint32_t HasTimer=1;
    Q_SECTIONTIMER_t() : ETCS_variable(1) {}
};
struct Q_SRSTOP_t : ETCS_variable
{
    static const uint32_t StopIfInSR=0;
    static const uint32_t GoIfInSR=1;
    Q_SRSTOP_t() : ETCS_variable(1) {}
};
struct Q_TEXT_t : ETCS_variable
{
    static const uint32_t LXNotProtected=0;
    static const uint32_t Acknowledgement=1;
    Q_TEXT_t() : ETCS_variable(8) {}
    bool is_valid() override
    {
        return rawdata<2;
    }
};
struct Q_TEXTCLASS_t : ETCS_variable
{
    static const uint32_t AuxiliaryInformation=0;
    static const uint32_t ImportantInformation=1;
    Q_TEXTCLASS_t() : ETCS_variable(2)
    {
        invalid.insert(2);
        invalid.insert(3);
    }
};
struct Q_TEXTDISPLAY_t : ETCS_variable
{
    static const uint32_t WaitOne=0;
    static const uint32_t WaitAll=1;
    Q_TEXTDISPLAY_t() : ETCS_variable(1) {}
};
struct Q_TEXTCONFIRM_t : ETCS_variable
{
    static const uint32_t NoConfirm=0;
    static const uint32_t Confirm=1;
    static const uint32_t ConfirmSB=2;
    static const uint32_t ConfirmEB=3;
    Q_TEXTCONFIRM_t() : ETCS_variable(2) {}
};
struct Q_TEXTREPORT_t : ETCS_variable
{
    static const uint32_t NoAckReport=0;
    static const uint32_t AckReport=1;
    Q_TEXTREPORT_t() : ETCS_variable(1) {}
};
struct Q_TRACKINIT_t : ETCS_variable
{
    static const uint32_t NoInitialState=0;
    static const uint32_t InitialState=1;
    Q_TRACKINIT_t() : ETCS_variable(1) {}
};
struct Q_UPDOWN_t : ETCS_variable
{
    static const uint32_t TrainToTrack=0;
    static const uint32_t TrackToTrain=1;
    Q_UPDOWN_t() : ETCS_variable(1) {}
};
struct V_t : ETCS_variable
{
    V_t() : ETCS_variable(7) {}
    double get_value()
    {
        return rawdata*(5/3.6);
    }
    bool is_valid() override
    {
        return rawdata < 121;
    }
};
struct V_DIFF_t : V_t
{
};
struct V_EMA_t : V_t
{
};
struct V_MAIN_t : V_t
{
};
struct V_MAMODE_t : V_t
{
    static const uint32_t UseNationalValue=127;
    bool is_valid() override
    {
        return rawdata<121 || rawdata>126;
    }
};
struct V_RELEASE_t : V_t
{
    static const uint32_t CalculateOnBoard=126;
    static const uint32_t UseNationalValue=127;
    bool is_valid() override
    {
        return rawdata<121 || rawdata>125;
    }
};
struct V_RELEASEDP_t : V_RELEASE_t
{
};
struct V_RELEASEOL_t : V_RELEASE_t
{
};
struct V_STATIC_t : V_t
{
    static const uint32_t EndOfProfile=127;
    bool is_valid() override
    {
        return rawdata<121 || rawdata==127;
    }
};
struct V_TSR_t : V_t
{
    bool is_valid() override
    {
        return rawdata<121;
    }
};
struct X_TEXT_t : ETCS_variable
{
    X_TEXT_t() : ETCS_variable(8) {}
};