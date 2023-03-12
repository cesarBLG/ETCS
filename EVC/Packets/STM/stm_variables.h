#pragma once
#include "../variables.h"
#include "../../../DMI/graphics/color.h"
struct L_CAPTION_t : ETCS_variable
{
    L_CAPTION_t() : ETCS_variable(6) {}
};
struct L_MESSAGE_stm_t : ETCS_variable
{
    L_MESSAGE_stm_t() : ETCS_variable(8) {}
};
struct M_BUT_ATTRIB_t : ETCS_variable
{
    M_BUT_ATTRIB_t() : ETCS_variable(10) {}
    bool is_valid() override
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
    bool is_valid() override
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
struct M_COLOUR_t : ETCS_variable
{
    M_COLOUR_t() : ETCS_variable(3)
    {
        invalid.insert(7);
    }
    Color get_value() const
    {
        switch (rawdata) {
            case 0:
                return White;
            case 1:
                return Grey;
            case 2:
                return MediumGrey;
            case 3:
                return DarkGrey;
            case 4:
                return Yellow;
            case 5:
                return Orange;
            case 6:
                return Red;
        }
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
struct T_SOUND_t : ETCS_variable
{
    T_SOUND_t() : ETCS_variable(8) {}
    bool is_valid() override
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
    bool is_valid() override
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
};