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
#include "radio.h"
#include "messages.h"
#include "15.h"
#include "../MA/movement_authority.h"
#include "../Supervision/supervision.h"
std::map<int, euroradio_message*> euroradio_message::message_factory;
std::map<int, euroradio_message_traintotrack*> euroradio_message_traintotrack::message_factory;
struct MA_message : euroradio_message
{
    std::shared_ptr<Level2_3_MA> MA;
    MA_message() {}
    MA_message(bit_read_temp &r)
    {
        r.read(&NID_MESSAGE);
        r.read(&L_MESSAGE);
        r.read(&T_TRAIN);
        r.read(&M_ACK);
        r.read(&NID_LRBG);
        MA = std::shared_ptr<Level2_3_MA>(new Level2_3_MA(r));
        packets.push_back(MA);
    }
    MA_message * create(bit_read_temp &r) override { return new MA_message(r);}
    /*virtual void serialize(bit_write &w) override
    {
        w.write(&NID_MESSAGE);
        w.write(&L_MESSAGE);
        w.write(&T_TRAIN);
        w.write(&M_ACK);
        w.write(&NID_LRBG);
    }*/
};
struct MA_request : euroradio_message_traintotrack
{
    Q_MARQSTREASON_t Q_MARQSTREASON;
    MA_request() {}
    MA_request(bit_read_temp &r)
    {
        r.read(&NID_MESSAGE);
        r.read(&L_MESSAGE);
        r.read(&T_TRAIN);
        r.read(&NID_ENGINE);
        r.read(&Q_MARQSTREASON);
        read_position_report(r);
    }
    MA_request * create(bit_read_temp &r) override { return new MA_request(r);}
    virtual void serialize(bit_write &w) override
    {
        w.write(&NID_MESSAGE);
        w.write(&L_MESSAGE);
        w.write(&T_TRAIN);
        w.write(&NID_ENGINE);
        w.write(&Q_MARQSTREASON);
        write_position_report(w);
    }
};
void euroradio_message::initialize()
{
    message_factory[3] = new MA_message();
    message_factory[24] = new euroradio_message();
}
void euroradio_message_traintotrack::initialize()
{
    message_factory[132] = new MA_request();
}
void ma_request(bool driver, bool perturb, bool timer, bool trackdel, bool taf);
void fill_pos_report(euroradio_message_traintotrack *m);
ETCS_packet *get_position_report();
#include <orts/client.h>
#include <orts/common.h>
#include <sstream>
extern ORserver::POSIXclient *s_client;
void send_message(euroradio_message_traintotrack *m)
{
    /*bit_write w;
    m->write_to(w);
    std::stringstream str;
    for (auto b : w.bits) {
        str<< (b ? "1" : "0");
    }
    if (s_client != nullptr && s_client->connected)
        s_client->WriteLine("noretain(etcs::messagetrain="+str.str()+")");

    bit_read_temp r(w.bits);
    auto msg = euroradio_message_traintotrack::build(r);*/
}
struct ma_request_parameters
{
    int64_t T_MAR;
    int64_t T_CYCRQSTD;
    int64_t T_TIMEOUTRQST;
};
ma_request_parameters ma_params = {30000, T_CYCRQSTD*1000, 30000};
int64_t ma_asked;
void update_radio()
{
    double V_MRSP = calc_ceiling_limit();
    double advance = (V_MRSP + dV_warning(V_MRSP))*ma_params.T_MAR;
    if ((d_perturbation_eoa && *d_perturbation_eoa-advance < d_estfront) || (d_perturbation_svl && *d_perturbation_svl-advance < d_maxsafefront(d_perturbation_svl->get_reference()))) {
        ma_request(false, true, false, false, false);
        ma_asked = get_milliseconds();
    }
    if (get_milliseconds() > ma_asked + ma_params.T_CYCRQSTD)
    {
        ma_request(false, false, false, false, false);
        ma_asked = get_milliseconds();
    }
}
void ma_request(bool driver, bool perturb, bool timer, bool trackdel, bool taf)
{
    MA_request req;
    req.NID_MESSAGE.rawdata = 132;
    fill_pos_report(&req);
    req.Q_MARQSTREASON.rawdata = 0;
    req.Q_MARQSTREASON.rawdata |= (driver<<Q_MARQSTREASON_t::StartSelectedByDriverBit);
    req.Q_MARQSTREASON.rawdata |= (perturb<<Q_MARQSTREASON_t::TimeBeforePerturbationBit);
    req.Q_MARQSTREASON.rawdata |= (timer<<Q_MARQSTREASON_t::TimeBeforeTimerBit);
    req.Q_MARQSTREASON.rawdata |= (trackdel<<Q_MARQSTREASON_t::TrackDescriptionDeletedBit);
    req.Q_MARQSTREASON.rawdata |= (taf<<Q_MARQSTREASON_t::TrackAheadFreeBit);
    send_message(&req);
}
void fill_pos_report(euroradio_message_traintotrack *m)
{
    ETCS_packet *pos = get_position_report();
    m->PositionReport1BG = {};
    m->PositionReport2BG = {};
    if (pos->NID_PACKET == 1)
        m->PositionReport2BG = std::shared_ptr<PositionReportBasedOnTwoBaliseGroups>((PositionReportBasedOnTwoBaliseGroups*)pos);
    else
        m->PositionReport1BG = std::shared_ptr<PositionReport>((PositionReport*)pos);
}
ETCS_packet *get_position_report()
{
    if (lrbgs.empty() || lrbgs.back().dir != -1) {
        PositionReport *r = new PositionReport();
        r->NID_PACKET = 0;
        r->Q_SCALE = Q_SCALE_t::m1;
        if (lrbgs.empty()) {
            r->NID_LRBG = NID_LRBG_t::Unknown;
            r->D_LRBG = D_LRBG_t::Unknown;
            r->Q_DIRLRBG = Q_DIRLRBG_t::Unknown;
            r->Q_DLRBG = Q_DLRBG_t::Unknown;
            r->L_DOUBTOVER = L_DOUBTOVER_t::Unknown;
            r->L_DOUBTUNDER = L_DOUBTUNDER_t::Unknown;
            r->Q_DIRTRAIN = Q_DIRTRAIN_t::Unknown;
        } else {
            int dir = lrbgs.back().dir == 0;
            r->NID_LRBG.set_value(lrbgs.back().nid_lrbg);
            double dist = d_estfront - lrbgs.back().position;
            r->D_LRBG.set_value(std::abs(dist), r->Q_SCALE);
            r->Q_DIRLRBG.set_value(dir);
            r->Q_DLRBG.set_value(dist > 0 ? dir : 1-dir);
            r->Q_DIRTRAIN.set_value(dir);
        }
        r->Q_LENGTH = Q_LENGTH_t::NoTrainIntegrityAvailable;
        r->V_TRAIN.set_value(V_est);
        r->M_MODE.set_value(mode);
        r->M_LEVEL.set_value(level);
        return r;
    } else {
        PositionReportBasedOnTwoBaliseGroups *r = new PositionReportBasedOnTwoBaliseGroups();
        r->NID_PACKET = 1;
        r->Q_SCALE = Q_SCALE_t::m1;
        r->NID_LRBG.set_value(lrbgs.back().nid_lrbg);
        double dist = d_estfront - lrbgs.back().position;
            r->D_LRBG.set_value(std::abs(dist), r->Q_SCALE);
        if (lrbgs.size() > 1) {
            r->Q_DIRLRBG.set_value(false);
            r->Q_DLRBG.set_value(dist < 0);
            r->Q_DIRTRAIN.set_value(false);
            r->NID_PRVLRBG.set_value((----lrbgs.end())->nid_lrbg);
        } else {
            r->Q_DIRLRBG = Q_DIRLRBG_t::Unknown;
            r->Q_DLRBG = Q_DLRBG_t::Unknown;
            r->Q_DIRTRAIN = Q_DIRTRAIN_t::Unknown;
            r->NID_PRVLRBG = NID_PRVLRBG_t::Unknown;
        }
        r->Q_LENGTH = Q_LENGTH_t::NoTrainIntegrityAvailable;
        r->V_TRAIN.set_value(V_est);
        r->M_MODE.set_value(mode);
        r->M_LEVEL.set_value(level);
        return r;
    }
    return nullptr;
}