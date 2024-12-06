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
#include "../TrainSubsystems/power.h"
#include "../TrainSubsystems/train_interface.h"
#include "../Supervision/locomotive_data.h"
#include <orts/common.h>
#include <nlohmann/json.hpp>
#include "platform_runtime.h"
using json = nlohmann::json;
extern ORserver::ParameterManager manager;
extern std::string traindata_file;
extern int data_entry_type;
extern int data_entry_type_tiu;
extern bool message_when_driver_id_entered;
extern bool message_when_running_number_entered;
extern bool message_when_train_data_entered;
extern bool message_when_level_selected;
extern bool message_when_driver_ack_level;
extern bool message_when_driver_ack_mode;
extern bool message_when_driver_ack_mode;
extern bool entering_mode_message_is_time_dependent;
extern bool asc_fitted;
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
			if (data_entry_type_tiu >= 0)
				data_entry_type = data_entry_type_tiu;
			else
				data_entry_type = 1;
		}
		unsupported_levels = {};
		if (cfg.contains("UnsupportedLevels")) {
			unsupported_levels = cfg["UnsupportedLevels"].get<std::set<int>>();
		}
		message_when_driver_id_entered = cfg.value("MessageWhenDriverIdEntered", false);
		message_when_level_selected = cfg.value("MessageWhenLevelSelected", false);
		message_when_running_number_entered = cfg.value("MessageWhenRunningNumberEntered", false);
		message_when_train_data_entered = cfg.value("MessageWhenTrainDataEntered", false);
		message_when_driver_ack_mode = cfg.value("MessageWhenModeAck", false);
		message_when_driver_ack_level = cfg.value("MessageWhenLevelAck", false);
		entering_mode_message_is_time_dependent = cfg.value("EnteringModeMessageIsTimeDependent", false);
		const_train_data.clear();
		if (cfg.contains("ConstTrainDataValues"))
		{
			for (json& j : cfg["ConstTrainDataValues"])
			{
				const_train_data[j["Field"].get<std::string>()] = j["Value"].get<std::string>();
			}
		}
		custom_train_data_inputs.clear();
		if (cfg.contains("CustomTrainDataInputs"))
		{
			for (json& j : cfg["CustomTrainDataInputs"])
			{
				custom_train_data_inputs[j["Field"].get<std::string>()] = j["Value"].get<std::vector<std::string>>();
			}
		}
		ntc_available_no_stm.clear();
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
		ntc_to_stm_lookup_table.clear();
		if (cfg.contains("STMPriorityTable")) {
			for (json& j : cfg["STMPriorityTable"])
			{
				ntc_to_stm_lookup_table[j["nid_ntc"].get<int>()] = j["nid_stms"].get<std::vector<int>>();
			}
		}
		traction_cutoff_implemented = cfg.value("TractionCutOffImplemented", true);
		automatic_close_power_switch = cfg.value("AutomaticCloseMainPowerSwitch", true);
		automatic_open_power_switch = cfg.value("AutomaticOpenMainPowerSwitch", true);
		automatic_raise_pantograph = cfg.value("AutomaticRaisePantograph", false);
		automatic_lower_pantograph = cfg.value("AutomaticLowerPantograph", false);
		automatic_traction_system_change = cfg.value("AutomaticTractionSystemChange", false);
		automatic_close_air_intake = cfg.value("AutomaticCloseAirIntake", true);
		automatic_open_air_intake = cfg.value("AutomaticOpenAirIntake", true);
		automatic_eddy_inhibition = cfg.value("AutomaticEddyCurrentBrakeInhibition", false);
		automatic_magnetic_inhibition = cfg.value("AutomaticMagneticShoeBrakeInhibition", false);
		automatic_regenerative_inhibition = cfg.value("AutomaticRegenerativeBrakeInhibition", false);
		asc_fitted = cfg.value("AutomaticSpeedControl", false);
		L_antenna_front = cfg.value("AntennaOffset", 0.0);
		L_locomotive = cfg.value("RearCabOffset", 0.0);
	}
	set_persistent_command("setSerie", serie);
}