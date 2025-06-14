/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "train_interface.h"
#include "../TrackConditions/track_condition.h"
#include "train_interface.h"
#include "../STM/stm.h"
bool cab_active[2] = {true, false};
bool sl_signal;
bool ps_signal;
bool nl_signal;
bool isolated;
bool failed;
bool SB_command;
bool EB_command;
int V_set_display = -1;
double V_set;
double brake_pressure;
int reverser_direction;
extern int data_entry_type;
extern int data_entry_type_tiu;
bool traction_cutoff_active;
bool ep_brake_available=true;
bool eddy_brake_available=true;
bool regenerative_brake_available=true;
bool magnetic_brake_available=true;
bool additional_brake_available;
bool automatic_close_air_intake;
bool automatic_open_air_intake;
bool automatic_eddy_inhibition;
bool automatic_magnetic_inhibition;
bool automatic_regenerative_inhibition;
extern bool TCO;
std::set<int> used_external_condition_ids;
std::map<std::shared_ptr<track_condition>, int> external_conditions;
std::string obu_tr_status;
void update_train_interface()
{
    for (auto it = external_conditions.begin(); it != external_conditions.end(); ) {
        if (track_conditions.find(it->first) == track_conditions.end()) {
            used_external_condition_ids.erase(it->second);
            it = external_conditions.erase(it);
        } else {
            ++it;
        }
    }
    json obu_json;
    json platform_conditions;
    json traction_change_condition;
    json allowed_current_consumption_condition;
    for (auto &tc : track_conditions) {
        if (tc->condition == TrackConditions::StationPlatform) {
            auto *platform = (track_condition_platforms*)tc.get();
            json cond;
            if (tc->external && tc->external->start)
                cond["StartDistanceToTrainM"] = *(tc->external->start);
            if (tc->external && tc->external->end)
                cond["EndDistanceToTrainM"] = *(tc->external->end);
            cond["LeftSide"] = platform->left_side;
            cond["RightSide"] = platform->right_side;
            cond["HeightM"] = platform->platform_height;
            platform_conditions.push_back(cond);
        } else if (tc->condition == TrackConditions::ChangeOfTractionSystem) {
            auto *traction = (track_condition_traction_change*)tc.get();
            if (tc->external && tc->external->start)
                traction_change_condition["DistanceToTrainM"] = *(tc->external->start);
            traction_change_condition["M_VOLTAGE"] = traction->m_voltage;
            traction_change_condition["NID_CTRACTION"] = traction->nid_ctraction;
        } else if (tc->condition == TrackConditions::ChangeOfAllowedCurrentConsumption) {
            auto *current = (track_condition_current_consumption*)tc.get();
            if (tc->external && tc->external->start)
                allowed_current_consumption_condition["DistanceToTrainM"] = *(tc->external->start);
            allowed_current_consumption_condition["MaxCurrentA"] = current->max_current;
        } else if (external_conditions.find(tc) == external_conditions.end()) {
            for (int i=0; i<256; i++) {
                if (used_external_condition_ids.find(i) == used_external_condition_ids.end()) {
                    used_external_condition_ids.insert(i);
                    external_conditions[tc] = i;
                    break;
                }
            }
        }
    }
    json profile_conditions;
    for (auto &kvp : external_conditions) {
        json cond;
        cond["Id"] = kvp.second;
        int type = 0;
        switch (kvp.first->condition)
        {
            case TrackConditions::SwitchOffRegenerativeBrake:
                type = 0;
                break;
            case TrackConditions::SwitchOffMagneticShoe:
                type = 1;
                break;
            case TrackConditions::SwitchOffEddyCurrentServiceBrake:
                type = 2;
                break;
            case TrackConditions::SwitchOffEddyCurrentEmergencyBrake:
                type = 3;
                break;
            case TrackConditions::AirTightness:
                type = 4;
                break;
            case TrackConditions::PowerLessSectionLowerPantograph:
                type = 5;
                break;
            case TrackConditions::PowerLessSectionSwitchMainPowerSwitch:
                type = 6;
                break;
        }
        cond["Type"] = type;
        if (kvp.first->external) {
            if (kvp.first->external->start)
                cond["StartDistanceToTrainM"] = *(kvp.first->external->start);
            if (kvp.first->external->end)
                cond["EndDistanceToTrainM"] = *(kvp.first->external->end);
        }
        profile_conditions.push_back(cond);
    }
    if (!traction_change_condition.empty())
        obu_json["TractionSystemChange"] = traction_change_condition;
    if (!allowed_current_consumption_condition.empty())
        obu_json["AllowedCurrentConsumptionChange"] = allowed_current_consumption_condition;
    if (!profile_conditions.empty())
        obu_json["TrackConditions"] = profile_conditions;
    
    traction_cutoff_active = TCO;
    if (mode == Mode::SL || mode == Mode::NL || mode == Mode::SN) {
        for (auto kvp : installed_stms) {
            auto *stm = kvp.second;
            if (stm->active()) {
                traction_cutoff_active |= stm->tiu_function.TCO;
                obu_json["OBU_TR_MPS_Cmd"] = !stm->tiu_function.open_circuit_breaker;
                obu_json["OBU_TR_PG_Cmd"] = stm->tiu_function.lower_pantograph;
                obu_json["OBU_TR_AT_Cmd"] = stm->tiu_function.close_air_intake;
                obu_json["OBU_TR_RBInhibit_Cmd"] = stm->tiu_function.regenerative_brake_inhibition;
                obu_json["OBU_TR_MGInhibit_Cmd"] = stm->tiu_function.magnetic_shoe_inhibition;
                obu_json["OBU_TR_ECEInhibit_Cmd"] = stm->tiu_function.eddy_emergency_brake_inhibition;
                obu_json["OBU_TR_ECSInhibit_Cmd"] = stm->tiu_function.eddy_service_brake_inhibition;
            }
        }
    }

    obu_json["OBU_TR_TCO_Cmd"] = traction_cutoff_active;
    obu_json["OBU_TR_EB3_Cmd"] = !EB_command;
    obu_json["OBU_TR_ServiceBrake"] = SB_command;

    obu_tr_status = obu_json.dump();
}
void handle_tr_inputs(json &j)
{
    if (j.contains("TR_OBU_TrainSleep") && j.contains("TR_OBU_TrainSleep_Not"))
        sl_signal = j["TR_OBU_TrainSleep"] && !j["TR_OBU_TrainSleep_Not"];
    if (j.contains("TR_OBU_PassiveShunting"))
        ps_signal = j["TR_OBU_PassiveShunting"];
    if (j.contains("TR_OBU_NLEnabled"))
        nl_signal = j["TR_OBU_NLEnabled"];
    if (j.contains("TR_OBU_CabStatusA"))
        cab_active[0] = j["TR_OBU_CabStatusA"];
    if (j.contains("TR_OBU_CabStatusB"))
        cab_active[1] = j["TR_OBU_CabStatusB"];
    if (j.contains("TR_OBU_TypeTrainData_S1") && j.contains("TR_OBU_TypeTrainData_S2")) {
        data_entry_type_tiu = j["TR_OBU_TypeTrainData_S1"].get<bool>()*2 + j["TR_OBU_TypeTrainData_S2"].get<bool>() - 1;
        if (data_entry_type_tiu >= 0) {
            if (platform->read_file(traindata_file))
                data_entry_type = data_entry_type_tiu;
            else
                data_entry_type = 0;
        }
    }
    //if (j.contains("TR_OBU_TractionStatus"))
    //if (j.contains("TR_OBU_AirTightFitted"))
    if (j.contains("TR_OBU_SetSpeedDisplay"))
        V_set_display = j["TR_OBU_SetSpeedDisplay"];
    if (j.contains("TR_OBU_BrakePressure"))
        brake_pressure = j["TR_OBU_BrakePressure"];
    if (j.contains("TR_OBU_Brake_Status")) {
        json &stat = j["TR_OBU_Brake_Status"];
        if (stat.contains("EP_S") && stat.contains("EP_S_NOT"))
            ep_brake_available = stat["EP_S"] && !stat["EP_S_NOT"];
        if (stat.contains("EC_S") && stat.contains("EC_S_NOT"))
            eddy_brake_available = stat["EC_S"] && !stat["EC_S_NOT"];
        if (stat.contains("RB_S") && stat.contains("RB_S_NOT"))
            regenerative_brake_available = stat["RB_S"] && !stat["RB_S_NOT"];
        if (stat.contains("MG_S") && stat.contains("MG_S_NOT"))
            magnetic_brake_available = stat["MG_S"] && !stat["MG_S_NOT"];
    }
    if (j.contains("TR_OBU_NTCIsolated")) {
        for (auto &ntc : j["TR_OBU_NTCIsolated"]) {
            int id = ntc["NID_NTC"];
            bool isolated = ntc["T_IS"];
            if (installed_stms.count(id))
                installed_stms[id]->isolated = isolated;
        }
    }
    if (j.contains("TR_OBU_SetSpeedValue"))
        V_set = j["TR_OBU_SetSpeedValue"];
}
void handle_tr_train_data(json &data)
{

}