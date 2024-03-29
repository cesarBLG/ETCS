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
extern bool message_when_driver_id_entered;
extern bool message_when_running_number_entered;
extern bool message_when_train_data_entered;
extern bool message_when_level_selected;
extern bool message_when_driver_ack_level;
extern bool message_when_driver_ack_mode;
extern bool message_when_driver_ack_mode;
extern bool entering_mode_message_is_time_dependent;
extern std::map<std::string, std::string> const_train_data;
extern std::map<std::string, std::vector<std::string>> custom_train_data_inputs;

void load_config(std::string serie)
{
	traindata_file = "traindata.json";
	data_entry_type = 0;
	json j;
	auto contents = platform->read_file("config.json");
	if (contents)
		j = json::parse(*contents);
	if (j.contains(serie)) {
		json& cfg = j[serie];
		if (cfg.contains("TrainData")) {
			traindata_file = cfg["TrainData"];
			data_entry_type = 1;
		}
		if (cfg.contains("UnsupportedLevels")) {
			unsupported_levels = cfg["UnsupportedLevels"].get<std::set<int>>();
		}
		if (cfg.contains("MessageWhenDriverIdEntered"))
		{
			message_when_driver_id_entered = cfg["MessageWhenDriverIdEntered"];
		}
		if (cfg.contains("MessageWhenLevelSelected"))
		{
			message_when_level_selected = cfg["MessageWhenLevelSelected"];
		}
		if (cfg.contains("MessageWhenRunningNumberEntered"))
		{
			message_when_running_number_entered = cfg["MessageWhenRunningNumberEntered"];
		}
		if (cfg.contains("MessageWhenTrainDataEntered"))
		{
			message_when_train_data_entered = cfg["MessageWhenTrainDataEntered"];
		}
		if (cfg.contains("MessageWhenModeAck"))
		{
			message_when_driver_ack_mode = cfg["MessageWhenModeAck"];
		}
		if (cfg.contains("MessageWhenLevelAck"))
		{
			message_when_driver_ack_level = cfg["MessageWhenLevelAck"];
		}
		if (cfg.contains("EnteringModeMessageIsTimeDependent"))
		{
			entering_mode_message_is_time_dependent = cfg["EnteringModeMessageIsTimeDependent"];
		}
		if (cfg.contains("ConstTrainDataValues"))
		{
			const_train_data.clear();
			for (json& j : cfg["ConstTrainDataValues"])
			{
				const_train_data[j["Field"].get<std::string>()] = j["Value"].get<std::string>();
			}
		}
		if (cfg.contains("CustomTrainDataInputs"))
		{
			custom_train_data_inputs.clear();
			for (json& j : cfg["CustomTrainDataInputs"])
			{
				custom_train_data_inputs[j["Field"].get<std::string>()] = j["Value"].get<std::vector<std::string>>();
			}
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
					std::string name = "stm::" + std::to_string(it->second->nid_stm) + "::isolated";
					for (auto* p : manager.parameters) {
						if (p->name == name) {
							manager.RemoveParameter(p);
							delete p;
							break;
						}
					}
					delete it->second;
					it = installed_stms.erase(it);
				}
				else {
					++it;
				}
			}
			for (int nid_stm : stms) {
				if (installed_stms.find(nid_stm) == installed_stms.end()) {
					installed_stms[nid_stm] = new stm_object();
					installed_stms[nid_stm]->nid_stm = nid_stm;

					auto* p = new ORserver::Parameter("stm::" + std::to_string(nid_stm) + "::isolated");
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
			for (json& j : cfg["STMPriorityTable"])
			{
				ntc_to_stm_lookup_table[j["nid_ntc"].get<int>()] = j["nid_stms"].get<std::vector<int>>();
			}
		}
	}
	set_persistent_command("setSerie", serie);
}