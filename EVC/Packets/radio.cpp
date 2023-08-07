/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "radio.h"
#include "messages.h"
#include "../Euroradio/session.h"
#include "../MA/movement_authority.h"
#include "../Supervision/supervision.h"
#include "../Version/version.h"
void ma_request(bool driver, bool perturb, bool timer, bool trackdel, bool taf);
void fill_pos_report(euroradio_message_traintotrack *m);
ETCS_packet *get_position_report();
std::shared_ptr<euroradio_message> euroradio_message::build(bit_manipulator &r, int m_version)
{
    int size = r.bits.size();
    NID_MESSAGE_t nid;
    r.peek(&nid);
    euroradio_message* msg;
    switch (nid.rawdata) {
        case 2: msg = new SR_authorisation(); break;
        case 3: msg = new MA_message(); break;
        case 6: msg = new TR_exit_recognition(); break;
        case 8: msg = new train_data_acknowledgement(); break;
        case 9: msg = new MA_shortening_message(); break;
        case 15: msg = new conditional_emergency_stop(m_version); break;
        case 16: msg = new unconditional_emergency_stop(); break;
        case 18: msg = new emergency_stop_revocation(); break;
        case 24: msg = new euroradio_message(); break;
        case 27: msg = new SH_refused(); break;
        case 28: msg = new SH_authorised(); break;
        case 32: msg = new RBC_version(); break;
        case 33: msg = new MA_shifted_message(); break;
        case 34: msg = new taf_request_message(m_version); break;
        case 39: msg = new ack_session_termination(); break;
        case 40: msg = new train_rejected(); break;
        case 41: msg = new train_accepted(); break;
        case 43: msg = new som_position_confirmed(); break;
        case 45: msg = new coordinate_system_assignment(); break;
        default:
            bool highery = false;
            for (int v : supported_versions) {
                if (VERSION_X(m_version)==VERSION_X(v) && VERSION_Y(m_version)>VERSION_Y(v))
                    highery = true;
            }
            if (!highery)
                r.sparefound = true;
            msg = new euroradio_message();
            break;
    }
    msg->copy(r);
    while (!r.error && r.position<=(r.bits.size()*8-8))
    {
        NID_PACKET_t NID_PACKET;
        r.peek(&NID_PACKET);
        if (NID_PACKET==255)
            break;
        msg->optional_packets.push_back(std::shared_ptr<ETCS_packet>(ETCS_packet::construct(r, m_version)));
    }
    msg->packets.insert(msg->packets.end(), msg->optional_packets.begin(), msg->optional_packets.end());
    if (msg->L_MESSAGE != size) r.error=true;
    msg->readerror = r.error;
    msg->valid = !r.sparefound;
    return std::shared_ptr<euroradio_message>(msg);
}
std::shared_ptr<euroradio_message_traintotrack> euroradio_message_traintotrack::build(bit_manipulator &r)
{
    int size = r.bits.size();
    NID_MESSAGE_t nid;
    r.peek(&nid);
    euroradio_message_traintotrack *msg;
    switch (nid.rawdata) {
        case 129: msg = new validated_train_data_message(); break;
        case 130: msg = new SH_request(); break;
        case 132: msg = new MA_request(); break;
        case 136: msg = new position_report(); break;
        case 137: msg = new ma_shorten_granted(); break;
        case 138: msg = new ma_shorten_rejected(); break;
        case 146: msg = new acknowledgement_message(); break;
        case 147: msg = new emergency_acknowledgement_message(); break;
        case 149: msg = new taf_granted(); break;
        case 154: msg = new no_compatible_session_supported(); break;
        case 155: msg = new init_communication_session(); break;
        case 156: msg = new terminate_communication_session(); break;
        case 157: msg = new SoM_position_report(); break;
        case 159: msg = new communication_session_established(); break;
        default: r.sparefound = true; msg = new euroradio_message_traintotrack(); break;
    }
    msg->copy(r);
    while (!r.error && r.position<=(r.bits.size()*8-8))
    {
        NID_PACKET_t NID_PACKET;
        r.peek(&NID_PACKET);
        if (NID_PACKET==255)
            break;
        msg->optional_packets.push_back(std::shared_ptr<ETCS_packet>(ETCS_packet::construct(r, 33)));
    }
    msg->packets.insert(msg->packets.end(), msg->optional_packets.begin(), msg->optional_packets.end());
    if (msg->L_MESSAGE != size) r.error=true;
    msg->readerror = r.error;
    msg->valid = !r.sparefound;
    return std::shared_ptr<euroradio_message_traintotrack>(msg);
}
/*void send_message(euroradio_message_traintotrack *m)
{
    m->T_TRAIN.rawdata = get_milliseconds()/10;
    bit_manipulator w;
    m->write_to(w);
    std::stringstream str;
    const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i=0; 6*i<w.bits.size(); i++) {
        int v=0;
        for (int j=0; j<6; j++) {
            int idx = 6*i+j;
            if (idx >= w.bits.size())
                break;
            v += w.bits[idx]<<(5-j);
        }
        str<<base64_table[v];
    }
    if (w.bits.size()%6>0)
        str<<'=';
    if (w.bits.size()%6 == 2)
        str<<'=';
    if (s_client != nullptr && s_client->connected)
        s_client->WriteLine("noretain(etcs::messagetrain="+str.str()+")");
    bit_manipulator r(base64_decode(str.str(), true));
    //auto msg = euroradio_message_traintotrack::build(r);
}*/
optional<position_report_parameters> pos_report_params;
ma_request_parameters ma_params = {30000, (int64_t)(T_CYCRQSTD*1000), 30000};
int64_t ma_asked;
bool ma_rq_reasons[5];
bool ma_rq_reasons_old[5];
int64_t t_last_pos_rep;
distance d_last_pos_rep;
void update_radio()
{
    update_euroradio();
    if (supervising_rbc) {
        double V_MRSP = calc_ceiling_limit();
        double advance = (V_MRSP + dV_warning(V_MRSP))*ma_params.T_MAR/1000;
        if ((mode == Mode::FS || mode == Mode::LS || mode == Mode::OS) && (level == Level::N2 || level == Level::N3)) {
            ma_rq_reasons[1] = (d_perturbation_eoa && *d_perturbation_eoa-advance < d_estfront) || (d_perturbation_svl && *d_perturbation_svl-advance < d_maxsafefront(*d_perturbation_svl));
            ma_rq_reasons[2] = ma_params.T_TIMEOUTRQST > 0 && MA && MA->timers_to_expire(ma_params.T_TIMEOUTRQST);
        }
        bool request = false;
        bool any = false;
        for (int i=0; i<5; i++) {
            if (ma_rq_reasons[i]) {
                any = true;
                if (!ma_rq_reasons_old[i])
                    request = true;
            }
            ma_rq_reasons_old[i] = ma_rq_reasons[i];
        }
        if (ma_params.T_CYCRQSTD > 0 && any && get_milliseconds() - ma_asked > ma_params.T_CYCRQSTD)
            request = true;
        if (request) {
            ma_request(ma_rq_reasons[0], ma_rq_reasons[1], ma_rq_reasons[2], ma_rq_reasons[3], ma_rq_reasons[4]);
            ma_asked = get_milliseconds();
        }
        bool rep = false;
        for (auto &b : position_report_reasons) {
            if (b) {
                rep = true;
                break;
            }
        }
        if (pos_report_params) {
            if (std::abs(d_estfront-d_last_pos_rep) > pos_report_params->D_sendreport)
                rep = true;
            if (get_milliseconds()-t_last_pos_rep > pos_report_params->T_sendreport)
                rep = true;
            for (auto it = pos_report_params->location_front.begin(); it != pos_report_params->location_front.end(); ) {
                if (d_maxsafefront(*it)>*it) {
                    rep = true;
                    it = pos_report_params->location_front.erase(it);
                } else {
                    ++it;
                }
            }
            for (auto it = pos_report_params->location_rear.begin(); it != pos_report_params->location_rear.end(); ) {
                if (d_minsafefront(*it)-L_TRAIN>*it) {
                    rep = true;
                    it = pos_report_params->location_rear.erase(it);
                } else {
                    ++it;
                }
            }
        }
        if (rep)
            send_position_report();
    }
    if (!supervising_rbc || supervising_rbc->status != session_status::Established) {
        for (auto &b : position_report_reasons)
            b = false;
    }
}
int position_report_reasons[12];
void send_position_report(bool som)
{
    if (som) {
        if (!supervising_rbc)
            return;
        auto *rep = new SoM_position_report();
        if (lrbgs.empty())
            rep->Q_STATUS.rawdata = Q_STATUS_t::Unknown;
        else
            rep->Q_STATUS.rawdata = position_valid ? Q_STATUS_t::Valid : Q_STATUS_t::Invalid; 
        fill_message(rep);
        supervising_rbc->send(std::shared_ptr<euroradio_message_traintotrack>(rep));
    } else {
        std::set<communication_session*> rbcs;
        if (supervising_rbc != nullptr)
            rbcs.insert(supervising_rbc);
        if (accepting_rbc != nullptr)
            rbcs.insert(accepting_rbc);
        if (handing_over_rbc != nullptr)
            rbcs.insert(handing_over_rbc);
        if (rbcs.empty())
            return;
        for (auto *session : rbcs) {
            auto *rep = new position_report();
            fill_message(rep);
            auto msg = std::shared_ptr<euroradio_message_traintotrack>(rep);
            session->send(msg);
            std::set<int> acks;
            if ((position_report_reasons[4] && session == handing_over_rbc) || (position_report_reasons[1] && mode == Mode::SH) || position_report_reasons[5] || position_report_reasons[6] == 2)
                acks.insert(-1);
            else if (position_report_reasons[1] && mode == Mode::PT)
                acks.insert(6);
            if (!acks.empty() && session->status == session_status::Established)
                session->pending_ack.push_back({acks, msg, 1, get_milliseconds()});
        }
        t_last_pos_rep = get_milliseconds();
        d_last_pos_rep = d_estfront;
    }
    for (auto &b : position_report_reasons)
        b = false;
}
void ma_request(bool driver, bool perturb, bool timer, bool trackdel, bool taf)
{
    auto req = new MA_request();
    fill_message(req);
    req->Q_MARQSTREASON.rawdata = 0;
    req->Q_MARQSTREASON.rawdata |= (driver<<Q_MARQSTREASON_t::StartSelectedByDriverBit);
    req->Q_MARQSTREASON.rawdata |= (perturb<<Q_MARQSTREASON_t::TimeBeforePerturbationBit);
    req->Q_MARQSTREASON.rawdata |= (timer<<Q_MARQSTREASON_t::TimeBeforeTimerBit);
    req->Q_MARQSTREASON.rawdata |= (trackdel<<Q_MARQSTREASON_t::TrackDescriptionDeletedBit);
    req->Q_MARQSTREASON.rawdata |= (taf<<Q_MARQSTREASON_t::TrackAheadFreeBit);
    supervising_rbc->send(std::shared_ptr<euroradio_message_traintotrack>(req));
}
int64_t last_sent_timestamp;
void fill_message(euroradio_message_traintotrack *m)
{
    m->NID_ENGINE.rawdata = 0;
    ETCS_packet *pos = get_position_report();
    m->PositionReport1BG = {};
    m->PositionReport2BG = {};
    if (pos->NID_PACKET == 1)
        m->PositionReport2BG = std::shared_ptr<PositionReportBasedOnTwoBaliseGroups>((PositionReportBasedOnTwoBaliseGroups*)pos);
    else
        m->PositionReport1BG = std::shared_ptr<PositionReport>((PositionReport*)pos);

    int64_t timestamp = get_milliseconds()/10;
    if (last_sent_timestamp >= timestamp)
        timestamp = last_sent_timestamp + 1;
    m->T_TRAIN.rawdata = timestamp & 4294967295ULL;
    last_sent_timestamp = timestamp;
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
            int dir = lrbgs.back().dir;
            r->NID_LRBG.set_value(lrbgs.back().nid_lrbg);
            double dist = d_estfront - lrbgs.back().position;
            r->D_LRBG.set_value(std::abs(dist), r->Q_SCALE);
            r->Q_DIRLRBG.set_value(odometer_orientation * lrbgs.back().position.get_orientation() == 1 ? dir : 1-dir);
            r->Q_DLRBG.set_value(dist > 0 ? dir : 1-dir);
            r->Q_DIRTRAIN.set_value(odometer_direction * lrbgs.back().position.get_orientation() == 1 ? dir : 1-dir);
            r->L_DOUBTOVER.set_value(d_maxsafefront(lrbgs.back().position)-d_estfront, r->Q_SCALE);
            r->L_DOUBTUNDER.set_value(d_estfront-d_minsafefront(lrbgs.back().position), r->Q_SCALE);
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
            lrbg_info &prevLRBG = *(----lrbgs.end());
            if (prevLRBG.position.get_orientation() != lrbgs.back().position.get_orientation()) {
                r->Q_DIRLRBG = Q_DIRLRBG_t::Unknown;
                r->Q_DLRBG = Q_DLRBG_t::Unknown;
                r->Q_DIRTRAIN = Q_DIRTRAIN_t::Unknown;
                r->NID_PRVLRBG = NID_PRVLRBG_t::Unknown;
            } else {
                r->Q_DIRLRBG.set_value((lrbgs.back().position.get()-prevLRBG.position.get())*odometer_orientation < 0);
                r->Q_DLRBG.set_value(dist < 0);
                r->Q_DIRTRAIN.set_value((lrbgs.back().position.get()-prevLRBG.position.get())*odometer_direction < 0);
                r->NID_PRVLRBG.set_value(prevLRBG.nid_lrbg);
            }
        } else {
            r->Q_DIRLRBG = Q_DIRLRBG_t::Unknown;
            r->Q_DLRBG = Q_DLRBG_t::Unknown;
            r->Q_DIRTRAIN = Q_DIRTRAIN_t::Unknown;
            r->NID_PRVLRBG = NID_PRVLRBG_t::Unknown;
        }
        r->L_DOUBTOVER.set_value(d_maxsafefront(lrbgs.back().position)-d_estfront, r->Q_SCALE);
        r->L_DOUBTUNDER.set_value(d_estfront-d_minsafefront(lrbgs.back().position), r->Q_SCALE);
        r->Q_LENGTH = Q_LENGTH_t::NoTrainIntegrityAvailable;
        r->V_TRAIN.set_value(V_est);
        r->M_MODE.set_value(mode);
        r->M_LEVEL.set_value(level);
        return r;
    }
    return nullptr;
}