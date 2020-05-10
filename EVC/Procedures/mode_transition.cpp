#include "mode_transition.h"
#include "train_trip.h"
#include <map>
cond mode_conditions[74];
static std::vector<mode_transition> transitions;
static std::vector<mode_transition> ordered_transitions[20];
void initialize_mode_transitions()
{
    cond *c = mode_conditions;
    c[1] = [](){return false;};
    c[2] = [](){return true;};
    c[3] = [](){return V_est == 0;};
    c[4] = [](){return true;};
    c[5] = [](){return V_est == 0 && (level==Level::N0 || level==Level::NTC || level==Level::N1);};
    c[7] = [](){return level!=Level::N0 && level!=Level::NTC && V_est==0;};
    c[8] = [](){return false;};
    c[12] = [](){return level == Level::N1 && EoA && *EoA<(d_minsafefront(EoA->get_reference())-L_antenna_front);};
    c[16] = [](){return (level == Level::N2 || level==Level::N3) && EoA && *EoA<d_minsafefront(EoA->get_reference());};
    c[25] = [](){return (level == Level::N1 || level == Level::N2 || level==Level::N3) && MA && !get_SSP().empty() && !get_gradient().empty();};
    c[37] = [](){return false;};
    c[31] = [](){return MA && !get_SSP().empty() && !get_gradient().empty() && (level == Level::N2 || level==Level::N3);};
    c[32] = [](){return MA && !get_SSP().empty() && !get_gradient().empty() && level == Level::N1;};
    c[39] = [](){return (level == Level::N1 || level == Level::N2 || level==Level::N3) && !MA;};
    c[62] = [](){return (level==Level::N0 || level==Level::NTC) && V_est==0 && L_TRAIN>0;};
    c[68] = [](){return (level==Level::N0 || level==Level::NTC) && V_est==0 && L_TRAIN<=0;};
    c[69] = [](){return get_SSP().begin()->get_start()>d_estfront || get_gradient().begin()->first>d_estfront;};

    transitions.push_back({Mode::SB, Mode::SR, {8,37}, 7});
    transitions.push_back({Mode::FS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::LS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::OS, Mode::SR, {37}, 6});
    transitions.push_back({Mode::PT, Mode::SR, {8,37}, 5});
    transitions.push_back({Mode::UN, Mode::SR, {44,45}, 4});
    transitions.push_back({Mode::SN, Mode::SR, {44,45}, 4});

    transitions.push_back({Mode::SB, Mode::FS, {29}, 7});
    transitions.push_back({Mode::LS, Mode::FS, {31,32}, 6});
    transitions.push_back({Mode::SR, Mode::FS, {31,32}, 6});
    transitions.push_back({Mode::OS, Mode::FS, {31,32}, 6});
    transitions.push_back({Mode::UN, Mode::FS, {25}, 7});
    transitions.push_back({Mode::PT, Mode::FS, {31}, 5});
    transitions.push_back({Mode::SN, Mode::FS, {25}, 7});

    transitions.push_back({Mode::SB, Mode::TR, {20}, 4});
    transitions.push_back({Mode::SH, Mode::TR, {49,52,65}, 4});
    transitions.push_back({Mode::FS, Mode::TR, {12,16,17,18,20,41,65,66,69}, 4});
    transitions.push_back({Mode::LS, Mode::TR, {12,16,17,18,20,41,65,66,69}, 4});
    transitions.push_back({Mode::SR, Mode::TR, {18,20,42,43,36,54,65}, 4});
    transitions.push_back({Mode::OS, Mode::TR, {12,16,17,18,20,41,65,66,69}, 4});
    transitions.push_back({Mode::UN, Mode::TR, {20,39,67}, 5});
    transitions.push_back({Mode::SN, Mode::TR, {20,35,38,39,67}, 5});

    transitions.push_back({Mode::TR, Mode::PT, {7}, 4});

    transitions.push_back({Mode::SB, Mode::UN, {60}, 7});
    transitions.push_back({Mode::FS, Mode::UN, {21}, 6});
    transitions.push_back({Mode::LS, Mode::UN, {21}, 6});
    transitions.push_back({Mode::SR, Mode::UN, {21}, 6});
    transitions.push_back({Mode::OS, Mode::UN, {21}, 6});
    transitions.push_back({Mode::TR, Mode::UN, {62}, 4});
    transitions.push_back({Mode::SN, Mode::UN, {21}, 7});

    transitions.push_back({Mode::TR, Mode::SH, {68}, 4});
    for (mode_transition &t : transitions) {
        ordered_transitions[(int)t.from].push_back(t);
    }
}

void update_mode_status()
{
    std::vector<mode_transition> &available=ordered_transitions[(int)mode];
    int priority = 10;
    Mode transition = mode;
    for (mode_transition &t : available) {
        if (t.from == mode && t.happens() && t.priority < priority) {
            transition = t.to;
            priority = t.priority;
        }
    }   
    if (mode != transition) {
        mode = transition;
        if (mode == Mode::SR) {
            if (std::isfinite(D_NVSTFF)) {
                SR_dist = distance(d_maxsafefront(0)+D_NVSTFF);
                SR_speed = speed_restriction(V_NVSTFF, std::numeric_limits<double>::lowest(), *SR_dist, false);
            } else {
                SR_dist = {};
                SR_speed = speed_restriction(V_NVSTFF, std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), false);
            }
        } else {
            SR_dist = {};
            SR_speed = {};
        }
        if (mode == Mode::TR) {
            train_trip();
        }
        recalculate_MRSP();
    }
}