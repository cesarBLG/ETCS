/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "config.h"
#include "../DMI/dmi.h"
#include "../Procedures/level_transition.h"
#include "../STM/stm.h"
#include <orts/common.h>
#include <nlohmann/json.hpp>
#include "platform_runtime.h"
using json = nlohmann::json;
extern ORserver::ParameterManager manager;
extern std::string traindata_file;
extern int data_entry_type;
extern bool messasge_when_driver_id_entered;
extern bool messasge_when_running_number_entered;
extern bool messasge_when_train_data_entered;
extern bool messasge_when_level_selected;
extern bool messasge_when_driver_ack_level;
extern bool messasge_when_driver_ack_mode;
extern bool messasge_when_driver_ack_mode;
extern bool entering_mode_message_is_time_dependent;
void load_config(std::string serie)
{
    traindata_file = "traindata.json";
    data_entry_type = 0;
    json j;
    auto contents = platform->read_file("config.json");
    if (contents)
        j = json::parse(*contents);
    if (j.contains(serie)) {
        json &cfg = j[serie];
        if (cfg.contains("TrainData")) {
            traindata_file = cfg["TrainData"];
            data_entry_type = 1;
        }
        if (cfg.contains("UnsupportedLevels")) {
            unsupported_levels = cfg["UnsupportedLevels"].get<std::set<int>>();
        }
        if (cfg.contains("MessasgeWhenDriverIdEntered"))
        {
            messasge_when_driver_id_entered = cfg["MessasgeWhenDriverIdEntered"];
        }
        if (cfg.contains("MessasgeWhenLevelSelected"))
        {
            messasge_when_level_selected = cfg["MessasgeWhenLevelSelected"];
        }
        if (cfg.contains("MessasgeWhenRunningNumberEntered"))
        {
            messasge_when_running_number_entered = cfg["MessasgeWhenRunningNumberEntered"];
        }
        if (cfg.contains("MessasgeWhenTrainDataEntered"))
        {
            messasge_when_train_data_entered = cfg["MessasgeWhenTrainDataEntered"];
        }
        if (cfg.contains("MessasgeWhenModeAck"))
        {
            messasge_when_driver_ack_mode = cfg["MessasgeWhenModeAck"];
        }
        if (cfg.contains("MessasgeWhenLevelAck"))
        {
            messasge_when_driver_ack_level = cfg["MessasgeWhenLevelAck"];
        }
        if (cfg.contains("EnteringModeMessageIsTimeDependent"))
        {
            entering_mode_message_is_time_dependent = cfg["EnteringModeMessageIsTimeDependent"];
        }
        if (cfg.contains("AvailableNTC")) {
            std::set<int> ntcs = cfg["AvailableNTC"].get<std::set<int>>();
            for (int ntc : ntcs)
            {
                ntc_available_no_stm.insert(ntc);
            }
        }
        if (cfg.contains("InstalledSTM")) {
            std::set<int> stms = cfg["InstalledSTM"].get<std::set<int>>();
            for (auto it = installed_stms.begin(); it != installed_stms.end(); ) {
                if (stms.find(it->second->nid_stm) == stms.end()) {
                    std::string name = "stm::"+std::to_string(it->second->nid_stm)+"::isolated";
                    for (auto *p : manager.parameters) {
                        if (p->name == name) {
                            manager.RemoveParameter(p);
                            delete p;
                            break;
                        }
                    }
                    delete it->second;
                    it = installed_stms.erase(it);
                } else {
                    ++it;
                }
            }
            for (int nid_stm : stms) {
                if (installed_stms.find(nid_stm) == installed_stms.end()) {
                    installed_stms[nid_stm] = new stm_object();
                    installed_stms[nid_stm]->nid_stm = nid_stm;

                    auto *p = new ORserver::Parameter("stm::"+std::to_string(nid_stm)+"::isolated");
                    p->SetValue = [nid_stm](std::string val) {
                        auto it = installed_stms.find(nid_stm);
                        if (it != installed_stms.end())
                            it->second->isolated = val == "1";
                    };
                    manager.AddParameter(p);
                }
            }
        }
        if (cfg.contains("STMPriorityTable")) {
            ntc_to_stm_lookup_table.clear();
            for (json &j : cfg["STMPriorityTable"])
            {
                ntc_to_stm_lookup_table[j["nid_ntc"].get<int>()] = j["nid_stms"].get<std::vector<int>>();
            }
        }
    }
    set_persistent_command("setSerie", serie);
}