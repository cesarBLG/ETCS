/*
 * European Train Control System
 * Copyright (C) 2022  César Benito <cesarbema2009@hotmail.com>
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
#pragma once
#include <vector>
#include <map>
#include <memory>
#include "variables.h"
#include "types.h"
#include "packets.h"
#include "TrainToTrack/0.h"
#include "TrainToTrack/1.h"
#include "TrainToTrack/11.h"
#include "../optional.h"
#include "15.h"
extern bool ma_rq_reasons[5];
extern int position_report_reasons[12];
struct position_report_parameters
{
    int64_t T_sendreport;
    double D_sendreport;
    optional<distance> location_front; 
    optional<distance> location_rear; 
    bool LRBG;
};
extern optional<position_report_parameters> pos_report_params;
struct euroradio_message
{
    NID_MESSAGE_t NID_MESSAGE;
    L_MESSAGE_t L_MESSAGE;
    T_TRAIN_t T_TRAIN;
    M_ACK_t M_ACK;
    NID_LRBG_t NID_LRBG;
    std::vector<std::shared_ptr<ETCS_packet>> packets;
    std::vector<std::shared_ptr<ETCS_packet>> optional_packets;
    bool valid;
    bool readerror;
    euroradio_message() {}
    virtual void copy(bit_manipulator &r)
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
    }
    void write_to(bit_manipulator &w)
    {
        copy(w);
        for (auto pack : optional_packets) {
            pack->write_to(w);
        }
        while(w.bits.size()%8) w.bits.push_back(false);
        L_MESSAGE_t L_MESSAGE;
        L_MESSAGE.rawdata = w.bits.size()/8;
        w.replace(&L_MESSAGE, 8);
    }
    static std::shared_ptr<euroradio_message> build(bit_manipulator &r);
};
struct euroradio_message_traintotrack
{
    NID_MESSAGE_t NID_MESSAGE;
    L_MESSAGE_t L_MESSAGE;
    T_TRAIN_t T_TRAIN;
    NID_ENGINE_t NID_ENGINE;
    optional<std::shared_ptr<PositionReport>> PositionReport1BG;
    optional<std::shared_ptr<PositionReportBasedOnTwoBaliseGroups>> PositionReport2BG;
    static std::map<int, euroradio_message_traintotrack*> message_factory;
    std::vector<std::shared_ptr<ETCS_packet>> packets;
    std::vector<std::shared_ptr<ETCS_packet>> optional_packets;
    bool valid;
    bool readerror;
    euroradio_message_traintotrack()
    {
        L_MESSAGE.rawdata = 0;
    }
    euroradio_message_traintotrack(bit_manipulator &r)
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        NID_ENGINE.copy(r);
    }
    void copy_position_report(bit_manipulator &r)
    {
        if (r.write_mode) {
            if (PositionReport1BG)
                (*PositionReport1BG)->write_to(r);
            else if (PositionReport2BG)
                (*PositionReport2BG)->write_to(r);
        } else {
            NID_PACKET_t NID_PACKET;
            r.peek(&NID_PACKET);
            PositionReport1BG = {};
            PositionReport2BG = {};
            if (NID_PACKET == 0) {
                PositionReport1BG = std::shared_ptr<PositionReport>(new PositionReport());
                (*PositionReport1BG)->copy(r);
            } else if (NID_PACKET == 1) {
                PositionReport2BG = std::shared_ptr<PositionReportBasedOnTwoBaliseGroups>(new PositionReportBasedOnTwoBaliseGroups());
                (*PositionReport2BG)->copy(r);
            }
        }
    }
    static std::shared_ptr<euroradio_message_traintotrack> build(bit_manipulator &r);
    virtual void copy(bit_manipulator &w) 
    {
        NID_MESSAGE.copy(w);
        L_MESSAGE.copy(w);
        T_TRAIN.copy(w);
        NID_ENGINE.copy(w);
    }
    void write_to(bit_manipulator &w)
    {
        copy(w);
        for (auto pack : optional_packets) {
            pack->write_to(w);
        }
        while(w.bits.size()%8) w.bits.push_back(false);
        L_MESSAGE_t L_MESSAGE;
        L_MESSAGE.rawdata = w.bits.size()/8;
        w.replace(&L_MESSAGE, 8);
    }
};
struct MA_message : euroradio_message
{
    std::shared_ptr<Level2_3_MA> MA;
    MA_message()
    {
        MA = std::shared_ptr<Level2_3_MA>(new Level2_3_MA());
        packets.push_back(MA);
    }
    virtual void copy(bit_manipulator &w) override
    {
        NID_MESSAGE.copy(w);
        L_MESSAGE.copy(w);
        T_TRAIN.copy(w);
        M_ACK.copy(w);
        NID_LRBG.copy(w);
        MA->copy(w);
    }
};
struct TR_exit_recognition : euroradio_message
{
};
struct train_data_acknowledgement : euroradio_message
{
    T_TRAIN_t T_TRAINack;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        T_TRAINack.copy(r);
    }
};
struct conditional_emergency_stop : euroradio_message
{
    NID_EM_t NID_EM;
    Q_SCALE_t Q_SCALE;
    D_REF_t D_REF;
    Q_DIR_t Q_DIR;
    D_EMERGENCYSTOP_t D_EMERGENCYSTOP;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        NID_EM.copy(r);
        Q_SCALE.copy(r);
        D_REF.copy(r);
        Q_DIR.copy(r);
        D_EMERGENCYSTOP.copy(r);
    }
};
struct unconditional_emergency_stop : euroradio_message
{
    NID_EM_t NID_EM;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        NID_EM.copy(r);
    }
};
struct emergency_stop_revocation : euroradio_message
{
    NID_EM_t NID_EM;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        NID_EM.copy(r);
    }
};
struct SR_authorisation : euroradio_message
{
    Q_SCALE_t Q_SCALE;
    D_SR_t D_SR;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        Q_SCALE.copy(r);
        D_SR.copy(r);
    }
};
struct SH_refused : euroradio_message
{
    T_TRAIN_t T_TRAINack;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        T_TRAINack.copy(r);
    }
};
struct SH_authorised : euroradio_message
{
    T_TRAIN_t T_TRAINack;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        T_TRAINack.copy(r);
    }
};
struct RBC_version : euroradio_message
{
    M_VERSION_t M_VERSION;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        M_VERSION.copy(r);
    }
};
struct MA_shifted_message : euroradio_message
{
    Q_SCALE_t Q_SCALE;
    D_REF_t D_REF;
    std::shared_ptr<Level2_3_MA> MA;
    MA_shifted_message()
    {
        MA = std::shared_ptr<Level2_3_MA>(new Level2_3_MA());
        packets.push_back(MA);
    }
    virtual void copy(bit_manipulator &w) override
    {
        NID_MESSAGE.copy(w);
        L_MESSAGE.copy(w);
        T_TRAIN.copy(w);
        M_ACK.copy(w);
        NID_LRBG.copy(w);
        Q_SCALE.copy(w);
        D_REF.copy(w);
        MA->copy(w);
    }
};
struct ack_session_termination : euroradio_message
{
};
struct train_rejected : euroradio_message
{
};
struct train_accepted : euroradio_message
{
};
struct som_position_confirmed : euroradio_message
{
};
struct coordinate_system_assignment : euroradio_message
{
    Q_ORIENTATION_t Q_ORIENTATION;
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        M_ACK.copy(r);
        NID_LRBG.copy(r);
        Q_ORIENTATION.copy(r);
    }
};
struct SH_request : euroradio_message_traintotrack
{
    SH_request() 
    {
        NID_MESSAGE.rawdata = 130;
    }
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        NID_ENGINE.copy(r);
        copy_position_report(r);
    }
};
struct MA_request : euroradio_message_traintotrack
{
    Q_MARQSTREASON_t Q_MARQSTREASON;
    MA_request() 
    {
        NID_MESSAGE.rawdata = 132;
    }
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        NID_ENGINE.copy(r);
        Q_MARQSTREASON.copy(r);
        copy_position_report(r);
    }
};
struct position_report : euroradio_message_traintotrack
{
    position_report()
    {
        NID_MESSAGE.rawdata = 136;
    }
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        NID_ENGINE.copy(r);
        copy_position_report(r);
    }
};
struct SoM_position_report : euroradio_message_traintotrack
{
    Q_STATUS_t Q_STATUS;
    SoM_position_report()
    {
        NID_MESSAGE.rawdata = 157;
    }
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        NID_ENGINE.copy(r);
        Q_STATUS.copy(r);
        copy_position_report(r);
    }
};
struct validated_train_data_message : euroradio_message_traintotrack
{
    std::shared_ptr<TrainDataPacket> TrainData;
    validated_train_data_message()
    {
        NID_MESSAGE.rawdata = 129;
        TrainData = std::shared_ptr<TrainDataPacket>(new TrainDataPacket());
        packets.push_back(TrainData);
    }
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        NID_ENGINE.copy(r);
        copy_position_report(r);
        TrainData->copy(r);
    }
};
struct init_communication_session : euroradio_message_traintotrack
{
    init_communication_session()
    {
        NID_MESSAGE.rawdata = 155;
    }
};
struct terminate_communication_session : euroradio_message_traintotrack
{
    terminate_communication_session()
    {
        NID_MESSAGE.rawdata = 156;
    }
};
struct acknowledgement_message : euroradio_message_traintotrack
{
    T_TRAIN_t T_TRAINack;
    acknowledgement_message()
    {
        NID_MESSAGE.rawdata = 146;
    }
    void copy(bit_manipulator &r) override
    {
        NID_MESSAGE.copy(r);
        L_MESSAGE.copy(r);
        T_TRAIN.copy(r);
        NID_ENGINE.copy(r);
        T_TRAINack.copy(r);
    }
};
void update_radio();
void send_position_report(bool som=false);
void fill_message(euroradio_message_traintotrack *msg);