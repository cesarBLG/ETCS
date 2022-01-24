#include "start.h"
#include "../Supervision/emergency_stop.h"
#include "../Supervision/supervision.h"
#include "../Euroradio/session.h"
#include "../DMI/text_message.h"
#include "../DMI/windows.h"
#include "../Position/linking.h"
#include "../Packets/radio.h"
#include "stored_information.h"
som_step som_status = S0;
som_step prev_status = S0;
bool som_active;
bool ongoing_mission;
extern bool desk_open;
bool status_changed;
void update_SoM()
{
    som_step save_status = som_status;
    if (som_active && !desk_open)
        desk_closed_som();
    if (mode != Mode::SB || !desk_open) {
        som_active = false;
        som_status = S0;
    } else {
        som_active = true;
    }
    switch (som_status) {
        case S0:
            if (som_active) {
                if (mode == Mode::SB && desk_open && (!supervising_rbc || (supervising_rbc->status != session_status::Establishing && supervising_rbc->status != session_status::Established)))
                    som_status = S1;
                active_dialog = dialog_sequence::StartUp;
                active_dialog_step = "S0";
            }
            break;
        case S1:
            if (driver_id_valid)
                som_status = D2;
            break;
        case D2:
            if (level_valid && position_valid)
                som_status = D3;
            else
                som_status = S2;
            break;
        case D3:
            if (level == Level::N2 || level == Level::N3)
                som_status = D7;
            else
                som_status = S10;
            break;
        case D7: {
            bool registered = false;
            for (mobile_terminal &t : mobile_terminals) {
                if (t.registered) {
                    registered = true;
                    break;
                }
            }
            if (registered)
                som_status = A31;
            else
                som_status = S4;
            break; }
        case S2:
            if (level_valid) {
                if (level == Level::N2 || level == Level::N3)
                    som_status = S3;
                else
                    som_status = S10;
            }
            break;
        case S3:
            break;
        case S4: {
            bool registered = false;
            for (mobile_terminal &t : mobile_terminals) {
                if (t.registered) {
                    registered = true;
                    break;
                }
            }
            if (registered)
                som_status = A31;
            else if (true/*TODO: timer elapses*/)
                som_status = A29;
            break; }
        case A29:
            som_status = S10;
        case S10:
        case D12:
        case S13:
        case D10:
        case D11:
        case S11:
        case S20:
        case S21:
        case S22:
        case S23:
        case S24:
        case S25:
            break;
        case A31:
            if (prev_status != A31 && supervising_rbc)
                supervising_rbc->open(N_tries_radio);
            if (!supervising_rbc || supervising_rbc->status != session_status::Establishing)
                som_status = D31;
            break;
        case D31:
            if (supervising_rbc && supervising_rbc->status == session_status::Established)
                som_status = D32;
            else
                som_status = A32;
            break;
        case A32:
            //TODO
            som_status = S10;
            break;
        case D32:
            if (position_valid)
                som_status = A33;
            else
                som_status = A34;
            break;
        case A33:
            send_position_report(true);
            som_status = S10;
            break;
        case A34:
            send_position_report(true);
            som_status = D33;
            break;
        case D33:
        case D22:
            if (!supervising_rbc || supervising_rbc->status != session_status::Established) {
                som_status = S10;
            } else if (position_valid) {
                som_status = A35;
            }
            break;
        case A35:
            som_status = S10;
            break;
        case A23:
            som_status = A24;
            break;
        case A24:
            position_valid = false;
            lrbgs.clear();
            som_status = S10;
            break;
        case A38:
            som_status = A39;
            break;
        case A39:
            if (supervising_rbc)
                supervising_rbc->close();
            position_valid = false;
            lrbgs.clear();
            som_status = A40;
            break;
        case A40:
            som_status = S10;
            break;
    }
    prev_status = save_status;
}
void start_pressed()
{
    bool c1 = V_est == 0 && mode == Mode::SB && train_data_valid && level != Level::Unknown/* && som_active && som_status == S20*/;
    bool c2 = V_est == 0 && mode == Mode::PT && train_data_valid && (level == Level::N1 || ((level == Level::N2 || level == Level::N3) && supervising_rbc && supervising_rbc->status == session_status::Established && emergency_stops.empty()));
    bool c3 = mode == Mode::SR && (level == Level::N2 || level == Level::N3) && supervising_rbc && supervising_rbc->status == session_status::Established;
    if (c1 || c2 || c3) {
        if (level == Level::NTC) {
            mode_to_ack = Mode::SN;
            mode_acknowledgeable = true;
            mode_acknowledged = false;
            /*if (som_active && som_status == S20)
                som_status = S22;*/
        } else if (level == Level::N0) {
            mode_to_ack = Mode::UN;
            mode_acknowledgeable = true;
            mode_acknowledged = false;
            /*if (som_active && som_status == S20)
                som_status = S23;*/
        } else if (level == Level::N1) {
            mode_to_ack = Mode::SR;
            mode_acknowledgeable = true;
            mode_acknowledged = false;
            /*if (som_active && som_status == S20)
                som_status = S24;*/
        } else if (level == Level::N2 || level == Level::N3) {
            if (supervising_rbc && supervising_rbc->status == session_status::Established) {
                ma_rq_reasons[0] = true;
            } else {
                mode_to_ack = Mode::SR;
                mode_acknowledgeable = true;
                mode_acknowledged = false;
            }
            /*if (som_active && som_status == S20)
                som_status = S21;*/
        }
    }
}