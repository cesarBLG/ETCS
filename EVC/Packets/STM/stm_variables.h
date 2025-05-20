/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "../variables.h"
struct L_CAPTION_t : ETCS_variable
{
    L_CAPTION_t() : ETCS_variable(6) {}
};
struct L_VALUE_t : ETCS_variable
{
    L_VALUE_t() : ETCS_variable(5) {}
};
struct L_MESSAGE_stm_t : ETCS_variable
{
    L_MESSAGE_stm_t() : ETCS_variable(8) {}
};
struct M_BUT_ATTRIB_t : ETCS_variable
{
    M_BUT_ATTRIB_t() : ETCS_variable(10) {}
    bool is_valid(int m_version) override
    {
        return ((rawdata>>6)&3) != 3;
    }
};
struct M_DATAENTRYFLAG_t : ETCS_variable
{
    static const uint32_t Stop=0;
    static const uint32_t Start=1;
    M_DATAENTRYFLAG_t() : ETCS_variable(1) {}
};
struct NID_DRV_LANGUAGE_t : ETCS_variable
{
    NID_DRV_LANGUAGE_t() : ETCS_variable(16) {}
};
struct M_IND_ATTRIB_t : ETCS_variable
{
    M_IND_ATTRIB_t() : ETCS_variable(10) {}
    bool is_valid(int m_version) override
    {
        return ((rawdata>>6)&3) != 3;
    }
};
struct M_TRAINTYPE_t : ETCS_variable
{
    M_TRAINTYPE_t() : ETCS_variable(8) {}
};
struct NID_ICON_t : ETCS_variable
{
    NID_ICON_t() : ETCS_variable(8) {}
};
struct NID_BUTTON_t : ETCS_variable
{
    NID_BUTTON_t() : ETCS_variable(8) {}
};
struct NID_DATA_t : ETCS_variable
{
    NID_DATA_t() : ETCS_variable(8) {}
};
struct NID_INDICATOR_t : ETCS_variable
{
    NID_INDICATOR_t() : ETCS_variable(8) {}
};
struct NID_BUTPOS_t : ETCS_variable
{
    NID_BUTPOS_t() : ETCS_variable(5) {}
};
struct NID_INDPOS_t : ETCS_variable
{
    NID_INDPOS_t() : ETCS_variable(5) {}
};
struct NID_SOUND_t : ETCS_variable
{
    NID_SOUND_t() : ETCS_variable(8) {}
};
struct NID_STM_t : ETCS_variable
{
    NID_STM_t() : ETCS_variable(8) {}
};
struct NID_STMSTATE_t : ETCS_variable
{
    NID_STMSTATE_t() : ETCS_variable(4) {}
};
struct NID_STMSTATEORDER_t : ETCS_variable
{
    NID_STMSTATEORDER_t() : ETCS_variable(4) {}
};
struct NID_STMSTATEREQUEST_t : ETCS_variable
{
    NID_STMSTATEREQUEST_t() : ETCS_variable(4) {}
};
struct NID_XMESSAGE_t : ETCS_variable
{
    NID_XMESSAGE_t() : ETCS_variable(8) {}
};
struct M_BIEB_CMD_t : ETCS_variable
{
    static const uint32_t ApplyEB=1;
    static const uint32_t ReleaseEB=2;
    static const uint32_t NoChange=3;
    M_BIEB_CMD_t() : ETCS_variable(2)
    {
        invalid.insert(0);
    }
};
struct M_BISB_CMD_t : ETCS_variable
{
    static const uint32_t ApplySBorEB=0;
    static const uint32_t ApplySB=1;
    static const uint32_t ReleaseSB=2;
    static const uint32_t NoChange=3;
    M_BISB_CMD_t() : ETCS_variable(2) {}
};
struct M_TIPANTO_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t PantoLift=1;
    static const uint32_t PantoLower=2;
    static const uint32_t NoChange=3;
    M_TIPANTO_CMD_t() : ETCS_variable(2) {}
};
struct M_TIFLAP_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t FlatOpen=1;
    static const uint32_t FlatClose=2;
    static const uint32_t NoChange=3;
    M_TIFLAP_CMD_t() : ETCS_variable(2) {}
};
struct M_TIMS_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t MainSwitchClose=1;
    static const uint32_t MainSwitchOpen=2;
    static const uint32_t NoChange=3;
    M_TIMS_CMD_t() : ETCS_variable(2) {}
};
struct M_TITR_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t TCO=1;
    static const uint32_t NoTCO=2;
    static const uint32_t NoChange=3;
    M_TITR_CMD_t() : ETCS_variable(2) {}
};
struct M_TIRB_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t AllowRegenerative=1;
    static const uint32_t SupressRegenerative=2;
    static const uint32_t NoChange=3;
    M_TIRB_CMD_t() : ETCS_variable(2) {}
};
struct M_TIMSH_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t AllowMagnetic=1;
    static const uint32_t SupressMagnetic=2;
    static const uint32_t NoChange=3;
    M_TIMSH_CMD_t() : ETCS_variable(2) {}
};
struct M_TIEDCBEB_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t AllowEddyEB=1;
    static const uint32_t SupressEddyEB=2;
    static const uint32_t NoChange=3;
    M_TIEDCBEB_CMD_t() : ETCS_variable(2) {}
};
struct M_TIEDCBSB_CMD_t : ETCS_variable
{
    static const uint32_t Reserved=0;
    static const uint32_t AllowEddySB=1;
    static const uint32_t SupressEddySB=2;
    static const uint32_t NoChange=3;
    M_TIEDCBSB_CMD_t() : ETCS_variable(2) {}
};
struct M_COLOUR_t : ETCS_variable
{
    M_COLOUR_t() : ETCS_variable(3)
    {
        invalid.insert(7);
    }
    int get_value() const
    {
        return rawdata;
    }
};
struct M_XATTRIBUTE_t : ETCS_variable
{
    M_XATTRIBUTE_t() : ETCS_variable(10) {}
};
struct M_FREQ_t : ETCS_variable
{
    M_FREQ_t() : ETCS_variable(8)
    {
        invalid.insert(1);
        invalid.insert(2);
        invalid.insert(3);
    }
    double get_value() const
    {
        if (rawdata == 0) return 0;
        return rawdata * 32;
    }
};
struct N_VERMAJOR_t : ETCS_variable
{
    N_VERMAJOR_t() : ETCS_variable(8) {}
};
struct N_VERMINOR_t : ETCS_variable
{
    N_VERMINOR_t() : ETCS_variable(8) {}
};
struct T_BUTTONEVENT_t : ETCS_variable
{
    T_BUTTONEVENT_t() : ETCS_variable(32) {}
};
struct T_SOUND_t : ETCS_variable
{
    T_SOUND_t() : ETCS_variable(8) {}
    bool is_valid(int m_version) override
    {
        return rawdata < 101 && rawdata > 0;
    }
    double get_value() const
    {
        return rawdata * 0.1f;
    }
};
struct V_DISPLAY_t : ETCS_variable
{
    static const uint32_t Unknown=1024;
    V_DISPLAY_t() : ETCS_variable(10) {}
    double get_value() const
    {
        return rawdata/3.6;
    }
    bool is_valid(int m_version) override
    {
        return rawdata < 601 || rawdata == 1023;
    }
};
struct Q_ACK_t : ETCS_variable
{
    static const uint32_t NoAcknowledgement=0;
    static const uint32_t AcknowledgementRequired=1;
    Q_ACK_t() : ETCS_variable(1) {}
};
struct Q_BUTTON_t : ETCS_variable
{
    static const uint32_t NotPressed=0;
    static const uint32_t Pressed=1;
    Q_BUTTON_t() : ETCS_variable(1) {}
};
struct Q_DATAENTRY_t : ETCS_variable
{
    static const uint32_t NoSpecificData=0;
    static const uint32_t SpecificDataNeeded=1;
    Q_DATAENTRY_t() : ETCS_variable(1) {}
};
struct Q_DISPLAY_PS_t : ETCS_variable
{
    static const uint32_t NoDisplay = 0;
    static const uint32_t HookOnly = 1;
    static const uint32_t GaugeOnly = 2;
    static const uint32_t GaugeAndHook = 3;
    Q_DISPLAY_PS_t() : ETCS_variable(2) {}
};
struct Q_DISPLAY_TS_t : ETCS_variable
{
    static const uint32_t NoDisplay = 0;
    static const uint32_t HookOnly = 1;
    static const uint32_t GaugeOnly = 2;
    static const uint32_t GaugeAndHook = 3;
    Q_DISPLAY_TS_t() : ETCS_variable(2) {}
};
struct Q_DISPLAY_RS_t : ETCS_variable
{
    static const uint32_t NoDisplay = 0;
    static const uint32_t DigitOnly = 1;
    static const uint32_t GaugeOnly = 2;
    static const uint32_t GaugeAndDigit = 3;
    Q_DISPLAY_RS_t() : ETCS_variable(2) {}
};
struct Q_DISPLAY_IS_t : ETCS_variable
{
    static const uint32_t NoDisplay = 0;
    static const uint32_t NormalGauge = 1;
    static const uint32_t WideGauge = 2;
    Q_DISPLAY_IS_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct Q_DISPLAY_TD_t : ETCS_variable
{
    static const uint32_t NoDisplay = 0;
    static const uint32_t DigitOnly = 1;
    static const uint32_t GaugeOnly = 2;
    static const uint32_t GaugeAndDigit = 3;
    Q_DISPLAY_TD_t() : ETCS_variable(2) {}
};
struct Q_FOLLOWING_t : ETCS_variable
{
    static const uint32_t NoFollowing = 0;
    static const uint32_t FollowingRequest = 1;
    Q_FOLLOWING_t() : ETCS_variable(1) {}
};
struct Q_OVR_STATUS_t : ETCS_variable
{
    static const uint32_t OverrideNotActive = 0;
    static const uint32_t OverrideActive = 1;
    Q_OVR_STATUS_t() : ETCS_variable(1) {}
};
struct Q_SOUND_t : ETCS_variable
{
    static const uint32_t Stop = 0;
    static const uint32_t PlayOnce = 1;
    static const uint32_t PlayContinuously = 2;
    Q_SOUND_t() : ETCS_variable(2)
    {
        invalid.insert(3);
    }
};
struct V_PERMIT_t : V_DISPLAY_t {};
struct V_RELEASE_t : V_DISPLAY_t {};
struct V_TARGET_t : V_t {};
struct V_INTERV_t : V_DISPLAY_t {};
struct D_TARGET_t : D_t {};
struct M_COLOUR_SP_t : M_COLOUR_t {};
struct M_COLOUR_PS_t : M_COLOUR_t {};
struct M_COLOUR_TS_t : M_COLOUR_t {};
struct M_COLOUR_RS_t : M_COLOUR_t {};
struct M_COLOUR_IS_t : M_COLOUR_t {};
struct X_CAPTION_t : ETCS_variable
{
    X_CAPTION_t() : ETCS_variable(8) {}
    static std::string getUTF8(const std::vector<X_CAPTION_t> &chars)
    {
        std::string text;
        for (int i=0; i<chars.size(); i++) {
            unsigned char c = chars[i];
            if (chars[i] < 0x80) {
                text += c;
            } else {
                text += 0xc2+(c>0xbf);
                text += (c&0x3f)+0x80;
            }
        }
        return text;
    }
};
struct X_VALUE_t : ETCS_variable
{
    X_VALUE_t() : ETCS_variable(8) {}
    static std::string getUTF8(const std::vector<X_VALUE_t> &chars)
    {
        std::string text;
        for (int i=0; i<chars.size(); i++) {
            unsigned char c = chars[i];
            if (chars[i] < 0x80) {
                text += c;
            } else {
                text += 0xc2+(c>0xbf);
                text += (c&0x3f)+0x80;
            }
        }
        return text;
    }
};
struct V_STMMAX_t : V_t
{
    static const uint32_t NoMaxSpeed = 127;
};
struct V_STMSYS_t : V_t
{
    static const uint32_t NoSystemSpeed = 127;
};
struct D_STMSYS_t : ETCS_variable
{
    D_STMSYS_t() : ETCS_variable(15) {}
    double get_value()
    {
        return rawdata * 10;
    }
};