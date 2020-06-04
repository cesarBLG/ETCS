#include "mode_profile.h"
std::list<mode_profile> mode_profiles;
bool in_mode_ack_area;
bool mode_timer_started = false;
int64_t mode_timer;
optional<mode_profile> first_profile;
void update_mode_profile()
{
    /*if (mode_timer_started && mode_timer + T_ACK*1000 < get_milliseconds()) {
        mode_timer_started = false;
        brake_conditions.push_back({nullptr, [](brake_command_information &i) {
            if (mode_acknowledged || mode_to_ack != mode || !mode_acknowledgeable)
                return true;
            return false;
        }});
    }
    in_mode_ack_area = false;
    if (mode_profiles.empty())
        return;
    mode_profile first = mode_profiles.front();
    if (first.mode != Mode::SH && first.start + first.length < d_minsafefront(first.start.get_reference())) {
        mode_profiles.pop_front();
        update_mode_profile();
        return;
    }
    if (d_maxsafefront(first.start.get_reference()) > first.start) {
        if (!mode_acknowledged || mode_to_ack != first.mode) {
            mode_timer_started = true;
            mode_timer = get_milliseconds();
        }
    } else if (d_estfront > first.start-first.acklength && V_est < first.speed) {
        in_mode_ack_area = true;
        if (!mode_acknowledged || mode_to_ack != first.mode) {
            mode_acknowledgeable = true;
            mode_acknowledged = false;
            mode_to_ack = first.mode;
        }
    }*/
}
void reset_mode_profile(distance ref, bool infill)
{
    /*if (infill) {
        for (auto it = mode_profiles.begin(); it != mode_profiles.end(); ++it) {
            if (it->start >= ref) {
                mode_profiles.erase(it, mode_profiles.end());
                break;
            }
        }
    } else {
        mode_profiles.clear();
    }*/
}
void set_mode_profile(ModeProfile profile, distance ref, bool infill)
{
    /*reset_mode_profile(ref, infill);
    std::vector<MP_element_packet> mps;
    mps.push_back(profile.element);
    mps.insert(mps.end(), profile.elements.begin(), profile.elements.end());
    distance start = ref;
    for (auto it = mps.begin(); it != mps.end(); ++it) {
        start += it->D_MAMODE.get_value(profile.Q_SCALE);
        mode_profile p;
        p.start = start;
        p.length = it->L_MAMODE.get_value(profile.Q_SCALE);
        p.acklength = it->L_ACKMAMODE.get_value(profile.Q_SCALE);
        switch (it->M_MAMODE)
        {
            case M_MAMODE_t::OS:
                p.mode = Mode::OS;
                p.speed = V_NVONSIGHT;
                break;
            case M_MAMODE_t::LS:
                p.mode = Mode::LS;
                p.speed = V_NVLIMSUPERV;
                break;
            case M_MAMODE_t::SH:
                p.mode = Mode::SH;
                p.speed = V_NVSHUNT;
                break;
        }
        p.start_SvL = it->Q_MAMODE==Q_MAMODE_t::BeginningIsSvL;
        if (it->V_MAMODE != V_MAMODE_t::UseNationalValue)
            p.speed = it->V_MAMODE.get_value();
        mode_profiles.push_back(p);
    }*/
}