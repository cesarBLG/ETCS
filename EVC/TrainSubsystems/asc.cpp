#include "../Supervision/supervision.h"
#include "../Supervision/supervision_targets.h"
#include "asc.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// Interface with automatic speed control
std::string asc_status;
bool asc_fitted=true;
void update_asc()
{
    if (mode == Mode::FS && asc_fitted) {
        json status;
        std::vector<std::pair<double, double>> targets;
        const std::list<std::shared_ptr<target>> &supervised_targets = get_supervised_targets();
        for (auto &t : supervised_targets) {
            dist_base pos;
            if (t->type == target_class::EoA || t->type == target_class::SvL) {
                pos = t->get_target_position();
            } else {
                t->calculate_curves(t->get_target_speed(), 0, f41(t->get_target_speed()));
                pos = t->d_P;
            }
            double d = pos - (t->is_EBD_based ? d_maxsafefront(t->get_target_position()) : d_estfront);
            targets.push_back({d, t->get_target_speed()});
        }
        status["AllowedSpeedMpS"] = V_perm;
        status["ReleaseSpeedMpS"] = V_release;
        json targs;
        for (auto kvp : targets) {
            json j;
            j["TargetLocationM"] = round(kvp.first + current_odometer_orientation * odometer_value);
            j["TargetSpeedMpS"] = kvp.second;
            targs.push_back(j);
        }
        status["Targets"] = targs;
        asc_status = status.dump();
    } else {
        asc_status = "";
    }
}