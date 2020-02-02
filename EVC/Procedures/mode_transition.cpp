#include "mode_transition.h"
std::map<int,cond> mode_conditions;
static std::vector<mode_transition> transitions;
void initialize_mode_transitions()
{
    std::map<int,cond> c;
    c[1] = [](){return false;};
    c[2] = [](){return true;};
    c[3] = [](){return V_est == 0;};
    c[4] = [](){return true;};
    c[5] = [](){return V_est == 0 && (level==Level::N0 || level==Level::NTC || level==Level::N1);};
    c[7] = [](){return level!=Level::N0 && level!=Level::NTC && V_est==0;};
    c[31] = [](){return MA && !get_SSP().empty() && !get_gradient().empty() && (level == Level::N2 || level==Level::N3);};
    c[32] = [](){return MA && !get_SSP().empty() && !get_gradient().empty() && level == Level::N1;};
    c[62] = [](){return (level==Level::N0 || level==Level::NTC) && V_est==0 && L_TRAIN>0;};
    c[68] = [](){return (level==Level::N0 || level==Level::NTC) && V_est==0 && L_TRAIN<=0;};
    c[69] = [](){return get_SSP().begin()->get_start()>d_estfront || get_gradient().begin()->first>d_estfront;};
    mode_conditions = c;

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

    transitions.push_back({Mode::FS, Mode::TR, {12,16,17,18,20,41,65,66, 69}, 4});

    transitions.push_back({Mode::TR, Mode::PT, {7}, 4});

    transitions.push_back({Mode::TR, Mode::UN, {62}, 4});

    transitions.push_back({Mode::TR, Mode::SH, {68}, 4});
}
void update_mode_status()
{
    std::vector<mode_transition> available=transitions;
    int priority = 10;
    Mode transition = mode;
    for (mode_transition t : available) {
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
        recalculate_MRSP();
    }
}