#pragma once
#include "../variables.h"
namespace V1
{
struct M_AXLELOAD_t : ETCS_variable
{
    M_AXLELOAD_t() : ETCS_variable(7) {}
};
struct M_MAMODE_t : ETCS_variable
{
    static const uint32_t OS=0;
    static const uint32_t SH=1;
    M_MAMODE_t() : ETCS_variable(2) 
    {
        invalid.insert(2);
        invalid.insert(3);
    }
};
struct M_POSITION_t : ETCS_variable
{
    static const uint32_t NoMoreCalculation=1048575;
    M_POSITION_t() : ETCS_variable(20) {}
};
struct M_TRACTION_t : ETCS_variable
{
    M_TRACTION_t() : ETCS_variable(8) {}
};
struct NC_DIFF_t : ETCS_variable
{
    static const uint32_t CD275=0;
    static const uint32_t CD80=1;
    static const uint32_t CD100=2;
    static const uint32_t CD130=3;
    static const uint32_t CD150=4;
    static const uint32_t CD165=5;
    static const uint32_t CD180=6;
    static const uint32_t CD225=7;
    static const uint32_t CD300=8;
    static const uint32_t FreightP=9;
    static const uint32_t FreightG=10;
    static const uint32_t Passenger=11;
    static const uint32_t CD245=12;
    static const uint32_t CD210=13;
    NC_DIFF_t() : ETCS_variable(4)
    {
        invalid.insert(14);
        invalid.insert(15);
    }
};
struct Q_TRACKDEL_t : ETCS_variable
{
    Q_TRACKDEL_t() : ETCS_variable(1) {}
};
}